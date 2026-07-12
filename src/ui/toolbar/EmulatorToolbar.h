#pragma once
#include <QFrame>

class EmulatorToolbar : public QFrame {
    Q_OBJECT
public:
    explicit EmulatorToolbar(QWidget* parent = nullptr);

signals:
    void minimizeRequested();
    void closeRequested();
    void powerPressed();
    void volumeUpPressed();
    void volumeDownPressed();
    void rotateLeftPressed();
    void rotateRightPressed();
    void screenshotPressed();
    void fullscreenPressed();
    void morePressed();

private:
    void setupUi();
};
