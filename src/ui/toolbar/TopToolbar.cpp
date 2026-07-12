#include "TopToolbar.h"
#include "styles/StyleHelper.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QLineEdit>

TopToolbar::TopToolbar(QWidget* parent) : QFrame(parent) {
    setObjectName("TopToolbarFrame");
    setStyleSheet(StyleHelper::getTopToolbarStyleSheet());
    
    setupUi();
}

void TopToolbar::setupUi() {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(6);
    
    // Create standard action buttons
    QToolButton* btnStart = new QToolButton(this);
    btnStart->setText("Start");
    btnStart->setToolTip("Start Android Container & Services");
    connect(btnStart, &QToolButton::clicked, this, &TopToolbar::startRequested);
    
    QToolButton* btnStop = new QToolButton(this);
    btnStop->setText("Stop");
    btnStop->setToolTip("Stop Android Container & Services");
    connect(btnStop, &QToolButton::clicked, this, &TopToolbar::stopRequested);
    
    QToolButton* btnRestart = new QToolButton(this);
    btnRestart->setText("Restart");
    btnRestart->setToolTip("Restart Android Container & Services");
    connect(btnRestart, &QToolButton::clicked, this, &TopToolbar::restartRequested);
    
    QToolButton* btnInstall = new QToolButton(this);
    btnInstall->setText("Install APK");
    btnInstall->setToolTip("Install an Android Application package");
    connect(btnInstall, &QToolButton::clicked, this, &TopToolbar::installApkRequested);
    
    QToolButton* btnRefresh = new QToolButton(this);
    btnRefresh->setText("Refresh");
    btnRefresh->setToolTip("Refresh connection and status indicators");
    connect(btnRefresh, &QToolButton::clicked, this, &TopToolbar::refreshRequested);
    
    QToolButton* btnSettings = new QToolButton(this);
    btnSettings->setText("Settings");
    btnSettings->setToolTip("Configure OctaDroid settings");
    connect(btnSettings, &QToolButton::clicked, this, &TopToolbar::settingsRequested);
    
    layout->addWidget(btnStart);
    layout->addWidget(btnStop);
    layout->addWidget(btnRestart);
    layout->addWidget(btnInstall);
    layout->addWidget(btnRefresh);
    layout->addWidget(btnSettings);
    
    layout->addStretch();
    
    // Search Box
    QLineEdit* searchBox = new QLineEdit(this);
    searchBox->setObjectName("SearchBox");
    searchBox->setPlaceholderText("Search apps or settings...");
    connect(searchBox, &QLineEdit::textChanged, this, &TopToolbar::searchTextChanged);
    layout->addWidget(searchBox);
}
