#pragma once
#include <QWidget>
#include <QPoint>
#include "AndroidRenderWidget.h"

class AndroidRenderWidget;
class EmulatorToolbar;
class FrameQueue;
class FreeRDPClient;

class EmulatorWindow : public QWidget {
    Q_OBJECT
public:
    explicit EmulatorWindow(QWidget* parent = nullptr);
    ~EmulatorWindow() override;

    void configureRenderer(FrameQueue* queue, FreeRDPClient* rdpClient);
    void setRenderState(AndroidRenderWidget::RenderState state, const QString& text = QString());

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    AndroidRenderWidget* m_renderWidget;
    EmulatorToolbar* m_emulatorToolbar;
    bool m_dragging;
    QPoint m_dragPosition;
    
    void setupUi();
};
