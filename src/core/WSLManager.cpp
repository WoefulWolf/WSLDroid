#include "WSLManager.h"
#include <QRegularExpression>
#include <QDebug>
#include <QSettings>
#include <QEventLoop>
#include <QTimer>

// Helper to decode wsl.exe stdout/stderr which often outputs UTF-16 LE on Windows
static QString decodeWslOutput(const QByteArray& bytes) {
    if (bytes.isEmpty()) return QString();
    
    // Check for UTF-16 LE Byte Order Mark (BOM)
    if (bytes.size() >= 2 && static_cast<uint8_t>(bytes[0]) == 0xFF && static_cast<uint8_t>(bytes[1]) == 0xFE) {
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(bytes.constData() + 2), (bytes.size() - 2) / 2);
    }
    
    // Check if it looks like UTF-16 LE without BOM (every second byte is 0)
    bool looksLikeUtf16 = false;
    if (bytes.size() >= 4) {
        int zeroCount = 0;
        int checkLen = qMin(100, bytes.size());
        for (int i = 1; i < checkLen; i += 2) {
            if (bytes[i] == 0) zeroCount++;
        }
        if (zeroCount * 2 >= (checkLen - 2)) {
            looksLikeUtf16 = true;
        }
    }
    
    if (looksLikeUtf16) {
        return QString::fromUtf16(reinterpret_cast<const char16_t*>(bytes.constData()), bytes.size() / 2);
    }
    
    return QString::fromLocal8Bit(bytes);
}

WSLManager::WSLManager(QObject* parent) : QObject(parent) {
    QSettings settings("config.ini", QSettings::IniFormat);
    m_targetDistro = settings.value("wsl/distro", "Ubuntu-24.04").toString();
}

void WSLManager::setTargetDistro(const QString& distroName) {
    m_targetDistro = distroName;
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.setValue("wsl/distro", distroName);
}

QString WSLManager::targetDistro() const {
    return m_targetDistro;
}

QStringList WSLManager::getInstalledDistros() const {
    QStringList distros;
    QProcess process;
    process.start("wsl.exe", {"--list", "--quiet"});
    if (!process.waitForFinished(5000)) {
        return distros;
    }
    
    QString output = decodeWslOutput(process.readAllStandardOutput());
    QStringList lines = output.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            distros << trimmed;
        }
    }
    return distros;
}

bool WSLManager::isDistroInstalled(const QString& distroName) const {
    QString target = distroName.isEmpty() ? m_targetDistro : distroName;
    QStringList installed = getInstalledDistros();
    
    for (const QString& d : installed) {
        if (d.compare(target, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

bool WSLManager::startDistro() const {
    QProcess process;
    process.start("wsl.exe", {"-d", m_targetDistro, "--exec", "true"});
    
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&process, &QProcess::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();
    
    if (timer.isActive()) {
        timer.stop();
        return (process.exitCode() == 0);
    } else {
        process.kill();
        return false;
    }
}

WSLManager::ExecResult WSLManager::executeCommandSync(const QString& command, const QStringList& args, int timeoutMs) const {
    ExecResult result;
    result.success = false;
    result.exitCode = -1;
    
    QStringList wslArgs = {"-d", m_targetDistro, "--exec", command};
    wslArgs.append(args);
    
    QString fullCmd = "wsl.exe " + wslArgs.join(" ");
    emit const_cast<WSLManager*>(this)->logMessage("[WSL EXEC] " + fullCmd);
    
    QProcess process;
    process.start("wsl.exe", wslArgs);
    
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&process, &QProcess::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);
    loop.exec();
    
    if (!timer.isActive()) {
        process.kill();
        result.stdErr = "Timeout waiting for WSL command execution.";
        emit const_cast<WSLManager*>(this)->logMessage("[WSL TIMEOUT] " + result.stdErr);
        return result;
    }
    
    timer.stop();
    
    result.exitCode = process.exitCode();
    result.stdOut = decodeWslOutput(process.readAllStandardOutput());
    result.stdErr = decodeWslOutput(process.readAllStandardError());
    result.success = (process.exitCode() == 0);
    
    if (!result.success) {
        QString err = result.stdErr.isEmpty() ? "No error details returned." : result.stdErr.trimmed();
        emit const_cast<WSLManager*>(this)->logMessage(QString("[WSL ERROR] Exit Code %1: %2").arg(result.exitCode).arg(err));
    } else {
        emit const_cast<WSLManager*>(this)->logMessage("[WSL SUCCESS] Command finished successfully.");
    }
    
    return result;
}

QProcess* WSLManager::executeCommandAsync(const QString& command, const QStringList& args, QObject* commandParent) const {
    QStringList wslArgs = {"-d", m_targetDistro, "--exec", command};
    wslArgs.append(args);
    
    QString fullCmd = "wsl.exe " + wslArgs.join(" ");
    emit const_cast<WSLManager*>(this)->logMessage("[WSL ASYNC] " + fullCmd);
    
    QProcess* process = new QProcess(commandParent);
    process->start("wsl.exe", wslArgs);
    return process;
}
