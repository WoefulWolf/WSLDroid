#pragma once
#include <QMainWindow>
#include <QStackedWidget>

#include "weston/WestonService.h"
#include "waydroid/WaydroidManager.h"

class SidebarPanel;
class TopToolbar;
class StatusBarPanel;
class EmulatorWindow;
class WSLManager;
class KernelManager;
class QTextEdit;
class FrameQueue;
class FreeRDPClient;
class ADBManager;

class WindowManager : public QMainWindow {
    Q_OBJECT
public:
    explicit WindowManager(QWidget* parent = nullptr);
    ~WindowManager() override = default;

private slots:
    void handleSidebarNavigation(int index);
    void handleStartRequested();
    void handleStopRequested();
    void handleRestartRequested();
    void handleShowEmulatorWindow();
    void handleWestonStateChanged(WestonService::WestonState state);

private:
    SidebarPanel* m_sidebar;
    TopToolbar* m_topToolbar;
    StatusBarPanel* m_statusBar;
    QStackedWidget* m_stackedWidget;
    EmulatorWindow* m_emulatorWindow;
    WSLManager* m_wsl;
    KernelManager* m_kernel;
    WestonService* m_weston;
    WaydroidManager* m_waydroid;
    QTextEdit* m_consoleLog;
    FrameQueue* m_frameQueue;
    FreeRDPClient* m_rdpClient;
    ADBManager* m_adb;

    void setupLayout();
    QWidget* createDevicePage();
    QWidget* createDashboardPage();
    QWidget* createPlaceholderPage(const QString& title);
    void appendConsoleLog(const QString& msg);
};
