#pragma once
#include <QFrame>
#include <QLabel>

class StatusBarPanel : public QFrame {
    Q_OBJECT
public:
    explicit StatusBarPanel(QWidget* parent = nullptr);

    void setAndroidVersion(const QString& version);
    void setResolution(const QString& res);
    void setFps(int fps);
    void setLatency(int ms);
    void setCpuUsage(double percent);
    void setRamUsage(double percent);
    
    void setWestonStatus(bool running);
    void setWaydroidStatus(bool running);
    void setAdbStatus(bool running);
    void setKernelStatus(bool running);

private:
    QLabel* m_lblAndroidVersion;
    QLabel* m_lblResolution;
    QLabel* m_lblFps;
    QLabel* m_lblLatency;
    QLabel* m_lblCpu;
    QLabel* m_lblRam;
    
    QLabel* m_lightWeston;
    QLabel* m_lightWaydroid;
    QLabel* m_lightAdb;
    QLabel* m_lightKernel;

    void setupUi();
    void setLightColor(QLabel* label, bool active);
};
