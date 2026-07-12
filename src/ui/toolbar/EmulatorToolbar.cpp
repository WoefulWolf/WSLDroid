#include "EmulatorToolbar.h"
#include "styles/StyleHelper.h"
#include <QVBoxLayout>
#include <QToolButton>

EmulatorToolbar::EmulatorToolbar(QWidget* parent) : QFrame(parent) {
    setObjectName("EmulatorToolbarFrame");
    setStyleSheet(StyleHelper::getEmulatorToolbarStyleSheet());
    
    setupUi();
}

void EmulatorToolbar::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 10, 0, 10);
    layout->setSpacing(6);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    
    // Action details mapping: text label, tooltip, signal connection
    struct ToolButtonData {
        QString text;
        QString tooltip;
        void (EmulatorToolbar::*signal)();
    };
    
    QList<ToolButtonData> buttonDataList = {
        {"—", "Minimize Window", &EmulatorToolbar::minimizeRequested},
        {"✕", "Close Emulator", &EmulatorToolbar::closeRequested},
        {"⏻", "Power Button", &EmulatorToolbar::powerPressed},
        {"＋", "Volume Up", &EmulatorToolbar::volumeUpPressed},
        {"－", "Volume Down", &EmulatorToolbar::volumeDownPressed},
        {"📷", "Take Screenshot", &EmulatorToolbar::screenshotPressed},
        {"🔍", "Zoom Screen", &EmulatorToolbar::fullscreenPressed},   // Mock signal
        {"↺", "Rotate Left", &EmulatorToolbar::rotateLeftPressed},
        {"↻", "Rotate Right", &EmulatorToolbar::rotateRightPressed},
        {"◁", "Android Back", &EmulatorToolbar::rotateLeftPressed},   // Mock signal
        {"◯", "Android Home", &EmulatorToolbar::rotateRightPressed},  // Mock signal
        {"☐", "Android Recents", &EmulatorToolbar::screenshotPressed}, // Mock signal
        {"⋯", "More Settings", &EmulatorToolbar::morePressed}
    };
    
    for (const auto& data : buttonDataList) {
        QToolButton* btn = new QToolButton(this);
        btn->setText(data.text);
        btn->setToolTip(data.tooltip);
        
        // Use a slightly different font for unicode symbols to render correctly on Segoe UI
        QFont f = btn->font();
        f.setPointSize(12);
        btn->setFont(f);
        
        connect(btn, &QToolButton::clicked, this, data.signal);
        layout->addWidget(btn);
    }
}
