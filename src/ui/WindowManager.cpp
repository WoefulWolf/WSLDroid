#include "WindowManager.h"
#include "styles/StyleHelper.h"
#include "panels/SidebarPanel.h"
#include "panels/StatusBarPanel.h"
#include "toolbar/TopToolbar.h"
#include "widgets/EmulatorWindow.h"
#include "core/WSLManager.h"
#include "core/KernelManager.h"
#include "weston/WestonService.h"
#include "graphics/FrameQueue.h"
#include "graphics/FreeRDPClient.h"
#include "adb/ADBManager.h"
#include <QMessageBox>
#include <QTextEdit>
#include <QTime>
#include <QCoreApplication>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>

WindowManager::WindowManager(QWidget* parent) : QMainWindow(parent), m_emulatorWindow(nullptr) {
    m_wsl = new WSLManager(this);
    m_kernel = new KernelManager(m_wsl, this);
    m_weston = new WestonService(m_wsl, this);
    m_waydroid = new WaydroidManager(m_wsl, this);
    m_frameQueue = new FrameQueue();
    m_rdpClient = new FreeRDPClient(m_frameQueue, this);
    m_adb = new ADBManager(m_wsl, this);
    
    // Connect FreeRDP logs to UI console
    connect(m_rdpClient, &FreeRDPClient::logMessage, this, &WindowManager::appendConsoleLog);
    
    // Connect ADB logs + state changes to UI console
    connect(m_adb, &ADBManager::logMessage, this, &WindowManager::appendConsoleLog);
    connect(m_adb, &ADBManager::stateChanged, this, [this](ADBManager::ADBState state) {
        bool connected = (state == ADBManager::ADBState::Connected);
        m_statusBar->setAdbStatus(connected);
    });
    
    setWindowTitle("OctaDroid");
    resize(1280, 800);
    setStyleSheet(StyleHelper::getApplicationStyleSheet());
    
    setupLayout();
}

void WindowManager::setupLayout() {
    // Main central widget with vertical layout
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // 1. Top Toolbar
    m_topToolbar = new TopToolbar(this);
    mainLayout->addWidget(m_topToolbar);
    
    // 2. Middle Area (Splitter for resizable Sidebar and Stacked Content)
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);
    
    m_sidebar = new SidebarPanel(this);
    splitter->addWidget(m_sidebar);
    
    m_stackedWidget = new QStackedWidget(this);
    
    // Create pages matching sidebar items
    // Index 0: Dashboard
    m_stackedWidget->addWidget(createDashboardPage());
    
    // Index 1: Device Emulator View (Contains AndroidRenderWidget + EmulatorToolbar)
    m_stackedWidget->addWidget(createDevicePage());
    
    // Index 2: APK Installer
    m_stackedWidget->addWidget(createPlaceholderPage("APK Package Manager"));
    
    // Index 3: Applications
    m_stackedWidget->addWidget(createPlaceholderPage("Installed Applications"));
    
    // Index 4: Files
    m_stackedWidget->addWidget(createPlaceholderPage("Device Files Browser"));
    
    // Index 5: Logcat
    m_stackedWidget->addWidget(createPlaceholderPage("Logcat Live Viewer"));
    
    // Index 6: Terminal
    m_stackedWidget->addWidget(createPlaceholderPage("WSL Terminal Console"));
    
    // Index 7: Settings
    m_stackedWidget->addWidget(createPlaceholderPage("OctaDroid Preferences"));
    
    // Index 8: About
    m_stackedWidget->addWidget(createPlaceholderPage("About OctaDroid"));
    
    splitter->addWidget(m_stackedWidget);
    
    // Set stretch factors: Sidebar (0), Content (1)
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);
    
    // 3. Bottom Status Bar
    m_statusBar = new StatusBarPanel(this);
    mainLayout->addWidget(m_statusBar);
    
    // Connect Signals & Slots
    connect(m_sidebar, &SidebarPanel::menuSelected, this, &WindowManager::handleSidebarNavigation);
    
    connect(m_topToolbar, &TopToolbar::startRequested, this, &WindowManager::handleStartRequested);
    connect(m_topToolbar, &TopToolbar::stopRequested, this, &WindowManager::handleStopRequested);
    connect(m_topToolbar, &TopToolbar::restartRequested, this, &WindowManager::handleRestartRequested);
    
    connect(m_weston, &WestonService::stateChanged, this, &WindowManager::handleWestonStateChanged);
    
    // Connect log signals to dashboard console
    connect(m_wsl, &WSLManager::logMessage, this, &WindowManager::appendConsoleLog);
    connect(m_weston, &WestonService::logMessage, this, &WindowManager::appendConsoleLog);
    connect(m_waydroid, &WaydroidManager::logMessage, this, &WindowManager::appendConsoleLog);
    
    // Default page: Dashboard
    m_stackedWidget->setCurrentIndex(0);
}

QWidget* WindowManager::createDevicePage() {
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    
    QFrame* card = new QFrame(page);
    card->setStyleSheet("background-color: #252525; border: 1px solid #303030; border-radius: 8px;");
    card->setFixedSize(500, 320);
    
    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(16);
    
    QLabel* titleLabel = new QLabel("Android Device Emulator", card);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #4FC3F7; border: none;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    QLabel* descLabel = new QLabel("The Emulator is running as a separate frameless window to maximize display workspace.", card);
    descLabel->setStyleSheet("font-size: 13px; color: #888888; border: none;");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    
    QPushButton* btnShow = new QPushButton("Show / Bring Emulator to Front", card);
    btnShow->setStyleSheet("background-color: #4FC3F7; color: #181818; font-weight: bold; border-radius: 4px; padding: 8px 16px; border: none;");
    connect(btnShow, &QPushButton::clicked, this, &WindowManager::handleShowEmulatorWindow);
    
    QPushButton* btnClose = new QPushButton("Close Emulator Window", card);
    btnClose->setStyleSheet("background-color: #EF5350; color: #FFFFFF; font-weight: bold; border-radius: 4px; padding: 8px 16px; border: none;");
    connect(btnClose, &QPushButton::clicked, this, &WindowManager::handleStopRequested);
    
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(descLabel);
    cardLayout->addWidget(btnShow);
    cardLayout->addWidget(btnClose);
    
    layout->addWidget(card);
    return page;
}

QWidget* WindowManager::createPlaceholderPage(const QString& title) {
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);
    
    // Center Card Frame
    QFrame* card = new QFrame(page);
    card->setStyleSheet("background-color: #252525; border: 1px solid #303030; border-radius: 8px;");
    card->setFixedSize(500, 300);
    
    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(16);
    
    QLabel* titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #4FC3F7; border: none;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    QLabel* descLabel = new QLabel("This module is currently in development.\nIt will connect to background WSL service workers in a future milestone.", card);
    descLabel->setStyleSheet("font-size: 13px; color: #888888; border: none;");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(descLabel);
    
    layout->addWidget(card);
    return page;
}

void WindowManager::handleSidebarNavigation(int index) {
    if (index >= 0 && index < m_stackedWidget->count()) {
        m_stackedWidget->setCurrentIndex(index);
        
        // If user explicitly navigates to the Device tab (index 1), automatically open the window
        if (index == 1) {
            handleShowEmulatorWindow();
        }
    }
}

void WindowManager::handleStartRequested() {
    appendConsoleLog("--> User requested START emulator services.");
    
    // 1. Check if target WSL distro is installed
    if (!m_wsl->isDistroInstalled()) {
        QMessageBox::critical(this, "WSL Distro Error", 
            "Target WSL distro '" + m_wsl->targetDistro() + "' is not installed.\n"
            "Please install the distro (e.g. via 'wsl --install -d Ubuntu') before starting OctaDroid.");
        appendConsoleLog("[WSL ERROR] Target distro is not installed. Startup aborted.");
        return;
    }
    
    // 2. Boot the distro
    if (!m_wsl->startDistro()) {
        QMessageBox::warning(this, "WSL Boot Warning",
            "Failed to start WSL distro '" + m_wsl->targetDistro() + "'.");
    }
    
    // 3. Inspect kernel binder support & Waydroid installation
    bool hasBinder = m_kernel->checkGuestBinderSupport();
    bool hasWaydroid = m_kernel->checkGuestWaydroidInstalled();
    
    // Update bottom status indicators dynamically
    m_statusBar->setKernelStatus(hasBinder);
    m_statusBar->setWaydroidStatus(false); // will be set true once container and session are running
    
    // Mock other values for now
    m_statusBar->setAdbStatus(true);
    
    if (!hasWaydroid) {
        QMessageBox::critical(this, "Waydroid Missing", 
            "Waydroid is not installed on the guest WSL distribution.\n"
            "Please install Waydroid inside WSL before running.");
        appendConsoleLog("[WSL ERROR] Waydroid is not installed on the guest distro. Startup aborted.");
        return;
    }
    
    // 4. Start the Weston compositor service
    m_weston->startWeston();
    
    m_statusBar->setFps(60);
    m_statusBar->setLatency(4);
    m_statusBar->setCpuUsage(12.5);
    m_statusBar->setRamUsage(45.2);
}

void WindowManager::handleStopRequested() {
    appendConsoleLog("--> User requested STOP emulator services.");
    
    // Stop FreeRDP background client first
    if (m_rdpClient && m_rdpClient->isConnected()) {
        m_rdpClient->disconnectFromHost();
    }
    
    // Disconnect ADB first
    m_adb->disconnectFromDevice();
    
    // Stop the Waydroid container and session
    m_waydroid->stopWaydroid();
    
    // Stop the Weston compositor service
    m_weston->stopWeston();
    
    // Close the separate window if running
    if (m_emulatorWindow) {
        m_emulatorWindow->close();
        // Since we connected destroyed signal, it will clear m_emulatorWindow to nullptr
    }
    
    // Reset state
    m_statusBar->setKernelStatus(false);
    m_statusBar->setWestonStatus(false);
    m_statusBar->setWaydroidStatus(false);
    m_statusBar->setAdbStatus(false);
    
    m_statusBar->setFps(0);
    m_statusBar->setLatency(0);
    m_statusBar->setCpuUsage(0.0);
    m_statusBar->setRamUsage(0.0);
}

void WindowManager::handleRestartRequested() {
    appendConsoleLog("--> User requested RESTART emulator services.");
    handleStopRequested();
    
    // Process events to let the UI update and display stop logs before starting again
    QCoreApplication::processEvents();
    
    handleStartRequested();
}

void WindowManager::handleShowEmulatorWindow() {
    if (m_weston->state() != WestonService::WestonState::Running || !m_waydroid->isWaydroidReady()) {
        return;
    }
    
    if (!m_emulatorWindow) {
        m_emulatorWindow = new EmulatorWindow();
        m_emulatorWindow->setAttribute(Qt::WA_DeleteOnClose);
        
        connect(m_emulatorWindow, &EmulatorWindow::destroyed, this, [this]() {
            m_emulatorWindow = nullptr;
        });
        
        // Show window FIRST so OpenGL context is created before configureRenderer
        m_emulatorWindow->show();
        m_emulatorWindow->raise();
        m_emulatorWindow->activateWindow();
        
        // Configure renderer AFTER the window is shown (OpenGL context is now valid)
        m_emulatorWindow->configureRenderer(m_frameQueue, m_rdpClient);
        
        // Start FreeRDP thread LAST — frames will be pushed into the now-ready queue
        if (m_rdpClient && !m_rdpClient->isConnected()) {
            appendConsoleLog("[FreeRDP] Starting RDP probe: localhost first, then WSL2 IP...");
            
            // Connect probe result to render state so the overlay updates correctly
            connect(m_rdpClient, &FreeRDPClient::probeResult, this, [this](bool rdpReachable, const QString& ip) {
                if (!m_emulatorWindow) return;
                if (rdpReachable) {
                    appendConsoleLog(QString("[FreeRDP] RDP server reachable at %1 — streaming active.").arg(ip));
                    m_emulatorWindow->setRenderState(AndroidRenderWidget::RenderState::Streaming,
                                                     "Connected to Weston at " + ip);
                } else {
                    appendConsoleLog("[FreeRDP] RDP server unreachable — using mock frame generator.");
                    m_emulatorWindow->setRenderState(AndroidRenderWidget::RenderState::MockFallback,
                                                     "Weston RDP not reachable");
                }
            }, Qt::QueuedConnection);
            
            m_rdpClient->probeAndConnect(3390);
        }
    } else {
        m_emulatorWindow->show();
        m_emulatorWindow->raise();
        m_emulatorWindow->activateWindow();
    }
}

void WindowManager::handleWestonStateChanged(WestonService::WestonState state) {
    m_statusBar->setWestonStatus(state == WestonService::WestonState::Running);
    
    if (state == WestonService::WestonState::Running) {
        appendConsoleLog("Weston compositor is running. Checking Waydroid (auto-start configured)...");
        
        // Wait 1.5 seconds for Waydroid auto-start to settle
        QTimer::singleShot(1500, this, [this]() {
            bool waydroidReady = m_waydroid->isWaydroidReady();
            m_statusBar->setWaydroidStatus(waydroidReady);
            if (waydroidReady) {
                appendConsoleLog("Waydroid container and session are verified active.");
                m_adb->connectToDevice();
            } else {
                appendConsoleLog("[WSL ERROR] Waydroid did not start automatically. Check Weston config.");
            }
            // handleShowEmulatorWindow now starts FreeRDP internally after window is shown
            handleShowEmulatorWindow();
        });
    }
}



QWidget* WindowManager::createDashboardPage() {
    QWidget* page = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);
    
    // Top Info Section
    QLabel* headerLabel = new QLabel("System Dashboard Overview", page);
    headerLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #FFFFFF;");
    layout->addWidget(headerLabel);
    
    // Status Grid Layout (horizontal box)
    QHBoxLayout* cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(12);
    
    auto createStatusCard = [](const QString& title, const QString& detail, QWidget* parent) {
        QFrame* card = new QFrame(parent);
        card->setStyleSheet("background-color: #202020; border: 1px solid #2d2d2d; border-radius: 6px; padding: 12px;");
        QVBoxLayout* l = new QVBoxLayout(card);
        l->setContentsMargins(8, 8, 8, 8);
        l->setSpacing(4);
        
        QLabel* t = new QLabel(title, card);
        t->setStyleSheet("font-size: 11px; text-transform: uppercase; color: #888888; font-weight: bold; border: none;");
        
        QLabel* d = new QLabel(detail, card);
        d->setStyleSheet("font-size: 14px; color: #E0E0E0; font-weight: bold; border: none;");
        
        l->addWidget(t);
        l->addWidget(d);
        return card;
    };
    
    cardsLayout->addWidget(createStatusCard("WSL Distro Target", m_wsl->targetDistro(), page));
    cardsLayout->addWidget(createStatusCard("Wayland Compositor", "Weston RDP (Port 3390)", page));
    cardsLayout->addWidget(createStatusCard("Android Container", "Waydroid Subsystem", page));
    
    layout->addLayout(cardsLayout);
    
    // Bottom Log Section
    QFrame* logContainer = new QFrame(page);
    logContainer->setStyleSheet("background-color: #1a1a1a; border: 1px solid #2a2a2a; border-radius: 8px;");
    QVBoxLayout* logLayout = new QVBoxLayout(logContainer);
    logLayout->setContentsMargins(12, 12, 12, 12);
    
    QHBoxLayout* logHeader = new QHBoxLayout();
    QLabel* logTitle = new QLabel("WSL Subsystem Logs & Command History", logContainer);
    logTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #4FC3F7; border: none;");
    
    QPushButton* btnClear = new QPushButton("Clear Console", logContainer);
    btnClear->setCursor(Qt::PointingHandCursor);
    btnClear->setStyleSheet("background-color: #333333; color: #CCCCCC; border: 1px solid #444444; border-radius: 4px; padding: 4px 12px; font-size: 11px;");
    connect(btnClear, &QPushButton::clicked, this, [this]() {
        if (m_consoleLog) m_consoleLog->clear();
    });
    
    logHeader->addWidget(logTitle);
    logHeader->addStretch();
    logHeader->addWidget(btnClear);
    logLayout->addLayout(logHeader);
    
    m_consoleLog = new QTextEdit(logContainer);
    m_consoleLog->setReadOnly(true);
    m_consoleLog->setStyleSheet("background-color: #0c0c0c; color: #c8c8c8; border: 1px solid #222222; border-radius: 4px; font-family: 'Consolas', 'Courier New', monospace; font-size: 11px; padding: 6px; line-height: 140%;");
    logLayout->addWidget(m_consoleLog);
    
    layout->addWidget(logContainer, 1); // Expand to fill space
    
    // Add initial log
    appendConsoleLog("OctaDroid Dashboard loaded.");
    appendConsoleLog("Target WSL distribution: " + m_wsl->targetDistro());
    
    return page;
}

void WindowManager::appendConsoleLog(const QString& msg) {
    // Always mirror to debug output so logs appear in terminal regardless of which tab is visible
    qDebug().noquote() << "[OctaDroid]" << msg;
    
    if (!m_consoleLog) return;
    
    QString timeStr = QTime::currentTime().toString("hh:mm:ss");
    QString formattedMsg;
    
    if (msg.contains("[WSL ERROR]") || msg.contains("[WSL TIMEOUT]") || msg.contains("Timeout") || msg.contains("Failed") || msg.contains("Error")) {
        formattedMsg = QString("<span style='color: #888888;'>[%1]</span> <span style='color: #EF5350; font-weight: bold;'>%2</span>").arg(timeStr).arg(msg);
    } else if (msg.contains("[WSL SUCCESS]") || msg.contains("successfully") || msg.contains("established") || msg.contains("Running")) {
        formattedMsg = QString("<span style='color: #888888;'>[%1]</span> <span style='color: #66BB6A; font-weight: bold;'>%2</span>").arg(timeStr).arg(msg);
    } else if (msg.contains("[WSL ASYNC]") || msg.contains("[WSL EXEC]")) {
        formattedMsg = QString("<span style='color: #888888;'>[%1]</span> <span style='color: #29B6F6;'>%2</span>").arg(timeStr).arg(msg);
    } else if (msg.contains("[FreeRDP]")) {
        formattedMsg = QString("<span style='color: #888888;'>[%1]</span> <span style='color: #CE93D8;'>%2</span>").arg(timeStr).arg(msg);
    } else if (msg.contains("[ADB]")) {
        formattedMsg = QString("<span style='color: #888888;'>[%1]</span> <span style='color: #80DEEA;'>%2</span>").arg(timeStr).arg(msg);
    } else {
        formattedMsg = QString("<span style='color: #888888;'>[%1]</span> <span style='color: #E0E0E0;'>%2</span>").arg(timeStr).arg(msg);
    }
    
    m_consoleLog->append(formattedMsg);
    m_consoleLog->ensureCursorVisible();
}
