#pragma once
#include <QThread>
#include <QMutex>
#include "FrameQueue.h"

class FreeRDPClient : public QThread {
    Q_OBJECT
public:
    explicit FreeRDPClient(FrameQueue* queue, QObject* parent = nullptr);
    ~FreeRDPClient() override;

    // Probe localhost first, fallback to WSL IP. Call this instead of connectToHost directly.
    void probeAndConnect(int port = 3390);

    void connectToHost(const QString& host, int port);
    void disconnectFromHost();
    bool isConnected() const;
    QString resolvedHost() const;

    void sendPointerEvent(uint16_t flags, uint16_t x, uint16_t y);
    void sendKeyboardEvent(uint16_t flags, uint16_t code);

protected:
    void run() override;

signals:
    void connectionStatusChanged(bool connected);
    void logMessage(const QString& msg);
    // Emitted once the probe finishes: rdpReachable = true if real Weston RDP was found
    void probeResult(bool rdpReachable, const QString& resolvedIp);

private:
    FrameQueue* m_queue;
    QString m_host;
    int m_port;
    bool m_connected;
    bool m_running;
    QMutex m_mutex;

    // Returns true if a TCP connection can be established to host:port
    bool probePort(const QString& host, int port, int timeoutMs = 1500);

    // Detect WSL2 IP via `wsl.exe hostname -I`
    QString detectWslIp();

    void generateMockFrame(int frameNumber);
};
