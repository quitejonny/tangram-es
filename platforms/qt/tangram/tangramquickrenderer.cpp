#include "tangramquickrenderer.h"
#include "tangramquick.h"
#include <QtGui/QOpenGLFramebufferObject>

#include "map.h"
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include "platform_qt.h"
#include <QDebug>
#include "qtangramgesturearea.h"
#include "qtangrammarkermanager.h"
#include <QQuickWindow>


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
    m_map->update((float)m_elapsedTimer.elapsed() / 1000.f);

    m_map->render();
    m_elapsedTimer.restart();

    if (m_platform->isContinuousRendering())
        update();

    m_item->window()->resetOpenGLState();
}

void TangramQuickRenderer::synchronize(QQuickFramebufferObject *item)
{
    QDeclarativeTangramMap *map = static_cast<QDeclarativeTangramMap*>(item);

    if (!m_initialized) {
        m_item =  item;
        m_platform->setDownloader(map->m_downloader);
        m_platform->setItem(map);
        m_markerManager = new QTangramMarkerManager(m_map, this);
        connect(map, &QDeclarativeTangramMap::updateSceneSignal,
                this, &TangramQuickRenderer::updateScene);
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
        m_map->loadSceneAsync(sceneFile.toStdString(), true);
        m_map->setSceneReadyListener([&](Tangram::SceneID id, const Tangram::SceneError* error) {
            emit this->sceneChanged();
        });
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

    if (map->m_isMapReady) {
        for (auto &marker : map->m_changedItems)
            m_markerManager->sync(marker);
        map->m_changedItems.clear();
        m_markerManager->syncDrag();
    }

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
    map->m_isUpdateRequested = false;

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
    format.setSamples(4);
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    return new QOpenGLFramebufferObject(size, format);
}

void TangramQuickRenderer::updateScene(const QString path, const QString value)
{
    std::vector<Tangram::SceneUpdate> updates;
    updates.push_back({path.toStdString(), value.toStdString()});
    m_map->updateSceneAsync(updates);
}
