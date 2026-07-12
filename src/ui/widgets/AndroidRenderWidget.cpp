#include "AndroidRenderWidget.h"
#include <QPainter>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

AndroidRenderWidget::AndroidRenderWidget(QWidget* parent)
    : QOpenGLWidget(parent), m_queue(nullptr), m_textureId(0), m_pboIndex(0),
      m_programId(0), m_vao(0), m_vbo(0), m_frameWidth(0), m_frameHeight(0),
      m_renderState(RenderState::Idle) {
    setMinimumSize(320, 520);
    setFocusPolicy(Qt::StrongFocus);
    m_pboIds[0] = 0;
    m_pboIds[1] = 0;
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
}

AndroidRenderWidget::~AndroidRenderWidget() {
    makeCurrent();
    if (m_textureId) glDeleteTextures(1, &m_textureId);
    if (m_pboIds[0]) glDeleteBuffers(2, m_pboIds);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_programId) glDeleteProgram(m_programId);
    doneCurrent();
}

void AndroidRenderWidget::setFrameQueue(FrameQueue* queue) {
    m_queue = queue;
}

void AndroidRenderWidget::setRenderState(RenderState state, const QString& statusText) {
    m_renderState = state;
    m_statusText = statusText;
    if (state != RenderState::Streaming && state != RenderState::MockFallback) {
        m_frameWidth = 0;
        m_frameHeight = 0;
    }
    update();
}

void AndroidRenderWidget::startRendering() {
    m_frameWidth = 0;
    m_frameHeight = 0;
    m_renderState = RenderState::Probing;
    m_statusText = "Probing Weston RDP server...";
    m_timer->start(16); // ~60 FPS
}

void AndroidRenderWidget::stopRendering() {
    m_timer->stop();
    m_frameWidth = 0;
    m_frameHeight = 0;
    m_renderState = RenderState::Idle;
    m_statusText.clear();
    update();
}

void AndroidRenderWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    const char* vertexShaderSource =
        "#version 330 core\n"
        "layout(location = 0) in vec2 position;\n"
        "layout(location = 1) in vec2 texCoords;\n"
        "out vec2 vTexCoords;\n"
        "void main() {\n"
        "    vTexCoords = texCoords;\n"
        "    gl_Position = vec4(position, 0.0, 1.0);\n"
        "}\n";

    const char* fragmentShaderSource =
        "#version 330 core\n"
        "in vec2 vTexCoords;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D screenTexture;\n"
        "void main() {\n"
        "    fragColor = texture(screenTexture, vec2(vTexCoords.x, 1.0 - vTexCoords.y));\n"
        "}\n";

    auto compileShader = [this](GLenum type, const char* source) -> GLuint {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint logLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
            if (logLen > 0) {
                QByteArray log; log.resize(logLen);
                glGetShaderInfoLog(shader, logLen, nullptr, log.data());
                qWarning() << "Shader error:" << log;
            }
        }
        return shader;
    };

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    m_programId = glCreateProgram();
    glAttachShader(m_programId, vs);
    glAttachShader(m_programId, fs);
    glLinkProgram(m_programId);
    glDeleteShader(vs);
    glDeleteShader(fs);

    float vertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenBuffers(2, m_pboIds);
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[i]);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, 1080 * 1920 * 4, nullptr, GL_STREAM_DRAW);
    }
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void AndroidRenderWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void AndroidRenderWidget::paintGL() {
    bool hasNewFrame = false;
    Frame frame;

    if (m_queue) {
        hasNewFrame = m_queue->pop(frame, 0); // Non-blocking: 0ms timeout
    }

    if (hasNewFrame) {
        m_frameWidth = frame.width;
        m_frameHeight = frame.height;

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pboIds[m_pboIndex]);
        glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, frame.width * frame.height * 4, frame.pixels.constData());
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.width, frame.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        m_pboIndex = (m_pboIndex + 1) % 2;
        glGetError();
    }

    if (m_frameWidth > 0 && m_frameHeight > 0) {
        // Render the actual frame via shader
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(m_programId);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
        glBindVertexArray(m_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    } else {
        // No frame yet — show status overlay based on current render state
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        switch (m_renderState) {
            case RenderState::Probing:
                drawStatusOverlay("Connecting to Weston",
                                  m_statusText.isEmpty() ? "Probing RDP server on localhost:3390..." : m_statusText,
                                  QColor("#29B6F6")); // Blue
                break;
            case RenderState::MockFallback:
                drawStatusOverlay("Mock Frame Generator Active",
                                  "Weston RDP unreachable — RDP integration pending",
                                  QColor("#FFA726")); // Orange
                break;
            case RenderState::Idle:
            default:
                drawStatusOverlay("OctaDroid",
                                  "Start Weston compositor to begin streaming",
                                  QColor("#546E7A")); // Grey-blue
                break;
        }
    }
}

void AndroidRenderWidget::drawStatusOverlay(const QString& line1, const QString& line2, const QColor& accent) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();

    // Dark background gradient
    QLinearGradient bg(0, 0, 0, h);
    bg.setColorAt(0.0, QColor("#0D1117"));
    bg.setColorAt(1.0, QColor("#161B22"));
    painter.fillRect(0, 0, w, h, bg);

    int cx = w / 2;
    int cy = h / 2 - 24;
    int r = 32;

    // Glow halo
    QRadialGradient glow(cx, cy, r * 2.5);
    glow.setColorAt(0.0, QColor(accent.red(), accent.green(), accent.blue(), 55));
    glow.setColorAt(1.0, Qt::transparent);
    painter.setBrush(glow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(cx - r * 3, cy - r * 3, r * 6, r * 6);

    // Accent ring
    painter.setPen(QPen(accent, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(cx - r, cy - r, r * 2, r * 2);

    // Center dot
    painter.setBrush(accent);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(cx - 4, cy - 4, 8, 8);

    // Title
    painter.setPen(QColor("#E6EDF3"));
    painter.setFont(QFont("Segoe UI", 10, QFont::Medium));
    painter.drawText(QRect(16, cy + r + 18, w - 32, 26), Qt::AlignHCenter | Qt::AlignVCenter, line1);

    // Subtitle
    if (!line2.isEmpty()) {
        painter.setPen(QColor("#8B949E"));
        painter.setFont(QFont("Segoe UI", 8));
        painter.drawText(QRect(16, cy + r + 48, w - 32, 20), Qt::AlignHCenter | Qt::AlignVCenter, line2);
    }

    // Bottom accent pill
    painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 70));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(cx - 36, h - 20, 72, 4, 2, 2);
}

void AndroidRenderWidget::mousePressEvent(QMouseEvent* event) {
    if (m_frameWidth > 0 && m_frameHeight > 0 && event->button() == Qt::LeftButton) {
        double scaleX = (double)m_frameWidth / width();
        double scaleY = (double)m_frameHeight / height();
        uint16_t rdpX = static_cast<uint16_t>(event->position().x() * scaleX);
        uint16_t rdpY = static_cast<uint16_t>(event->position().y() * scaleY);
        emit pointerEventOccurred(0x1000 | 0x8000, rdpX, rdpY);
    }
}

void AndroidRenderWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (m_frameWidth > 0 && m_frameHeight > 0 && event->button() == Qt::LeftButton) {
        double scaleX = (double)m_frameWidth / width();
        double scaleY = (double)m_frameHeight / height();
        uint16_t rdpX = static_cast<uint16_t>(event->position().x() * scaleX);
        uint16_t rdpY = static_cast<uint16_t>(event->position().y() * scaleY);
        emit pointerEventOccurred(0x1000, rdpX, rdpY);
    }
}

void AndroidRenderWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_frameWidth > 0 && m_frameHeight > 0) {
        double scaleX = (double)m_frameWidth / width();
        double scaleY = (double)m_frameHeight / height();
        uint16_t rdpX = static_cast<uint16_t>(event->position().x() * scaleX);
        uint16_t rdpY = static_cast<uint16_t>(event->position().y() * scaleY);
        uint16_t flags = 0x0800;
        if (event->buttons() & Qt::LeftButton) flags |= 0x1000 | 0x8000;
        emit pointerEventOccurred(flags, rdpX, rdpY);
    }
}

void AndroidRenderWidget::keyPressEvent(QKeyEvent* event) {
    emit keyboardEventOccurred(0x0000, static_cast<uint16_t>(event->nativeScanCode()));
}

void AndroidRenderWidget::keyReleaseEvent(QKeyEvent* event) {
    emit keyboardEventOccurred(0x8000, static_cast<uint16_t>(event->nativeScanCode()));
}
