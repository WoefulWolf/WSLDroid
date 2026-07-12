#include "SidebarPanel.h"
#include "styles/StyleHelper.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

SidebarPanel::SidebarPanel(QWidget* parent) : QFrame(parent) {
    setObjectName("SidebarFrame");
    setStyleSheet(StyleHelper::getSidebarStyleSheet());
    setFixedWidth(280);
    
    setupUi();
}

void SidebarPanel::setupUi() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Header title
    QLabel* titleLabel = new QLabel("OctaDroid", this);
    titleLabel->setObjectName("SidebarTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // Button Group for exclusive checks
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
    
    // Menu items
    QStringList menuItems = {
        "Dashboard", "Device", "APK", "Applications", 
        "Files", "Logcat", "Terminal", "Settings", "About"
    };
    
    QVBoxLayout* btnLayout = new QVBoxLayout();
    btnLayout->setContentsMargins(0, 10, 0, 10);
    btnLayout->setSpacing(4);
    
    for (int i = 0; i < menuItems.size(); ++i) {
        QPushButton* btn = new QPushButton(menuItems[i], this);
        btn->setCheckable(true);
        if (i == 0) btn->setChecked(true); // Default to Dashboard
        
        m_buttonGroup->addButton(btn, i);
        btnLayout->addWidget(btn);
    }
    
    layout->addLayout(btnLayout);
    
    // Bottom spacer
    layout->addStretch();
    
    // Connect group button clicked to emission of page change signal
    connect(m_buttonGroup, &QButtonGroup::idClicked, this, &SidebarPanel::menuSelected);
}
