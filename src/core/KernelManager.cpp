#include "KernelManager.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

KernelManager::KernelManager(WSLManager* wslManager, QObject* parent) 
    : QObject(parent), m_wsl(wslManager) {
}

QString KernelManager::getWslConfigFilePath() const {
    return QDir::toNativeSeparators(QDir::homePath() + "/.wslconfig");
}

bool KernelManager::checkWslConfigPresent() const {
    return QFile::exists(getWslConfigFilePath());
}

bool KernelManager::checkWslConfigHasKernel() const {
    QFile file(getWslConfigFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("#") || line.startsWith(";")) {
            continue;
        }
        if (line.startsWith("kernel", Qt::CaseInsensitive)) {
            // Check if there is an equals sign and a path
            int eqIdx = line.indexOf('=');
            if (eqIdx != -1 && line.mid(eqIdx + 1).trimmed().length() > 0) {
                return true;
            }
        }
    }
    return false;
}

QString KernelManager::getWslConfigKernelPath() const {
    QFile file(getWslConfigFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString();
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("#") || line.startsWith(";")) {
            continue;
        }
        if (line.startsWith("kernel", Qt::CaseInsensitive)) {
            int eqIdx = line.indexOf('=');
            if (eqIdx != -1) {
                QString path = line.mid(eqIdx + 1).trimmed();
                // Strip quotes if present
                if (path.startsWith("\"") && path.endsWith("\"")) {
                    path = path.mid(1, path.length() - 2);
                }
                return path;
            }
        }
    }
    return QString();
}

bool KernelManager::checkGuestKernelVersion(QString& versionOut) const {
    WSLManager::ExecResult res = m_wsl->executeCommandSync("uname", {"-r"});
    if (res.success) {
        versionOut = res.stdOut.trimmed();
        return true;
    }
    return false;
}

bool KernelManager::checkGuestBinderSupport() const {
    // Check if binderfs is registered in /proc/filesystems or /dev/binderfs exists
    WSLManager::ExecResult res = m_wsl->executeCommandSync("sh", {"-c", "grep -q binder /proc/filesystems && echo yes || echo no"});
    if (res.success && res.stdOut.trimmed() == "yes") {
        return true;
    }
    
    // Fallback: check if /dev/binderfs directory exists
    WSLManager::ExecResult res2 = m_wsl->executeCommandSync("sh", {"-c", "[ -d /dev/binderfs ] && echo yes || echo no"});
    if (res2.success && res2.stdOut.trimmed() == "yes") {
        return true;
    }
    
    return false;
}

bool KernelManager::checkGuestWaydroidInstalled() const {
    WSLManager::ExecResult res = m_wsl->executeCommandSync("which", {"waydroid"});
    return res.success && !res.stdOut.trimmed().isEmpty();
}
