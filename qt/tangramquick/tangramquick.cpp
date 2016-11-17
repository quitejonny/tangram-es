#include "tangramquick.h"
#include <QtGui/QOpenGLFramebufferObject>
#include <QQuickWindow>
#include <QMutexLocker>

#include <tangram.h>
#include <curl/curl.h>
#include <cstdlib>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include "platform_qt.h"
#include <QRunnable>
#include <QDebug>
#include "qtangrammap.h"
#include "qtangrammapcontroller.h"
#include "qtangramgesturearea.h"
#include "qtangramgeometry.h"


QDeclarativeTangramMap::QDeclarativeTangramMap(QQuickItem *parent)
    : QQuickFramebufferObject(parent),
      m_center(),
      m_moveAnimationDuration(1000),
      m_heading(0),
      m_zoomLevel(-1.0),
      m_map(0),
      m_gestureArea(new QTangramGestureArea(this)),
      m_tangramMapInitialized(false)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    registerItem((QObject*)this);
    setMirrorVertically(true);
}

QDeclarativeTangramMap::~QDeclarativeTangramMap()
{
}

void QDeclarativeTangramMap::setMap(QTangramMap *map)
{
    if (m_map)
        return;
    m_map = map;
    m_gestureArea->setMap(map);
    connect(m_map, SIGNAL(sceneChanged()), this, SLOT(populateMap()));
    connect(m_map->mapController(), SIGNAL(centerChanged(QGeoCoordinate)), this, SIGNAL(centerChanged(QGeoCoordinate)));
    connect(m_map->mapController(), SIGNAL(tiltChanged(qreal)), this, SIGNAL(tiltChanged(qreal)));
    connect(m_map->mapController(), SIGNAL(zoomChanged(qreal)), this, SLOT(setZoomLevel(qreal)));
    if (m_center.isValid())
        m_map->mapController()->setCenter(m_center);
    if (m_zoomLevel != -1)
        m_map->mapController()->setZoom(m_zoomLevel);
    m_tangramMapInitialized = true;
}

QQuickFramebufferObject::Renderer* QDeclarativeTangramMap::createRenderer() const
{
    return new TangramQuickRenderer();
}

void QDeclarativeTangramMap::setSceneConfiguration(const QUrl scene)
{
    if (m_sceneUrl != scene) {
        m_sceneUrl = scene;
        emit sceneConfigurationChanged();
        if (m_tangramMapInitialized) {
            if (m_center.isValid())
                m_map->mapController()->setCenter(m_center);
            if (m_zoomLevel != -1)
                m_map->mapController()->setZoom(m_zoomLevel);
        }
        update();
    }
}

QUrl QDeclarativeTangramMap::sceneConfiguration() const
{
    return m_sceneUrl;
}

void QDeclarativeTangramMap::setCenter(const QGeoCoordinate &center)
{
    if (m_center == center || !center.isValid())
        return;

    m_center = center;
    if (m_tangramMapInitialized)
        m_map->mapController()->setCenter(m_center);
    update();
}

QGeoCoordinate QDeclarativeTangramMap::center() const
{
    return m_center;
}

void QDeclarativeTangramMap::setMoveAnimationDuration(const int duration)
{
    if (m_moveAnimationDuration != duration) {
        m_moveAnimationDuration = duration;
        emit moveAnimationDurationChanged();
        update();
    }
}

void QDeclarativeTangramMap::setHeading(const qreal heading)
{
    if (m_heading != heading) {
        m_heading = heading;
        emit headingChanged();
        update();
    }
}

qreal QDeclarativeTangramMap::heading() const
{
    return m_heading;
}

void QDeclarativeTangramMap::setZoomLevel(qreal zoomLevel)
{
    if (m_zoomLevel == zoomLevel || zoomLevel < 0)
        return;

    m_zoomLevel = zoomLevel;
    if (m_tangramMapInitialized)
        m_map->mapController()->setZoom(zoomLevel);
    emit zoomLevelChanged(zoomLevel);
}

qreal QDeclarativeTangramMap::zoomLevel() const
{
    return m_zoomLevel;
}

void QDeclarativeTangramMap::setTilt(const float tilt)
{
    if (m_tilt == tilt)
        return;

    m_tilt = tilt;
    if (m_tangramMapInitialized)
        m_map->mapController()->setTilt(m_tilt);
}

float QDeclarativeTangramMap::tilt() const
{
    return m_tilt;
}

int QDeclarativeTangramMap::moveAnimationDuration() const
{
    return m_moveAnimationDuration;
}

void QDeclarativeTangramMap::setContinuousRendering(const bool continuousRendering)
{
    if (m_continuousRendering != continuousRendering) {
        m_continuousRendering = continuousRendering;
        emit continuousRenderingChanged(continuousRendering);
    }
}

bool QDeclarativeTangramMap::continuousRendering() const
{
    return m_continuousRendering;
}

bool QDeclarativeTangramMap::event(QEvent *e)
{
    if (e->type() == TANGRAM_REQ_RENDER_EVENT_TYPE) {
        processNetworkQueue();
        update();
        return true;
    }
    return QQuickItem::event(e);
}

void QDeclarativeTangramMap::mousePressEvent(QMouseEvent *event)
{
    if (m_map)
        m_gestureArea->handleMousePressEvent(event);
    else
        QQuickFramebufferObject::mousePressEvent(event);
}

void QDeclarativeTangramMap::mouseMoveEvent(QMouseEvent *event)
{
    if (m_map)
        m_gestureArea->handleMouseMoveEvent(event);
    else
        QQuickFramebufferObject::mouseMoveEvent(event);
}

void QDeclarativeTangramMap::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_map) {
        m_gestureArea->handleMouseReleaseEvent(event);
        ungrabMouse();
    } else {
        QQuickFramebufferObject::mouseReleaseEvent(event);
    }
}

void QDeclarativeTangramMap::mouseUngrabEvent()
{
    if (m_map)
        m_gestureArea->handleMouseUngrabEvent();
    else
        QQuickFramebufferObject::mouseUngrabEvent();
}

void QDeclarativeTangramMap::touchUngrabEvent()
{
    if (m_map) {
        m_gestureArea->handleTouchUngrabEvent();
    } else {
        QQuickFramebufferObject::mouseUngrabEvent();
    }
}

void QDeclarativeTangramMap::touchEvent(QTouchEvent *event)
{
    if (m_map)
        m_gestureArea->handleTouchEvent(event);
    else
        QQuickFramebufferObject::touchEvent(event);
}

void QDeclarativeTangramMap::wheelEvent(QWheelEvent *event)
{
    if (m_map)
        m_gestureArea->handleWheelEvent(event);
    else
        QQuickFramebufferObject::wheelEvent(event);
}

void QDeclarativeTangramMap::queueSceneUpdate(const QString path, const QString value)
{
    m_map->tangramObject()->queueSceneUpdate(path.toStdString().c_str(),
                              value.toStdString().c_str());
}

void QDeclarativeTangramMap::applySceneUpdates()
{
    m_map->tangramObject()->applySceneUpdates();
}

void QDeclarativeTangramMap::removeMapItem(QTangramGeometry *item)
{
    if (!item || !m_map)
        return;

    item->setMap(0);
    //emit mapItemsChanged();
}

void QDeclarativeTangramMap::addMapItem(QTangramGeometry *item)
{
    qDebug() << Q_FUNC_INFO;
    if (!item || item->map())
        return;
    if (m_map)
        item->setMap(m_map);
    // emit mapItemsChanged();
}

void QDeclarativeTangramMap::populateMap()
{
    qDebug() << Q_FUNC_INFO;
    if (!m_map)
        return;
    QObjectList kids = children();
    for (auto &kid : kids) {
        auto *mapItem = qobject_cast<QTangramGeometry *>(kid);
        if (mapItem)
            addMapItem(mapItem);
    }
}

void QDeclarativeTangramMap::componentComplete()
{
    populateMap();
    QQuickItem::componentComplete();
}

TangramQuickRenderer::TangramQuickRenderer()
    : QQuickFramebufferObject::Renderer(),
    m_glInitialized(false),
    m_useScenePosition(false),
    m_map(0)
{
    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    setQtGlFunctions(QOpenGLContext::currentContext());
    m_map = new QTangramMap();

}

TangramQuickRenderer::~TangramQuickRenderer()
{
    curl_global_cleanup();
}

void TangramQuickRenderer::initializeGL()
{
    m_dataSource = std::make_shared<Tangram::ClientGeoJsonSource>("touch", "");
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    qDebug() << Q_FUNC_INFO << "Version:" << f->glGetString(GL_VERSION);
    f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    {
        QMutexLocker locker(&m_map->m_mutex);
        m_map->setScene(m_sceneUrl, m_useScenePosition);
        m_map->tangramObject()->setupGL();
        m_map->tangramObject()->addDataSource(m_dataSource);
    }

    m_glInitialized = true;
}

void TangramQuickRenderer::render()
{
    {
        QMutexLocker locker(&m_map->m_mutex);
        m_map->tangramObject()->update((float)m_elapsedTimer.elapsed() / 100.f);
        //m_map->update(100.f);
        m_map->tangramObject()->render();
    }
    m_elapsedTimer.restart();
}

void TangramQuickRenderer::synchronize(QQuickFramebufferObject *item)
{
    QDeclarativeTangramMap *quickMap = static_cast<QDeclarativeTangramMap*>(item);
    if (!quickMap)
        return;

    if (!quickMap->m_map && m_map) {
        quickMap->setMap(m_map);
    }

    m_useScenePosition = !(quickMap->m_center.isValid() && quickMap->m_zoomLevel != -1);

    m_sceneUrl = quickMap->sceneConfiguration();
    m_moveAnimationDuration = quickMap->moveAnimationDuration();

}

QOpenGLFramebufferObject* TangramQuickRenderer::createFramebufferObject(const QSize &size)
{
    qDebug() << Q_FUNC_INFO << size;
    if (!m_glInitialized)
        initializeGL();
    {
        QMutexLocker locker(&m_map->m_mutex);
        m_map->resize(size.width(), size.height());
    }

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    return new QOpenGLFramebufferObject(size, format);
}
