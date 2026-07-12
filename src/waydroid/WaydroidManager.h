#pragma once
#include <QObject>
#include <QProcess>
#include "core/WSLManager.h"

class WaydroidManager : public QObject {
    Q_OBJECT
public:
    explicit WaydroidManager(WSLManager* wslManager, QObject* parent = nullptr);
    ~WaydroidManager() override;

    bool startContainer();
    bool startSession();
    bool stopWaydroid();

    bool isContainerRunning() const;
    bool isSessionRunning() const;
    bool isWaydroidReady() const;

signals:
    void logMessage(const QString& msg);

private:
    WSLManager* m_wsl;
    QProcess* m_containerProcess;
    QProcess* m_sessionProcess;
};
