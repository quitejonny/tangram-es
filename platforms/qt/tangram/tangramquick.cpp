#include "tangramquick.h"
#include <QtGui/QOpenGLFramebufferObject>

#include "map.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSGSimpleTextureNode>
#include "platform_qt.h"
#include <QDebug>
#include "qtangramgesturearea.h"
#include "qtangramgeometry.h"
#include "qtangrampoint.h"
#include "qtangrammarkermanager.h"
#include "contentdownloader.h"


QDeclarativeTangramMap::QDeclarativeTangramMap(QQuickItem *parent)
    : QQuickFramebufferObject(parent),
      m_zoomLevel(-1.0),
      m_center(),
      m_sceneUrl(),
      m_heading(0),
      m_tilt(0),
      m_rotation(0),
      m_pixelScale(1),
      m_gestureArea(new QTangramGestureArea(this)),
      m_downloader(new ContentDownloader(this))
{
    setAcceptedMouseButtons(Qt::AllButtons);
    connect(this, &QDeclarativeTangramMap::sceneChanged, this, &QDeclarativeTangramMap::updateScene);
}

QDeclarativeTangramMap::~QDeclarativeTangramMap()
{
}

QTangramGestureArea *QDeclarativeTangramMap::gesture()
{
    return m_gestureArea;
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
        item->setMap(this);
    }

    if (m_center.isValid())
        m_syncState |= CenterNeedsSync;
    if (m_zoomLevel != -1)
        m_syncState |= ZoomNeedsSync;
    m_syncState |= PixelScaleNeedsSync | RotationNeedsSync | TiltNeedsSync;
    update();
}

QQuickFramebufferObject::Renderer* QDeclarativeTangramMap::createRenderer() const
{
    return new TangramQuickRenderer();
}

void QDeclarativeTangramMap::setSceneConfiguration(const QUrl &scene)
{
    if (m_sceneUrl == scene)
        return;

    m_sceneUrl = scene;
    m_syncState |= SceneConfigurationNeedsSync;
    update();

    emit sceneConfigurationChanged();
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
    m_syncState |= CenterNeedsSync;
    update();

    emit centerChanged(m_center);
}

QGeoCoordinate QDeclarativeTangramMap::center() const
{
    return m_center;
}

void QDeclarativeTangramMap::setHeading(const qreal heading)
{
    if (m_heading == heading)
        return;

    m_heading = heading;
    m_syncState |= HeadingNeedsSync;
    update();

    emit headingChanged();
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
    m_syncState |= ZoomNeedsSync;
    update();

    emit zoomLevelChanged(m_zoomLevel);
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
    m_syncState |= TiltNeedsSync;
    update();

    emit tiltChanged(m_tilt);
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
    m_syncState |= RotationNeedsSync;
    update();

    emit rotationChanged(m_rotation);
}

qreal QDeclarativeTangramMap::rotation() const
{
    return m_rotation;
}

void QDeclarativeTangramMap::setPixelScale(const qreal pixelScale)
{
    if (qAbs(pixelScale - m_pixelScale) < 1e-6)
        return;

    m_pixelScale = pixelScale;
    m_syncState |= PixelScaleNeedsSync;
    update();

    emit pixelScaleChanged(m_pixelScale);
}

qreal QDeclarativeTangramMap::pixelScale() const
{
    return m_pixelScale;
}

void QDeclarativeTangramMap::itemchangedData(QTangramGeometry *item)
{
    if (!m_changedItems.contains(item)) {
        m_changedItems.insert(item);
        update();
    }
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

bool QDeclarativeTangramMap::event(QEvent *e)
{
    if (e->type() == TANGRAM_REQ_RENDER_EVENT_TYPE) {
        update();
        return true;
    }
    return QQuickItem::event(e);
}

void QDeclarativeTangramMap::queueSceneUpdate(const QString path, const QString value)
{
    emit queueSceneUpdateSignal(path, value);
}

void QDeclarativeTangramMap::applySceneUpdates()
{
    emit applySceneUpdatesSignal();
}

void QDeclarativeTangramMap::removeMapItem(QTangramGeometry *item)
{
    if (!item)
        return;

    item->setMap(0);
    if (m_changedItems.contains(item))
        m_changedItems.remove(item);

    if (m_mapItems.contains(item)) {
        m_mapItems.removeOne(item);
        m_removedMarkerIds.append(item->markerId());
    }
    //emit mapItemsChanged();
}

void QDeclarativeTangramMap::addMapItem(QTangramGeometry *item)
{
    if (!item || item->map())
        return;
    item->setMap(this);
    m_mapItems.append(item);
    // emit mapItemsChanged();
}

void QDeclarativeTangramMap::populateMap()
{
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

TangramQuickRenderer::TangramQuickRenderer(QQuickItem *mapItem)
    : QObject(mapItem),
      QQuickFramebufferObject::Renderer(),
      m_initialized(false),
      m_glInitialized(false),
      m_platform(std::make_shared<Tangram::QtPlatform>()),
      m_map(new Tangram::Map(m_platform))
{
    auto f = QOpenGLContext::currentContext()->functions();
    auto context = QOpenGLContext::currentContext();
    Tangram::setQtGlFunctions(context);

    qDebug() << Q_FUNC_INFO << "Version:" << f->glGetString(GL_VERSION);

    m_map->setupGL();
}

TangramQuickRenderer::~TangramQuickRenderer()
{
}

void TangramQuickRenderer::initMap()
{
}

void TangramQuickRenderer::render()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    m_map->update((float)m_elapsedTimer.elapsed() / 1000.f);
    m_map->render();
    m_elapsedTimer.restart();

    if (m_platform->isContinuousRendering())
        update();

    f->glActiveTexture(GL_TEXTURE0);
}

void TangramQuickRenderer::synchronize(QQuickFramebufferObject *item)
{
    QDeclarativeTangramMap *map = static_cast<QDeclarativeTangramMap*>(item);

    if (!m_initialized) {
        m_platform->setDownloader(map->m_downloader);
        m_platform->setItem(map);
        m_markerManager = new QTangramMarkerManager(m_map, this);
        connect(map, &QDeclarativeTangramMap::queueSceneUpdateSignal,
                this, &TangramQuickRenderer::queueSceneUpdate);
        connect(map, &QDeclarativeTangramMap::applySceneUpdatesSignal,
                this, &TangramQuickRenderer::applySceneUpdates);
        connect(this, &TangramQuickRenderer::sceneChanged,
                map, &QDeclarativeTangramMap::sceneChanged);
        connect(m_markerManager, &QTangramMarkerManager::startDrag,
                map->m_gestureArea, &QTangramGestureArea::startDrag);

        m_initialized = true;
    }

    auto syncState = map->m_syncState;

    if (syncState & QDeclarativeTangramMap::ZoomNeedsSync) {
        m_map->setZoom(map->m_zoomLevel);
    }

    if (syncState & QDeclarativeTangramMap::CenterNeedsSync) {
        m_map->setPosition(map->m_center.longitude(), map->m_center.latitude());
    }

    if (syncState & QDeclarativeTangramMap::SceneConfigurationNeedsSync) {

        QUrl scene = map->m_sceneUrl;
        QString sceneFile = scene.isLocalFile() ? scene.toLocalFile() : scene.url();
        m_map->loadSceneAsync(sceneFile.toStdString().c_str(),
                              true, std::bind(&TangramQuickRenderer::sceneChanged, this));
    }

    if (syncState & QDeclarativeTangramMap::HeadingNeedsSync) {

    }

    if (syncState & QDeclarativeTangramMap::TiltNeedsSync) {
        m_map->setTilt(map->m_tilt);
    }

    if (syncState & QDeclarativeTangramMap::RotationNeedsSync) {
        m_map->setRotation(map->m_rotation);
    }

    if (syncState & QDeclarativeTangramMap::PixelScaleNeedsSync) {
        m_map->setPixelScale(map->m_pixelScale);
    }

    map->m_syncState = QDeclarativeTangramMap::NothingNeedsSync;

    for (auto &marker : map->m_changedItems)
        m_markerManager->sync(marker);
    map->m_changedItems.clear();
    m_markerManager->syncDrag();

    for (auto &markerId : map->m_removedMarkerIds)
        m_markerManager->remove(markerId);


    int gestureState = map->m_gestureArea->m_syncState;
    map->m_gestureArea->m_syncState = QTangramGestureArea::NothingNeedsSync;
    auto gestureArea = map->m_gestureArea;

    if (gestureState & QTangramGestureArea::PanNeedsSync) {
        m_map->handlePanGesture(gestureArea->m_touchPointsCentroidLast.x(),
                                gestureArea->m_touchPointsCentroidLast.y(),
                                gestureArea->m_touchPointsCentroid.x(),
                                gestureArea->m_touchPointsCentroid.y());
        m_syncState |= CenterNeedsSync;
    }

    if (gestureState & QTangramGestureArea::FlingNeedsSync) {
        m_map->handleFlingGesture(gestureArea->m_touchPointsCentroidLast.x(),
                                  gestureArea->m_touchPointsCentroidLast.y(),
                                  gestureArea->m_flick.m_vector.x(),
                                  gestureArea->m_flick.m_vector.y());
    }

    if (gestureState & QTangramGestureArea::PinchNeedsSync) {
        m_map->handlePinchGesture(gestureArea->m_touchPointsCentroid.x(),
                                  gestureArea->m_touchPointsCentroid.y(),
                                  gestureArea->m_pinch.m_scale, 0.f);
        m_syncState |= ZoomNeedsSync;
    }

    if (gestureState & QTangramGestureArea::RotateNeedsSync) {
        m_map->handleRotateGesture(gestureArea->m_touchPointsCentroid.x(),
                                   gestureArea->m_touchPointsCentroid.y(),
                                   gestureArea->m_pinch.m_rotation.m_angle);
        m_syncState |= RotationNeedsSync;
    }

    if (gestureState & QTangramGestureArea::ShoveNeedsSync) {
        m_map->handleShoveGesture(gestureArea->m_pinch.m_tilt.m_verticalDisplacement);
        m_syncState |= TiltNeedsSync;
    }

    if (gestureState & QTangramGestureArea::TryClickNeedsSync) {
        m_markerManager->tryToClick(gestureArea->m_touchPointsCentroid.x(),
                                    gestureArea->m_touchPointsCentroid.y());
    }

    if (gestureState & QTangramGestureArea::TryDragNeedsSync) {
        m_markerManager->tryToDrag(gestureArea->m_touchPointsCentroid.x(),
                                   gestureArea->m_touchPointsCentroid.y());
    }

    if (gestureState & QTangramGestureArea::DragNeedsSync) {
        m_markerManager->drag(gestureArea->m_touchPointsCentroid.x(),
                              gestureArea->m_touchPointsCentroid.y());
    }

    if (gestureState & QTangramGestureArea::EndDragNeedsSync) {
        m_markerManager->endDrag();
    }

    if (gestureState & QTangramGestureArea::StopPointNeedsSync) {
        m_map->handlePanGesture(0, 0, 0, 0);
    }

    syncTo(map);

}

void TangramQuickRenderer::syncTo(QDeclarativeTangramMap *map)
{
    int syncState = popSyncState();

    if (syncState & TangramQuickRenderer::ZoomNeedsSync) {
        map->m_zoomLevel = m_map->getZoom();
        emit map->zoomLevelChanged(map->m_zoomLevel);
    }

    if (syncState & TangramQuickRenderer::CenterNeedsSync) {
        double lon, lat;
        m_map->getPosition(lon, lat);
        map->m_center.setLongitude(lon);
        map->m_center.setLatitude(lat);
        emit map->centerChanged(map->m_center);

    }
    if (syncState & TangramQuickRenderer::TiltNeedsSync) {
        map->m_tilt = m_map->getTilt();
        map->tiltChanged(map->m_tilt);
    }

    if (syncState & TangramQuickRenderer::RotationNeedsSync) {
        map->m_rotation = m_map->getRotation();
        map->rotationChanged(map->m_rotation);
    }
}

int TangramQuickRenderer::popSyncState()
{
    int state = m_syncState;
    m_syncState = NothingNeedsSync;
    return state;
}

QGeoCoordinate TangramQuickRenderer::itemPositionToCoordinate(const QPointF &pos) const
{
    double lat, lon;
    m_map->screenPositionToLngLat(pos.x(), pos.y(), &lon, &lat);

    return QGeoCoordinate(lat, lon);
}

QPointF TangramQuickRenderer::coordinateToItemPosition(const QGeoCoordinate &coordinate) const
{
    double x, y;
    m_map->lngLatToScreenPosition(coordinate.longitude(), coordinate.latitude(), &x, &y);

    return QPointF(x, y);
}

QOpenGLFramebufferObject* TangramQuickRenderer::createFramebufferObject(const QSize &size)
{
    m_map->resize(size.width(), size.height());

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    return new QOpenGLFramebufferObject(size, format);
}

void TangramQuickRenderer::queueSceneUpdate(const QString path, const QString value)
{
    m_map->queueSceneUpdate(path.toStdString().c_str(), value.toStdString().c_str());
}

void TangramQuickRenderer::applySceneUpdates()
{
    m_map->applySceneUpdates();
}
