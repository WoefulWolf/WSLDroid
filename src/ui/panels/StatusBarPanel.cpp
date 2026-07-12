#include "StatusBarPanel.h"
#include "styles/StyleHelper.h"
#include <QHBoxLayout>

StatusBarPanel::StatusBarPanel(QWidget* parent) : QFrame(parent) {
    setObjectName("StatusBarFrame");
    setStyleSheet(StyleHelper::getStatusBarStyleSheet());
    
    setupUi();
    
    // Set default initial values
    setAndroidVersion("Android 11");
    setResolution("1080x1920");
    setFps(0);
    setLatency(0);
    setCpuUsage(0.0);
    setRamUsage(0.0);
    
    setWestonStatus(false);
    setWaydroidStatus(false);
    setAdbStatus(false);
    setKernelStatus(false);
}

void StatusBarPanel::setupUi() {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(12);
    
    m_lblAndroidVersion = new QLabel(this);
    m_lblResolution = new QLabel(this);
    m_lblFps = new QLabel(this);
    m_lblLatency = new QLabel(this);
    m_lblCpu = new QLabel(this);
    m_lblRam = new QLabel(this);
    
    layout->addWidget(m_lblAndroidVersion);
    layout->addWidget(m_lblResolution);
    layout->addWidget(m_lblFps);
    layout->addWidget(m_lblLatency);
    layout->addWidget(m_lblCpu);
    layout->addWidget(m_lblRam);
    
    layout->addStretch();
    
    // Status indicators
    m_lightKernel = new QLabel(" KERNEL ", this);
    m_lightKernel->setObjectName("StatusLight");
    
    m_lightWeston = new QLabel(" WESTON ", this);
    m_lightWeston->setObjectName("StatusLight");
    
    m_lightWaydroid = new QLabel(" WAYDROID ", this);
    m_lightWaydroid->setObjectName("StatusLight");
    
    m_lightAdb = new QLabel(" ADB ", this);
    m_lightAdb->setObjectName("StatusLight");
    
    layout->addWidget(new QLabel("Services:", this));
    layout->addWidget(m_lightKernel);
    layout->addWidget(m_lightWeston);
    layout->addWidget(m_lightWaydroid);
    layout->addWidget(m_lightAdb);
}

void StatusBarPanel::setAndroidVersion(const QString& version) {
    m_lblAndroidVersion->setText("Version: " + version);
}

void StatusBarPanel::setResolution(const QString& res) {
    m_lblResolution->setText("Res: " + res);
}

void StatusBarPanel::setFps(int fps) {
    m_lblFps->setText("FPS: " + QString::number(fps));
}

void StatusBarPanel::setLatency(int ms) {
    m_lblLatency->setText("Latency: " + QString::number(ms) + " ms");
}

void StatusBarPanel::setCpuUsage(double percent) {
    m_lblCpu->setText("CPU: " + QString::number(percent, 'f', 1) + "%");
}

void StatusBarPanel::setRamUsage(double percent) {
    m_lblRam->setText("RAM: " + QString::number(percent, 'f', 1) + "%");
}

void StatusBarPanel::setLightColor(QLabel* label, bool active) {
    if (active) {
        label->setStyleSheet("background-color: #66BB6A; color: #181818;"); // Green
    } else {
        label->setStyleSheet("background-color: #EF5350; color: #FFFFFF;"); // Red
    }
}

void StatusBarPanel::setWestonStatus(bool running) {
    setLightColor(m_lightWeston, running);
}

void StatusBarPanel::setWaydroidStatus(bool running) {
    setLightColor(m_lightWaydroid, running);
}

void StatusBarPanel::setAdbStatus(bool running) {
    setLightColor(m_lightAdb, running);
}

void StatusBarPanel::setKernelStatus(bool running) {
    setLightColor(m_lightKernel, running);
}
