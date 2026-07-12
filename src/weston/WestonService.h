#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
#include "core/WSLManager.h"

class WestonService : public QObject {
    Q_OBJECT
public:
    enum class WestonState {
        Starting,
        Running,
        Restarting,
        Failed,
        Recovering
    };
    Q_ENUM(WestonState)

    explicit WestonService(WSLManager* wslManager, QObject* parent = nullptr);
    ~WestonService() override;

    bool startWeston();
    void stopWeston();
    void restartWeston();

    WestonState state() const;
    QString stateString() const;

signals:
    void stateChanged(WestonState newState);
    void logMessage(const QString& msg);

private slots:
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleProcessError(QProcess::ProcessError error);
    void checkConnection();

private:
    WSLManager* m_wsl;
    QProcess* m_process;
    WestonState m_state;
    QTimer* m_connectionTimer;
    int m_connectionAttempts;
    QString m_guestHome;

    void transitionTo(WestonState newState);
    bool generateCertificate();
    bool queryGuestHome();
};
