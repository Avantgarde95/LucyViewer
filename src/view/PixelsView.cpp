#include <QOpenGLFunctions>

#include "PixelsView.hpp"

PixelsView::PixelsView(QWidget *parent) : QOpenGLWidget(parent) {
    auto format_ = format();
    format_.setRenderableType(QSurfaceFormat::OpenGL);
    format_.setVersion(3, 3);
    format_.setProfile(QSurfaceFormat::CoreProfile);
    format_.setOption(QSurfaceFormat::DebugContext);
    format_.setSwapBehavior(QSurfaceFormat::SingleBuffer);
    setFormat(format_);

    setMinimumSize(100, 100);
}

PixelsView::~PixelsView() {
    makeCurrent();
    delete m_program;
    delete m_vao;
    delete m_vbo;
    delete m_ibo;
    delete m_texture;
    doneCurrent();
}

const glm::ivec2 &PixelsView::getViewSize() const {
    return m_viewSize;
}

const glm::ivec2 &PixelsView::getTextureSize() const {
    return m_textureSize;
}

void PixelsView::setPixels(const void *pixels) {
    m_texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, pixels);
}

void PixelsView::initializeGL() {
    auto gl = QOpenGLContext::currentContext()->functions();

    gl->glEnable(GL_DEPTH);
    gl->glDepthFunc(GL_LESS);
    gl->glEnable(GL_CULL_FACE);
    gl->glCullFace(GL_BACK);

    m_program->addShaderFromSourceCode(
            QOpenGLShader::Vertex,
            R"(#version 330 core

struct Vertex {
    vec2 position;
    vec2 uv;
};

layout(location = 0) in vec2 v_vertexPosition;
layout(location = 1) in vec2 v_vertexUV;

out Vertex f_vertex;

void main() {
    Vertex vertex = Vertex(v_vertexPosition, v_vertexUV);

    gl_Position = vec4(vertex.position, 0.0, 1.0);
    f_vertex = vertex;
})"
    );

    m_program->addShaderFromSourceCode(
            QOpenGLShader::Fragment,
            R"(#version 330 core

struct Vertex {
    vec2 position;
    vec2 uv;
};

uniform sampler2D u_texture;

in Vertex f_vertex;

layout(location = 0) out vec3 glFragColor;

void main() {
    glFragColor = texture(u_texture, f_vertex.uv).rgb;
})"
    );

    m_program->link();

    m_vao->create();
    m_vao->bind();
    {
        m_vbo->create();
        m_vbo->bind();

        float x = 1.0f;
        float y = 1.0f;

        GLfloat vertices[] = {
                -x, -y, 0.0f, 0.0f,
                x, -y, 1.0f, 0.0f,
                -x, y, 0.0f, 1.0f,
                x, y, 1.0f, 1.0f
        };

        m_vbo->allocate(vertices, static_cast<int>(sizeof(vertices)));
        m_program->enableAttributeArray(0);
        m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2, sizeof(GLfloat) * 4);
        m_program->enableAttributeArray(1);
        m_program->setAttributeBuffer(1, GL_FLOAT, sizeof(GLfloat) * 2, 2, sizeof(GLfloat) * 4);
    }
    {
        m_ibo->create();
        m_ibo->bind();

        GLuint indices[] = {
                0, 1, 3,
                0, 3, 2
        };

        m_ibo->allocate(indices, static_cast<int>(sizeof(indices)));
    }

    resetTexture();
}

void PixelsView::paintGL() {
    auto gl = QOpenGLContext::currentContext()->functions();

    gl->glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    gl->glClear(
            static_cast<unsigned int>(GL_COLOR_BUFFER_BIT)
            | static_cast<unsigned int>(GL_DEPTH_BUFFER_BIT)
    );

    m_program->bind();
    m_texture->bind(0);
    m_program->setUniformValue("u_texture", 0);

    gl->glViewport(0, 0, m_viewSize.x, m_viewSize.y);
    gl->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}

void PixelsView::resizeGL(int width, int height) {
    m_viewSize = {width, height};

    /*
    int sizeLimit = 800;

    if (m_viewSize.x >= sizeLimit && m_viewSize.x >= m_viewSize.y) {
        m_textureSize = {sizeLimit, m_viewSize.y * sizeLimit / m_viewSize.x};
    } else if (m_viewSize.y >= sizeLimit && m_viewSize.y >= m_viewSize.x) {
        m_textureSize = {m_viewSize.x * sizeLimit / m_viewSize.y, sizeLimit};
    } else {
        m_textureSize = m_viewSize;
    }
    */

    m_textureSize = m_viewSize;

    resetTexture();
}

QSize PixelsView::sizeHint() const {
    return {600, 500};
}

void PixelsView::resetTexture() {
    if (m_texture->isCreated()) {
        m_texture->destroy();
    }

    m_texture->create();
    m_texture->setSize(m_textureSize.x, m_textureSize.y);
    m_texture->setFormat(QOpenGLTexture::RGBA32F);
    m_texture->allocateStorage();
}

