#pragma once
#include <QFrame>

class TopToolbar : public QFrame {
    Q_OBJECT
public:
    explicit TopToolbar(QWidget* parent = nullptr);

signals:
    void startRequested();
    void stopRequested();
    void restartRequested();
    void installApkRequested();
    void refreshRequested();
    void settingsRequested();
    void searchTextChanged(const QString& text);

private:
    void setupUi();
};
