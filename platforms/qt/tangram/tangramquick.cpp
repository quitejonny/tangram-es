#include "tangramquick.h"

#include <QSGSimpleTextureNode>
#include "platform_qt.h"
#include <QDebug>
#include "qtangramgesturearea.h"
#include "qtangramgeometry.h"
#include "tangramquickrenderer.h"
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
      m_isMapReady(false),
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
        addSyncState(CenterNeedsSync);
    if (m_zoomLevel != -1)
        addSyncState(ZoomNeedsSync);
    addSyncState(PixelScaleNeedsSync | RotationNeedsSync | TiltNeedsSync);

    m_isMapReady = true;
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

    m_isMapReady = false;
    m_sceneUrl = scene;
    addSyncState(SceneConfigurationNeedsSync);

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
    addSyncState(CenterNeedsSync);

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
    addSyncState(HeadingNeedsSync);

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
    addSyncState(ZoomNeedsSync);

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
    addSyncState(TiltNeedsSync);

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
    addSyncState(RotationNeedsSync);

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
    addSyncState(PixelScaleNeedsSync);

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
        if (m_isMapReady)
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
}

void QDeclarativeTangramMap::addMapItem(QTangramGeometry *item)
{
    if (!item || item->map())
        return;
    item->setMap(this);
    m_mapItems.append(item);
}

void QDeclarativeTangramMap::addSyncState(int syncState)
{
    int oldState = m_syncState;
    m_syncState |= syncState;
    if (m_isMapReady && !oldState)
        update();
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
