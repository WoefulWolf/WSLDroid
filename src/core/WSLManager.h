#pragma once
#include <QObject>
#include <QProcess>
#include <QStringList>

class WSLManager : public QObject {
    Q_OBJECT
public:
    explicit WSLManager(QObject* parent = nullptr);
    ~WSLManager() override = default;

    void setTargetDistro(const QString& distroName);
    QString targetDistro() const;

    bool isDistroInstalled(const QString& distroName = QString()) const;
    bool startDistro() const;
    
    struct ExecResult {
        int exitCode;
        QString stdOut;
        QString stdErr;
        bool success;
    };
    
    // Synchronous execution using wsl.exe wrapper
    ExecResult executeCommandSync(const QString& command, const QStringList& args = QStringList(), int timeoutMs = 10000) const;

    // Asynchronous execution returning QProcess pointer
    QProcess* executeCommandAsync(const QString& command, const QStringList& args = QStringList(), QObject* commandParent = nullptr) const;

    QStringList getInstalledDistros() const;

signals:
    void logMessage(const QString& msg);

private:
    QString m_targetDistro;
};
