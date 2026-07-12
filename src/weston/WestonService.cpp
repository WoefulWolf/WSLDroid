#include "WestonService.h"
#include <QTcpSocket>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

WestonService::WestonService(WSLManager* wslManager, QObject* parent)
    : QObject(parent), m_wsl(wslManager), m_process(nullptr), m_state(WestonState::Failed), m_connectionAttempts(0) {
    
    m_connectionTimer = new QTimer(this);
    connect(m_connectionTimer, &QTimer::timeout, this, &WestonService::checkConnection);
}

WestonService::~WestonService() {
    stopWeston();
}

WestonService::WestonState WestonService::state() const {
    return m_state;
}

QString WestonService::stateString() const {
    switch (m_state) {
        case WestonState::Starting: return "Starting";
        case WestonState::Running: return "Running";
        case WestonState::Restarting: return "Restarting";
        case WestonState::Failed: return "Failed";
        case WestonState::Recovering: return "Recovering";
    }
    return "Unknown";
}

void WestonService::transitionTo(WestonState newState) {
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged(m_state);
        emit logMessage(QString("Weston service state changed to: %1").arg(stateString()));
    }
}

bool WestonService::queryGuestHome() {
    WSLManager::ExecResult res = m_wsl->executeCommandSync("sh", {"-c", "echo $HOME"});
    if (res.success) {
        m_guestHome = res.stdOut.trimmed();
        return !m_guestHome.isEmpty();
    }
    return false;
}

bool WestonService::generateCertificate() {
    emit logMessage("Verifying RDP TLS certificate in WSL guest...");
    
    // Command to create directory and generate self-signed certificate if tls.key doesn't exist
    QString cmd = "mkdir -p ~/.local/share/weston && "
                  "[ ! -f ~/.local/share/weston/tls.key ] && "
                  "openssl req -x509 -newkey rsa:2048 -nodes "
                  "-keyout ~/.local/share/weston/tls.key "
                  "-out ~/.local/share/weston/tls.crt "
                  "-subj \"/CN=localhost\" -days 365 || echo \"Cert already exists\"";
                  
    WSLManager::ExecResult res = m_wsl->executeCommandSync("sh", {"-c", cmd});
    if (res.success) {
        emit logMessage("TLS certificate verified/generated successfully.");
        return true;
    } else {
        emit logMessage(QString("TLS certificate generation failed: %1").arg(res.stdErr));
        return false;
    }
}

bool WestonService::startWeston() {
    if (m_state == WestonState::Starting || m_state == WestonState::Running) {
        return true;
    }
    
    if (m_state != WestonState::Recovering && m_state != WestonState::Restarting) {
        transitionTo(WestonState::Starting);
    }
    
    // 1. Generate TLS Certificates
    if (!generateCertificate()) {
        transitionTo(WestonState::Failed);
        return false;
    }
    
    // 2. Launch Weston with RDP Backend
    emit logMessage("Spawning Weston compositor in guest...");
    
    // Use exec to overlay shell and allow QProcess to monitor weston directly
    QString westonCmd = "exec weston --backend=rdp-backend.so --port=3390 "
                        "--rdp-tls-cert=$HOME/.local/share/weston/tls.crt "
                        "--rdp-tls-key=$HOME/.local/share/weston/tls.key "
                        "--width=1080 --height=1920";
                        
    m_process = m_wsl->executeCommandAsync("sh", {"-c", westonCmd}, this);
    
    if (!m_process) {
        emit logMessage("Failed to instantiate QProcess for Weston.");
        transitionTo(WestonState::Failed);
        return false;
    }
    
    connect(m_process, &QProcess::finished, this, &WestonService::handleProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &WestonService::handleProcessError);
    
    // 3. Start Connection Monitoring (polls port 3390)
    m_connectionAttempts = 0;
    m_connectionTimer->start(500);
    
    return true;
}

void WestonService::stopWeston() {
    m_connectionTimer->stop();
    
    if (m_process) {
        emit logMessage("Stopping Weston compositor...");
        m_process->disconnect(this); // Disconnect finished/error handlers
        m_process->terminate();
        
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(m_process, &QProcess::finished, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(3000);
        loop.exec();
        
        if (!timer.isActive()) {
            m_process->kill();
        } else {
            timer.stop();
        }
        
        m_process->deleteLater();
        m_process = nullptr;
    }
    
    transitionTo(WestonState::Failed);
}

void WestonService::restartWeston() {
    emit logMessage("Restarting Weston compositor...");
    transitionTo(WestonState::Restarting);
    stopWeston();
    startWeston();
}

void WestonService::checkConnection() {
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 3390);
    
    if (socket.waitForConnected(200)) {
        m_connectionTimer->stop();
        transitionTo(WestonState::Running);
        emit logMessage("Weston compositor is running and accepting RDP connections.");
    } else {
        m_connectionAttempts++;
        if (m_connectionAttempts >= 20) { // 10 seconds timeout
            m_connectionTimer->stop();
            emit logMessage("Timeout waiting for Weston to bind to RDP port 3390.");
            
            // Trigger Recovery state if we were trying to start
            if (m_state == WestonState::Starting) {
                transitionTo(WestonState::Recovering);
                stopWeston();
                // Attempt to auto-recover once
                startWeston();
            } else {
                transitionTo(WestonState::Failed);
                stopWeston();
            }
        }
    }
}

void WestonService::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    
    m_connectionTimer->stop();
    emit logMessage(QString("Weston process exited with code %1").arg(exitCode));
    
    if (m_state == WestonState::Running) {
        // Weston crashed while running, try to recover
        transitionTo(WestonState::Recovering);
        startWeston();
    } else {
        transitionTo(WestonState::Failed);
    }
}

void WestonService::handleProcessError(QProcess::ProcessError error) {
    emit logMessage(QString("Weston process error occurred: %1").arg(error));
    m_connectionTimer->stop();
    
    if (m_state != WestonState::Recovering) {
        transitionTo(WestonState::Failed);
    }
}
