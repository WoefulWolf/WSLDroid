#pragma once
#include <QFrame>
#include <QButtonGroup>

class SidebarPanel : public QFrame {
    Q_OBJECT
public:
    explicit SidebarPanel(QWidget* parent = nullptr);

signals:
    void menuSelected(int index);

private:
    QButtonGroup* m_buttonGroup;
    void setupUi();
};
