#include "tangramquick.h"
#include <QtGui/QOpenGLFramebufferObject>
#include <QQuickWindow>

#include <tangram.h>
#include <curl/curl.h>
#include <cstdlib>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include "platform_qt.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <QRunnable>
#include <QDebug>

PFNGLBINDVERTEXARRAYPROC glBindVertexArrayOESEXT = 0;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysOESEXT = 0;
PFNGLGENVERTEXARRAYSPROC glGenVertexArraysOESEXT = 0;

TangramQuick::TangramQuick(QQuickItem *parent):
    QQuickFramebufferObject(parent)
{
    registerItem((QObject*)this);
    setMirrorVertically(true);
}

TangramQuick::~TangramQuick()
{
}

QQuickFramebufferObject::Renderer* TangramQuick::createRenderer() const
{
    return new TangramQuickRenderer();
}

void TangramQuick::setSceneConfiguration(const QUrl scene)
{
    if (m_sceneUrl != scene) {
        m_sceneUrl = scene;
        emit sceneConfigurationChanged();
        update();
    }
}

QUrl TangramQuick::sceneConfiguration() const
{
    return m_sceneUrl;
}

bool TangramQuick::event(QEvent *e)
{
    if (e->type() == TANGRAM_REQ_RENDER_EVENT_TYPE) {
        processNetworkQueue();
        update();
        return true;
    }
    return QQuickItem::event(e);
}

TangramQuickRenderer::TangramQuickRenderer()
    : QQuickFramebufferObject::Renderer()
    , m_glInitialized(false)
    , m_item(NULL)
{
    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

TangramQuickRenderer::~TangramQuickRenderer()
{
    curl_global_cleanup();
}

void TangramQuickRenderer::initializeGL()
{
    qDebug() << Q_FUNC_INFO << ".--------------";
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    glBindVertexArrayOESEXT = (PFNGLBINDVERTEXARRAYPROC)glctx->getProcAddress("glBindVertexArray");
    glDeleteVertexArraysOESEXT = (PFNGLDELETEVERTEXARRAYSPROC)glctx->getProcAddress("glDeleteVertexArrays");
    glGenVertexArraysOESEXT = (PFNGLGENVERTEXARRAYSPROC)glctx->getProcAddress("glGenVertexArrays");

    qDebug() << Q_FUNC_INFO << glBindVertexArrayOESEXT << glDeleteVertexArraysOESEXT << glGenVertexArraysOESEXT;
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    qDebug() << "Version:" << f->glGetString(GL_VERSION);
    f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    Tangram::initialize(m_sceneUrl.fileName().toStdString().c_str());
    Tangram::setupGL();

    m_dataSource = std::make_shared<Tangram::ClientGeoJsonSource>("touch", "");
    Tangram::addDataSource(m_dataSource);
}

void TangramQuickRenderer::render()
{
    qDebug() << Q_FUNC_INFO;
    Tangram::update((float)m_elapsedTimer.elapsed() / 1000.f);
    Tangram::render();    
    m_elapsedTimer.restart();

    if (m_item)
        m_item->window()->resetOpenGLState();
}

void TangramQuickRenderer::synchronize(QQuickFramebufferObject *item)
{
    TangramQuick *tangramItem = static_cast<TangramQuick*>(item);
    if (tangramItem) {
        m_sceneUrl = tangramItem->sceneConfiguration();
    }
    m_item = (QQuickItem*)item;
}

QOpenGLFramebufferObject* TangramQuickRenderer::createFramebufferObject(const QSize &size)
{
    qDebug() << Q_FUNC_INFO << size;
    if (Q_UNLIKELY(!m_glInitialized)) {
        initializeGL();
        m_glInitialized = true;
    }
    Tangram::resize(size.width(),
                    size.height());


    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    return new QOpenGLFramebufferObject(size, format);
}
