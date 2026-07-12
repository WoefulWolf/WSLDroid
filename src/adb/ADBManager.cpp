#include "ADBManager.h"
#include "core/WSLManager.h"
#include <QRegularExpression>
#include <QProcess>
#include <QDebug>

ADBManager::ADBManager(WSLManager* wslManager, QObject* parent)
    : QObject(parent), m_wsl(wslManager), m_state(ADBState::Disconnected),
      m_connectAttempts(0) {

    m_pollTimer = new QTimer(this);
    connect(m_pollTimer, &QTimer::timeout, this, &ADBManager::pollStatus);
}

ADBManager::~ADBManager() {
    disconnectFromDevice();
}

ADBManager::ADBState ADBManager::state() const {
    return m_state;
}

QString ADBManager::deviceIp() const {
    return m_deviceIp;
}

QString ADBManager::stateString() const {
    switch (m_state) {
        case ADBState::Disconnected: return "Disconnected";
        case ADBState::Detecting:   return "Detecting";
        case ADBState::Connecting:  return "Connecting";
        case ADBState::Connected:   return "Connected";
        case ADBState::Failed:      return "Failed";
    }
    return "Unknown";
}

void ADBManager::transitionTo(ADBState newState) {
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(m_state);
        emit logMessage(QString("[ADB] State changed to: %1").arg(stateString()));
    }
}

bool ADBManager::detectWaydroidIp() {
    emit logMessage("[ADB] Detecting Waydroid container IP address...");

    // Try `waydroid prop get` to read the Android IP assigned by LXC networking
    WSLManager::ExecResult res = m_wsl->executeCommandSync(
        "sh", {"-c", "waydroid prop get persist.waydroid.net.shared_system_ips 2>/dev/null || "
                      "ip -4 addr show waydroid0 2>/dev/null | grep -oP '(?<=inet )([0-9.]+)'"}
    );

    if (res.success && !res.stdOut.trimmed().isEmpty()) {
        QString rawOutput = res.stdOut.trimmed();
        // Extract the first valid IPv4 address from the output
        QRegularExpression ipRegex(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))");
        QRegularExpressionMatch match = ipRegex.match(rawOutput);
        if (match.hasMatch()) {
            m_deviceIp = match.captured(1);
            emit logMessage(QString("[ADB] Found Waydroid IP: %1").arg(m_deviceIp));
            return true;
        }
    }

    // Fallback: check the waydroid status output for IP
    WSLManager::ExecResult statusRes = m_wsl->executeCommandSync(
        "sh", {"-c", "waydroid status 2>/dev/null"}
    );
    if (statusRes.success) {
        QRegularExpression ipInStatus(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))");
        QRegularExpressionMatchIterator it = ipInStatus.globalMatch(statusRes.stdOut);
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            QString candidate = m.captured(1);
            // Exclude loopback and common host bridge IPs
            if (!candidate.startsWith("127.") && !candidate.startsWith("10.0.2.")) {
                m_deviceIp = candidate;
                emit logMessage(QString("[ADB] Found Waydroid IP from status: %1").arg(m_deviceIp));
                return true;
            }
        }
    }

    emit logMessage("[ADB] Could not detect Waydroid IP address.");
    return false;
}

bool ADBManager::runAdbConnect() {
    if (m_deviceIp.isEmpty()) return false;

    // Run `adb connect <ip>:5555` from inside WSL
    QString target = m_deviceIp + ":5555";
    emit logMessage(QString("[ADB] Connecting to %1...").arg(target));

    WSLManager::ExecResult res = m_wsl->executeCommandSync(
        "sh", {"-c", QString("adb connect %1 2>&1").arg(target)}
    );

    if (res.success && (res.stdOut.contains("connected") || res.stdOut.contains("already connected"))) {
        emit logMessage(QString("[ADB] Successfully connected to %1.").arg(target));
        return true;
    }

    emit logMessage(QString("[ADB] Failed to connect: %1").arg(res.stdOut.trimmed()));
    return false;
}

void ADBManager::connectToDevice() {
    if (m_state == ADBState::Connected || m_state == ADBState::Connecting) return;

    m_connectAttempts = 0;
    transitionTo(ADBState::Detecting);

    if (!detectWaydroidIp()) {
        // Retry via polling
        m_pollTimer->start(3000);
        return;
    }

    transitionTo(ADBState::Connecting);
    if (runAdbConnect()) {
        transitionTo(ADBState::Connected);
    } else {
        m_pollTimer->start(3000); // Retry polling
        transitionTo(ADBState::Failed);
    }
}

void ADBManager::disconnectFromDevice() {
    m_pollTimer->stop();

    if (!m_deviceIp.isEmpty()) {
        emit logMessage(QString("[ADB] Disconnecting from %1:5555...").arg(m_deviceIp));
        // Kill adb server inside WSL
        m_wsl->executeCommandSync("sh", {"-c", "adb disconnect 2>/dev/null; adb kill-server 2>/dev/null"});
        m_deviceIp.clear();
    }

    transitionTo(ADBState::Disconnected);
}

void ADBManager::installApk(const QString& apkPath) {
    if (m_state != ADBState::Connected) {
        emit logMessage("[ADB] Cannot install APK: device is not connected.");
        emit apkInstallFinished(false, "Device not connected.");
        return;
    }

    emit logMessage(QString("[ADB] Installing APK: %1").arg(apkPath));

    // Convert Windows path to WSL path and run adb install
    QString wslPath = apkPath;
    wslPath.replace("\\", "/");
    // Handle drive letter conversion: C:\... -> /mnt/c/...
    if (wslPath.length() >= 2 && wslPath[1] == ':') {
        QString driveLetter = QString(wslPath[0]).toLower();
        wslPath = "/mnt/" + driveLetter + wslPath.mid(2);
    }

    QProcess* process = m_wsl->executeCommandAsync(
        "sh", {"-c", QString("adb install -r \"%1\" 2>&1").arg(wslPath)}, this
    );

    if (!process) {
        emit apkInstallFinished(false, "Failed to start install process.");
        return;
    }

    connect(process, &QProcess::finished, this, [this, process](int exitCode, QProcess::ExitStatus) {
        QString output = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
        if (exitCode == 0 && output.contains("Success")) {
            emit logMessage("[ADB] APK installed successfully.");
            emit apkInstallFinished(true, output);
        } else {
            emit logMessage(QString("[ADB] APK install failed: %1").arg(output));
            emit apkInstallFinished(false, output);
        }
        process->deleteLater();
    });
}

void ADBManager::pollStatus() {
    m_connectAttempts++;

    if (m_connectAttempts > 10) {
        m_pollTimer->stop();
        emit logMessage("[ADB] Timeout: Could not connect to Waydroid ADB after 10 attempts.");
        transitionTo(ADBState::Failed);
        return;
    }

    emit logMessage(QString("[ADB] Retry attempt %1/10...").arg(m_connectAttempts));

    if (m_deviceIp.isEmpty() && !detectWaydroidIp()) return;

    if (m_state != ADBState::Connecting) transitionTo(ADBState::Connecting);

    if (runAdbConnect()) {
        m_pollTimer->stop();
        transitionTo(ADBState::Connected);
    }
}
