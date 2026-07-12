#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QTimer>
#include "graphics/FrameQueue.h"

class AndroidRenderWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
    Q_OBJECT
public:
    enum class RenderState {
        Idle,        // Not started yet
        Probing,     // Probing RDP server
        Streaming,   // Receiving real frames from Weston
        MockFallback // Mock frames (RDP unreachable)
    };

    explicit AndroidRenderWidget(QWidget* parent = nullptr);
    ~AndroidRenderWidget() override;

    void setFrameQueue(FrameQueue* queue);
    void startRendering();
    void stopRendering();
    void setRenderState(RenderState state, const QString& statusText = QString());

signals:
    void pointerEventOccurred(uint16_t flags, uint16_t x, uint16_t y);
    void keyboardEventOccurred(uint16_t flags, uint16_t code);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    FrameQueue* m_queue;
    GLuint m_textureId;
    GLuint m_pboIds[2];
    int m_pboIndex;
    GLuint m_programId;
    GLuint m_vao;
    GLuint m_vbo;
    QTimer* m_timer;
    int m_frameWidth;
    int m_frameHeight;
    RenderState m_renderState;
    QString m_statusText;

    void drawStatusOverlay(const QString& line1, const QString& line2 = QString(),
                           const QColor& accent = QColor("#29B6F6"));
};
