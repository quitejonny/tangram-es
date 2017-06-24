#include "tangramquick.h"
#include <QtGui/QOpenGLFramebufferObject>

#include <tangram.h>
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSGSimpleTextureNode>
#include "platform_qt.h"
#include <QDebug>
#include "qtangrammap.h"
#include "qtangrammapcontroller.h"
#include "qtangramgesturearea.h"
#include "qtangramgeometry.h"


QDeclarativeTangramMap::QDeclarativeTangramMap(QQuickItem *parent)
    : QQuickFramebufferObject(parent),
      m_center(),
      m_heading(0),
      m_zoomLevel(-1.0),
      m_tilt(0),
      m_rotation(0),
      m_pixelScale(1),
      m_map(new QTangramMap(parent)),
      m_gestureArea(new QTangramGestureArea(this))
{
    setAcceptedMouseButtons(Qt::AllButtons);
    registerItem((QObject*)this);

    m_gestureArea->setMap(m_map);
    connect(m_map, SIGNAL(sceneChanged()), this, SLOT(updateScene()));
    connect(m_map->mapController(), SIGNAL(centerChanged(QGeoCoordinate)),
            this, SIGNAL(centerChanged(QGeoCoordinate)));
    connect(m_map->mapController(), SIGNAL(tiltChanged(qreal)),
            this, SLOT(mapTiltChanged(qreal)));
    connect(m_map->mapController(), SIGNAL(zoomChanged(qreal)),
            this, SLOT(mapZoomLevelChanged(qreal)));
    connect(m_map->mapController(), SIGNAL(rotationChanged(qreal)),
            this, SLOT(mapRotationChanged(qreal)));
    connect(m_map, SIGNAL(pixelScaleChanged(qreal)),
            this, SLOT(mapPixelScaleChanged(qreal)));
}

QDeclarativeTangramMap::~QDeclarativeTangramMap()
{
}

QSGNode *QDeclarativeTangramMap::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
    // before Qt5.6: mirror vertically
    auto *n = static_cast<QSGSimpleTextureNode *>(QQuickFramebufferObject::updatePaintNode(node, NULL));
    n->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);

    return n;
}

void QDeclarativeTangramMap::updateScene()
{
    populateMap();

    for (auto &item : m_mapItems) {
        item->setMap(0);
        item->setMap(m_map);
    }

    if (m_center.isValid())
        m_map->mapController()->setCenter(m_center);
    if (m_zoomLevel != -1)
        m_map->mapController()->setZoom(m_zoomLevel);
    m_map->setPixelScale(m_pixelScale);
    m_map->mapController()->setRotation(m_rotation);
    m_map->mapController()->setTilt(m_tilt);
}

void QDeclarativeTangramMap::mapZoomLevelChanged(qreal zoom)
{
    if (zoom == m_zoomLevel)
        return;
    m_zoomLevel = zoom;
    emit zoomLevelChanged(m_zoomLevel);
}

void QDeclarativeTangramMap::mapTiltChanged(qreal tilt)
{
    if (tilt == m_tilt)
        return;
    m_tilt = tilt;
    emit tiltChanged(m_tilt);
}

void QDeclarativeTangramMap::mapRotationChanged(qreal rotation)
{
    if (rotation == m_rotation)
        return;
    m_rotation = rotation;
    emit rotationChanged(m_rotation);
}

void QDeclarativeTangramMap::mapPixelScaleChanged(qreal pixelScale)
{
    if (pixelScale == m_pixelScale)
        return;
    m_pixelScale = pixelScale;
    emit pixelScaleChanged(m_pixelScale);
}

QQuickFramebufferObject::Renderer* QDeclarativeTangramMap::createRenderer() const
{
    return new TangramQuickRenderer(m_map);
}

void QDeclarativeTangramMap::setSceneConfiguration(const QUrl &scene)
{
    if (m_sceneUrl != scene) {
        m_sceneUrl = scene;
        emit sceneConfigurationChanged();

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
    m_map->mapController()->setCenter(m_center);
    update();
}

QGeoCoordinate QDeclarativeTangramMap::center() const
{
    return m_center;
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
    m_map->mapController()->setZoom(zoomLevel);
    emit zoomLevelChanged(zoomLevel);
}

qreal QDeclarativeTangramMap::zoomLevel() const
{
    return m_zoomLevel;
}

void QDeclarativeTangramMap::setTilt(const qreal tilt)
{
    if (m_tilt == tilt)
        return;

    m_tilt = tilt;
    m_map->mapController()->setTilt(m_tilt);
}

qreal QDeclarativeTangramMap::tilt() const
{
    return m_tilt;
}

void QDeclarativeTangramMap::setRotation(const qreal rotation)
{
    if (m_rotation == rotation)
        return;

    m_rotation = rotation;
    m_map->mapController()->setRotation(rotation);
}

qreal QDeclarativeTangramMap::rotation() const
{
    return m_rotation;
}

void QDeclarativeTangramMap::setPixelScale(const qreal pixelScale)
{
    if (m_pixelScale == pixelScale)
        return;

    m_pixelScale = pixelScale;
    m_map->setPixelScale(m_pixelScale);
}

qreal QDeclarativeTangramMap::pixelScale() const
{
    return m_pixelScale;
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
        update();
        return true;
    }
    return QQuickItem::event(e);
}

void QDeclarativeTangramMap::mousePressEvent(QMouseEvent *event)
{
    m_gestureArea->handleMousePressEvent(event);
}

void QDeclarativeTangramMap::mouseMoveEvent(QMouseEvent *event)
{
    m_gestureArea->handleMouseMoveEvent(event);
}

void QDeclarativeTangramMap::mouseReleaseEvent(QMouseEvent *event)
{
    m_gestureArea->handleMouseReleaseEvent(event);
}

void QDeclarativeTangramMap::mouseUngrabEvent()
{
    m_gestureArea->handleMouseUngrabEvent();
}

void QDeclarativeTangramMap::touchUngrabEvent()
{
    m_gestureArea->handleTouchUngrabEvent();
}

void QDeclarativeTangramMap::touchEvent(QTouchEvent *event)
{
    m_gestureArea->handleTouchEvent(event);
}

void QDeclarativeTangramMap::wheelEvent(QWheelEvent *event)
{
    m_gestureArea->handleWheelEvent(event);
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
    if (!item)
        return;

    item->setMap(0);
    m_mapItems.removeOne(item);
    //emit mapItemsChanged();
}

void QDeclarativeTangramMap::addMapItem(QTangramGeometry *item)
{
    if (!item || item->map())
        return;
    item->setMap(m_map);
    m_mapItems.append(item);
    // emit mapItemsChanged();
}

void QDeclarativeTangramMap::populateMap()
{
    qDebug() << Q_FUNC_INFO;
    QObjectList kids = children();
    for (auto &kid : kids) {
        auto *mapItem = qobject_cast<QTangramGeometry *>(kid);
        if (mapItem)
            addMapItem(mapItem);
    }
}

void QDeclarativeTangramMap::componentComplete()
{
    QQuickItem::componentComplete();
}

TangramQuickRenderer::TangramQuickRenderer(QTangramMap *map)
    : QQuickFramebufferObject::Renderer(),
    m_glInitialized(false),
    m_useScenePosition(false),
    m_map(map)
{
}

TangramQuickRenderer::~TangramQuickRenderer()
{
}

void TangramQuickRenderer::initializeGL()
{
    m_program.reset(new QOpenGLShaderProgram);
    m_program->link();

    auto f = QOpenGLContext::currentContext()->functions();
    auto context = QOpenGLContext::currentContext();
    setQtGlFunctions(context);

    m_dataSource = std::make_shared<Tangram::ClientGeoJsonSource>("touch", "");

    qDebug() << Q_FUNC_INFO << "Version:" << f->glGetString(GL_VERSION);

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
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    {
        QMutexLocker locker(&m_map->m_mutex);
        m_map->tangramObject()->update((float)m_elapsedTimer.elapsed() / 100.f);
        m_map->tangramObject()->render();
    m_elapsedTimer.restart();

    f->glActiveTexture(GL_TEXTURE0);
    update();
    }
}

void TangramQuickRenderer::synchronize(QQuickFramebufferObject *item)
{
    QDeclarativeTangramMap *quickMap = static_cast<QDeclarativeTangramMap*>(item);
    if (!quickMap)
        return;

    m_useScenePosition = !(quickMap->m_center.isValid() && quickMap->m_zoomLevel != -1);

    m_sceneUrl = quickMap->sceneConfiguration();
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
