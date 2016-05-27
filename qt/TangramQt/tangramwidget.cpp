#include <QDebug>
#include "tangramwidget.h"

#include <tangram.h>
#include <curl/curl.h>
#include <cstdlib>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include "platform_qt.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <QRunnable>

PFNGLBINDVERTEXARRAYPROC glBindVertexArrayOESEXT = 0;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysOESEXT = 0;
PFNGLGENVERTEXARRAYSPROC glGenVertexArraysOESEXT = 0;

TangramWidget::TangramWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_sceneFile("scene.yaml")
{
    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    startTimer(100);
}

TangramWidget::~TangramWidget()
{
    curl_global_cleanup();
}

void TangramWidget::timerEvent(QTimerEvent *ev)
{
    Q_UNUSED(ev);
    processNetworkQueue();
    update();
}

void TangramWidget::initializeGL()
{
    qDebug() << Q_FUNC_INFO << ".--------------";
    glBindVertexArrayOESEXT = (PFNGLBINDVERTEXARRAYPROC)context()->getProcAddress("glBindVertexArray");
    glDeleteVertexArraysOESEXT = (PFNGLDELETEVERTEXARRAYSPROC)context()->getProcAddress("glDeleteVertexArrays");
    glGenVertexArraysOESEXT = (PFNGLGENVERTEXARRAYSPROC)context()->getProcAddress("glGenVertexArrays");

    qDebug() << Q_FUNC_INFO << glBindVertexArrayOESEXT << glDeleteVertexArraysOESEXT << glGenVertexArraysOESEXT;
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    qDebug() << "Version:" << f->glGetString(GL_VERSION);
    f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
//    glBindVertexArrayOESEXT = context()->getProcAddress("glBindVertexArray");
    Tangram::initialize(m_sceneFile.fileName().toStdString().c_str());
    Tangram::setupGL();
    Tangram::resize(width(),
                    height());

    data_source = std::make_shared<Tangram::ClientGeoJsonSource>("touch", "");
    Tangram::addDataSource(data_source);
}

void TangramWidget::paintGL()
{
    QDateTime now = QDateTime::currentDateTime();
    Tangram::update(1.f);
    Tangram::render();
    m_lastRendering = now;
}

void TangramWidget::resizeGL(int w, int h)
{
    Tangram::resize(w,
                    h);
}

void TangramWidget::mousePressEvent(QMouseEvent *event)
{
    m_startPoint = event->pos();
    m_lastPoint = event->pos();
}

void TangramWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_startPoint.x() >= 0 && m_startPoint.y() >= 0) {

    }
}

void TangramWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_startPoint = QPoint(-1, -1);
    m_lastPoint = QPoint(-1, -1);
}

class UrlRunner : public QRunnable
{
public:
    UrlRunner(QUrl url, UrlCallback callback) {
        m_url = url;
        m_callback = callback;
    }

    void run() {

    }

private:
    QUrl m_url;
    UrlCallback m_callback;
};

void TangramWidget::startUrlRequest(const std::__cxx11::string &url, UrlCallback callback)
{

}

void TangramWidget::cancelUrlRequest(const std::__cxx11::string &url)
{

}

void TangramWidget::handleCallback(UrlCallback callback)
{

}
