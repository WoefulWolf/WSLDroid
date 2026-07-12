#include "WaydroidManager.h"
#include <QThread>
#include <QRegularExpression>
#include <QDebug>

WaydroidManager::WaydroidManager(WSLManager* wslManager, QObject* parent)
    : QObject(parent), m_wsl(wslManager), m_containerProcess(nullptr), m_sessionProcess(nullptr) {
}

WaydroidManager::~WaydroidManager() {
    stopWaydroid();
}

bool WaydroidManager::isContainerRunning() const {
    WSLManager::ExecResult res = m_wsl->executeCommandSync("waydroid", {"status"});
    if (!res.success) {
        return false;
    }
    
    // Check if output contains "Container: RUNNING" or similar
    QRegularExpression re("Container\\s*:\\s*RUNNING", QRegularExpression::CaseInsensitiveOption);
    return re.match(res.stdOut).hasMatch();
}

bool WaydroidManager::isSessionRunning() const {
    WSLManager::ExecResult res = m_wsl->executeCommandSync("waydroid", {"status"});
    if (!res.success) {
        return false;
    }
    
    // Check if output contains "Session: RUNNING" or similar
    QRegularExpression re("Session\\s*:\\s*RUNNING", QRegularExpression::CaseInsensitiveOption);
    return re.match(res.stdOut).hasMatch();
}

bool WaydroidManager::isWaydroidReady() const {
    return isContainerRunning() && isSessionRunning();
}

bool WaydroidManager::startContainer() {
    if (isContainerRunning()) {
        emit logMessage("Waydroid container is already running.");
        return true;
    }
    
    emit logMessage("Starting Waydroid container as root...");
    
    if (m_containerProcess) {
        m_containerProcess->terminate();
        m_containerProcess->deleteLater();
    }
    
    m_containerProcess = new QProcess(this);
    m_containerProcess->start("wsl.exe", {"-d", m_wsl->targetDistro(), "-u", "root", "--exec", "waydroid", "container", "start"});
    
    // Poll for container status up to 10 seconds (20 * 500ms)
    for (int i = 0; i < 20; ++i) {
        if (isContainerRunning()) {
            emit logMessage("Waydroid container started successfully.");
            return true;
        }
        QThread::msleep(500);
    }
    
    emit logMessage("Timeout waiting for Waydroid container to start.");
    return false;
}

bool WaydroidManager::startSession() {
    if (isSessionRunning()) {
        emit logMessage("Waydroid session is already running.");
        return true;
    }
    
    emit logMessage("Starting Waydroid session...");
    
    if (m_sessionProcess) {
        m_sessionProcess->terminate();
        m_sessionProcess->deleteLater();
    }
    
    m_sessionProcess = new QProcess(this);
    m_sessionProcess->start("wsl.exe", {"-d", m_wsl->targetDistro(), "--exec", "waydroid", "session", "start"});
    
    // Poll for session status up to 10 seconds (20 * 500ms)
    for (int i = 0; i < 20; ++i) {
        if (isSessionRunning()) {
            emit logMessage("Waydroid session started successfully.");
            return true;
        }
        QThread::msleep(500);
    }
    
    emit logMessage("Timeout waiting for Waydroid session to start.");
    return false;
}

bool WaydroidManager::stopWaydroid() {
    emit logMessage("Stopping Waydroid session and container...");
    
    // 1. Stop the user session
    m_wsl->executeCommandSync("waydroid", {"session", "stop"});
    
    // 2. Stop the container (requires root)
    QProcess::execute("wsl.exe", {"-d", m_wsl->targetDistro(), "-u", "root", "--exec", "waydroid", "container", "stop"});
    
    if (m_containerProcess) {
        m_containerProcess->terminate();
        if (!m_containerProcess->waitForFinished(2000)) {
            m_containerProcess->kill();
        }
        m_containerProcess->deleteLater();
        m_containerProcess = nullptr;
    }
    
    if (m_sessionProcess) {
        m_sessionProcess->terminate();
        if (!m_sessionProcess->waitForFinished(2000)) {
            m_sessionProcess->kill();
        }
        m_sessionProcess->deleteLater();
        m_sessionProcess = nullptr;
    }
    
    emit logMessage("Waydroid stopped.");
    return true;
}
