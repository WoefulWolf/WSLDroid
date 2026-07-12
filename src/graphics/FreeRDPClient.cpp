#include "FreeRDPClient.h"
#include <QDebug>
#include <QTcpSocket>
#include <QProcess>
#include <QEventLoop>
#include <QTimer>
#include <cmath>
#include <algorithm>

FreeRDPClient::FreeRDPClient(FrameQueue* queue, QObject* parent)
    : QThread(parent), m_queue(queue), m_port(3390), m_connected(false), m_running(false) {
}

FreeRDPClient::~FreeRDPClient() {
    disconnectFromHost();
}

bool FreeRDPClient::probePort(const QString& host, int port, int timeoutMs) {
    QTcpSocket socket;
    socket.connectToHost(host, static_cast<quint16>(port));
    bool ok = socket.waitForConnected(timeoutMs);
    socket.close();
    return ok;
}

QString FreeRDPClient::detectWslIp() {
    QProcess proc;
    proc.start("wsl.exe", {"hostname", "-I"});
    
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&proc, &QProcess::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(3000);
    loop.exec();
    
    if (timer.isActive()) {
        timer.stop();
        QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        // hostname -I returns space-separated IPs, take the first non-empty one
        const QStringList ips = output.split(' ', Qt::SkipEmptyParts);
        if (!ips.isEmpty()) return ips.first();
    } else {
        proc.kill();
    }
    return QString();
}

QString FreeRDPClient::resolvedHost() const {
    return m_host;
}

void FreeRDPClient::probeAndConnect(int port) {
    // Run the probe in a short-lived background thread so the main UI stays responsive
    QThread* probeThread = QThread::create([this, port]() {
        // 1. Try localhost (127.0.0.1) first — WSL2 forwards ports to localhost
        emit logMessage("[FreeRDP] Probing localhost:" + QString::number(port) + "...");
        if (probePort("127.0.0.1", port)) {
            emit logMessage("[FreeRDP] ✓ localhost:" + QString::number(port) + " reachable. Connecting.");
            emit probeResult(true, "127.0.0.1");
            connectToHost("127.0.0.1", port);
            return;
        }
        
        // 2. Fallback: detect WSL2 IP and try that
        emit logMessage("[FreeRDP] localhost unreachable. Detecting WSL2 IP address...");
        QString wslIp = detectWslIp();
        if (!wslIp.isEmpty()) {
            emit logMessage("[FreeRDP] Probing WSL2 IP " + wslIp + ":" + QString::number(port) + "...");
            if (probePort(wslIp, port)) {
                emit logMessage("[FreeRDP] ✓ WSL2 IP " + wslIp + ":" + QString::number(port) + " reachable. Connecting.");
                emit probeResult(true, wslIp);
                connectToHost(wslIp, port);
                return;
            }
            emit logMessage("[FreeRDP] WSL2 IP " + wslIp + " also unreachable.");
        }
        
        // 3. Neither worked — run mock frames so UI is still functional
        emit logMessage("[WSL ERROR] Cannot reach Weston RDP. Running mock frame generator as fallback.");
        emit probeResult(false, QString());
        connectToHost("127.0.0.1", port);
    });
    
    // Auto-delete when the probe thread finishes
    connect(probeThread, &QThread::finished, probeThread, &QObject::deleteLater);
    probeThread->start();
}

void FreeRDPClient::connectToHost(const QString& host, int port) {
    QMutexLocker locker(&m_mutex);
    m_host = host;
    m_port = port;
    m_running = true;
    start();
}

void FreeRDPClient::disconnectFromHost() {
    {
        QMutexLocker locker(&m_mutex);
        m_running = false;
    }
    wait();
}

bool FreeRDPClient::isConnected() const {
    return m_connected;
}

void FreeRDPClient::run() {
    QString startMsg = QString("[FreeRDP] Connecting to Weston RDP server at %1:%2...").arg(m_host).arg(m_port);
    qDebug() << startMsg;
    emit logMessage(startMsg);
    
    {
        QMutexLocker locker(&m_mutex);
        m_connected = true;
    }
    emit connectionStatusChanged(true);
    
    QString connMsg = "[FreeRDP] Mock RDP connection established. Starting frame generation at 60 FPS...";
    qDebug() << connMsg;
    emit logMessage(connMsg);
    
    int frameNumber = 0;
    while (true) {
        {
            QMutexLocker locker(&m_mutex);
            if (!m_running) break;
        }
        
        generateMockFrame(frameNumber++);
        
        // Log every 300 frames (~5 seconds at 60fps) so we don't spam
        if (frameNumber % 300 == 0) {
            QString frameMsg = QString("[FreeRDP] Generated %1 frames so far.").arg(frameNumber);
            qDebug() << frameMsg;
            emit logMessage(frameMsg);
        }
        
        // Sleep to throttle frame rate to ~60 FPS (16.67ms per frame)
        QThread::msleep(16);
    }
    
    {
        QMutexLocker locker(&m_mutex);
        m_connected = false;
    }
    emit connectionStatusChanged(false);
    
    QString stopMsg = "[FreeRDP] Client thread stopped.";
    qDebug() << stopMsg;
    emit logMessage(stopMsg);
}

void FreeRDPClient::generateMockFrame(int frameNumber) {
    int width = 1080;
    int height = 1920;
    
    QByteArray buffer;
    buffer.resize(width * height * 4);
    uint32_t* pixels = reinterpret_cast<uint32_t*>(buffer.data());
    
    // Generate a premium gradient background shifting dynamically
    int shift = frameNumber % 360;
    for (int y = 0; y < height; ++y) {
        uint8_t r = static_cast<uint8_t>(24 + 16 * sin((y + shift) * 0.005));
        uint8_t g = static_cast<uint8_t>(24 + 16 * cos((y * 2 + shift) * 0.003));
        uint8_t b = static_cast<uint8_t>(28 + 20 * sin((y * 0.5 + shift) * 0.01));
        uint32_t color = (0xFF << 24) | (b << 16) | (g << 8) | r;
        
        std::fill_n(pixels + y * width, width, color);
    }
    
    // Draw an animated status indicator ring moving across the frame
    int radius = 40;
    int centerX = (width / 2) + static_cast<int>(150 * cos(frameNumber * 0.05));
    int centerY = (height / 2) + static_cast<int>(150 * sin(frameNumber * 0.05));
    
    for (int y = centerY - radius; y < centerY + radius; ++y) {
        if (y < 0 || y >= height) continue;
        for (int x = centerX - radius; x < centerX + radius; ++x) {
            if (x < 0 || x >= width) continue;
            
            // Equation of circle boundary
            int dx = x - centerX;
            int dy = y - centerY;
            int distSq = dx * dx + dy * dy;
            if (distSq <= radius * radius && distSq >= (radius - 5) * (radius - 5)) {
                pixels[y * width + x] = 0xFF4FC3F7; // Accent color
            }
        }
    }
    
    Frame frame;
    frame.pixels = buffer;
    frame.width = width;
    frame.height = height;
    m_queue->push(frame);
}

void FreeRDPClient::sendPointerEvent(uint16_t flags, uint16_t x, uint16_t y) {
    // In a real implementation:
    // if (m_connected && m_instance) {
    //     m_instance->input->MouseEvent(m_instance->input, flags, x, y);
    // }
    (void)flags; (void)x; (void)y;
}

void FreeRDPClient::sendKeyboardEvent(uint16_t flags, uint16_t code) {
    // In a real implementation:
    // if (m_connected && m_instance) {
    //     m_instance->input->KeyboardEvent(m_instance->input, flags, code);
    // }
    (void)flags; (void)code;
}
