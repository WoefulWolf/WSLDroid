#pragma once
#include <QObject>
#include "WSLManager.h"

class KernelManager : public QObject {
    Q_OBJECT
public:
    explicit KernelManager(WSLManager* wslManager, QObject* parent = nullptr);
    ~KernelManager() override = default;

    bool checkWslConfigPresent() const;
    bool checkWslConfigHasKernel() const;
    QString getWslConfigKernelPath() const;

    bool checkGuestKernelVersion(QString& versionOut) const;
    bool checkGuestBinderSupport() const;
    bool checkGuestWaydroidInstalled() const;

private:
    WSLManager* m_wsl;
    QString getWslConfigFilePath() const;
};
