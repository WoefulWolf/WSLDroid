#include "EmulatorWindow.h"
#include "ui/widgets/AndroidRenderWidget.h"
#include "ui/toolbar/EmulatorToolbar.h"
#include "graphics/FreeRDPClient.h"
#include <QHBoxLayout>
#include <QMouseEvent>

EmulatorWindow::EmulatorWindow(QWidget* parent) 
    : QWidget(parent, Qt::Window | Qt::FramelessWindowHint), m_dragging(false) {
    setWindowTitle("OctaDroid Emulator");
    setAttribute(Qt::WA_TranslucentBackground);
    
    setupUi();
}

EmulatorWindow::~EmulatorWindow() {
    if (m_renderWidget) {
        m_renderWidget->stopRendering();
    }
}

void EmulatorWindow::configureRenderer(FrameQueue* queue, FreeRDPClient* rdpClient) {
    if (m_renderWidget) {
        m_renderWidget->setFrameQueue(queue);
        m_renderWidget->startRendering();
        
        if (rdpClient) {
            connect(m_renderWidget, &AndroidRenderWidget::pointerEventOccurred, rdpClient, &FreeRDPClient::sendPointerEvent, Qt::QueuedConnection);
            connect(m_renderWidget, &AndroidRenderWidget::keyboardEventOccurred, rdpClient, &FreeRDPClient::sendKeyboardEvent, Qt::QueuedConnection);
        }
    }
}

void EmulatorWindow::setRenderState(AndroidRenderWidget::RenderState state, const QString& text) {
    if (m_renderWidget) {
        m_renderWidget->setRenderState(state, text);
    }
}

void EmulatorWindow::setupUi() {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    m_renderWidget = new AndroidRenderWidget(this);
    m_emulatorToolbar = new EmulatorToolbar(this);
    
    layout->addWidget(m_renderWidget, 1);
    layout->addWidget(m_emulatorToolbar);
    
    // Connect toolbar close & minimize actions
    connect(m_emulatorToolbar, &EmulatorToolbar::minimizeRequested, this, &EmulatorWindow::showMinimized);
    connect(m_emulatorToolbar, &EmulatorToolbar::closeRequested, this, &EmulatorWindow::close);
}

void EmulatorWindow::mousePressEvent(QMouseEvent* event) {
    // Enable dragging from the top casing title bar area (approx. 40px height)
    if (event->button() == Qt::LeftButton && event->position().y() < 40) {
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void EmulatorWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void EmulatorWindow::mouseReleaseEvent(QMouseEvent* event) {
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}
