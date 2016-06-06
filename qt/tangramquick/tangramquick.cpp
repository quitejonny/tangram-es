#include "tangramquick.h"
#include <QtGui/QOpenGLFramebufferObject>
#include <QQuickWindow>

#include <tangram.h>
#include <gl/renderState.h>
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

TangramQuick::TangramQuick(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
    , m_position(40.70532700869127, -74.00976419448854)
    , m_moveAnimationDuration(1000)
    , m_heading(0)
    , m_lastMousePos(-1, -1)
    , m_panning(false)
    , m_glInit(false)
{
    setAcceptedMouseButtons(Qt::AllButtons);
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

void TangramQuick::setPosition(const QPointF position)
{
    if (m_position != position) {
        m_position = position;
        emit positionChanged();
        update();
    }
}

QPointF TangramQuick::position() const
{
    return m_position;
}

void TangramQuick::setMoveAnimationDuration(const int duration)
{
    if (m_moveAnimationDuration != duration) {
        m_moveAnimationDuration = duration;
        emit moveAnimationDurationChanged();
        update();
    }
}

void TangramQuick::setHeading(const qreal heading)
{
    if (m_heading != heading) {
        m_heading = heading;
        emit headingChanged();
        update();
    }
}

qreal TangramQuick::heading() const
{
    return m_heading;
}

void TangramQuick::setZoom(const float zoom)
{
    if (m_zoom != zoom) {
        m_zoom = zoom;
        emit zoomChanged();
    }
}

float TangramQuick::zoom() const
{
    return m_zoom;
}

void TangramQuick::setTilt(const float tilt)
{
    if (m_tilt != tilt) {
        m_tilt = tilt;
        emit tiltChanged();
    }
}

float TangramQuick::tilt() const
{
    return m_tilt;
}

int TangramQuick::moveAnimationDuration() const
{
    return m_moveAnimationDuration;
}

void TangramQuick::setContinuousRendering(const bool continuousRendering)
{
    qDebug() << Q_FUNC_INFO << continuousRendering;
    if (m_continuousRendering != continuousRendering) {
        m_continuousRendering = continuousRendering;
        emit continuousRenderingChanged();
    }
}

bool TangramQuick::continuousRendering() const
{
    qDebug() << Q_FUNC_INFO << m_continuousRendering;
    return m_continuousRendering;
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

void TangramQuick::mousePressEvent(QMouseEvent *event)
{
    if (Q_UNLIKELY(!m_glInit))
        return;
    Tangram::handlePanGesture(0.0f, 0.0f, 0.0f, 0.0f);
    m_lastMousePos = event->pos();
    m_lastMouseEvent = event->timestamp();
}

void TangramQuick::mouseMoveEvent(QMouseEvent *event)
{
    if (Q_UNLIKELY(!m_glInit))
        return;
    Tangram::handlePanGesture(m_lastMousePos.x(), m_lastMousePos.y(),
                              event->x(), event->y());

    if (m_panning && (event->timestamp() - m_lastMouseEvent) != 0) {
        m_lastMouseSpeed.setX((event->x() - m_lastMousePos.x()) / ((float)(event->timestamp() - m_lastMouseEvent) / 1000.f));
        m_lastMouseSpeed.setY((event->y() - m_lastMousePos.y()) / ((float)(event->timestamp() - m_lastMouseEvent) / 1000.f));
    }

    m_lastMousePos = event->pos();
    m_lastMouseEvent = event->timestamp();
    m_panning = true;
}

void TangramQuick::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_panning) {
        Tangram::handleFlingGesture(event->x(), event->y(),
                                    qBound(-2000.0, m_lastMouseSpeed.x(), 2000.0),
                                    qBound(-2000.0, m_lastMouseSpeed.y(), 2000.0));
        m_panning = false;
        m_lastMouseSpeed.setX(0.);
        m_lastMouseSpeed.setY(0.);
    }
}

void TangramQuick::setGLInitialized(bool init)
{
    m_glInit = init;
}

void TangramQuick::queueSceneUpdate(const QString path, const QString value)
{
    Tangram::queueSceneUpdate(path.toStdString().c_str(),
                              value.toStdString().c_str());
}

void TangramQuick::applySceneUpdates()
{
    Tangram::applySceneUpdates();
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
    Tangram::RenderState::configure2();
    Tangram::update((float)m_elapsedTimer.elapsed() / 100.f);
    Tangram::render();
    m_elapsedTimer.restart();
}

void TangramQuickRenderer::synchronize(QQuickFramebufferObject *item)
{
    TangramQuick *tangramItem = static_cast<TangramQuick*>(item);
    if (tangramItem) {
        m_sceneUrl = tangramItem->sceneConfiguration();
        m_moveAnimationDuration = tangramItem->moveAnimationDuration();

        if (m_position != tangramItem->position()) {
            m_position = tangramItem->position();
            if (m_glInitialized)
                Tangram::setPosition(m_position.y(), m_position.x());
        }
        if (m_heading != tangramItem->heading()) {
            m_heading = tangramItem->heading();
            if (m_glInitialized)
                Tangram::setRotation(m_heading * (2*M_PI / 360.f));
        }
        if (m_zoom != tangramItem->zoom()) {
            m_zoom = tangramItem->zoom();
            if (m_glInitialized)
                Tangram::setZoom(m_zoom);
        }
        if (m_tilt != tangramItem->tilt()) {
            m_tilt = tangramItem->tilt();
            if (m_glInitialized)
                Tangram::setTilt(m_tilt * (2*M_PI / 360.f));
        }

        tangramItem->setGLInitialized(m_glInitialized);
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
