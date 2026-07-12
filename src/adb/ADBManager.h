#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>

class WSLManager;

class ADBManager : public QObject {
    Q_OBJECT
public:
    enum class ADBState {
        Disconnected,
        Detecting,
        Connecting,
        Connected,
        Failed
    };
    Q_ENUM(ADBState)

    explicit ADBManager(WSLManager* wslManager, QObject* parent = nullptr);
    ~ADBManager() override;

    void connectToDevice();
    void disconnectFromDevice();
    void installApk(const QString& apkPath);

    ADBState state() const;
    QString deviceIp() const;

signals:
    void stateChanged(ADBState state);
    void logMessage(const QString& msg);
    void apkInstallFinished(bool success, const QString& output);

private slots:
    void pollStatus();

private:
    WSLManager* m_wsl;
    QTimer* m_pollTimer;
    ADBState m_state;
    QString m_deviceIp;
    int m_connectAttempts;

    void transitionTo(ADBState newState);
    bool detectWaydroidIp();
    bool runAdbConnect();
    QString stateString() const;
};
