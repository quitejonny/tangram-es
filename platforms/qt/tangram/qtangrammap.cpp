#include "qtangrammap.h"
#include "qtangrammapcontroller.h"
#include "tangram.h"
#include "platform_qt.h"
#include <QGeoCoordinate>
#include <QPointF>
#include <QDebug>
#include "qtangramgeometry.h"
#include "qtangrampoint.h"
#include <locale>

QTangramMap::QTangramMap(std::shared_ptr<Tangram::QtPlatform> platform, QObject *parent)
    : QObject(parent),
      m_tangramMap(0),
      m_controller(0),
      m_mutex(),
      m_scene()
{
    std::locale::global(std::locale("C"));
    m_tangramMap = new Tangram::Map(platform);
    m_controller = new QTangramMapController(this);
}

QTangramMap::~QTangramMap() {}

int QTangramMap::width() const
{
    return m_tangramMap->getViewportWidth();
}

int QTangramMap::height() const
{
    return m_tangramMap->getViewportHeight();
}

void QTangramMap::resize(int width, int height)
{
    m_tangramMap->resize(width, height);
}

void QTangramMap::setScene(QUrl scene, bool useScenePos)
{
    m_scene = scene;
    QString sceneFile = m_scene.isLocalFile() ? m_scene.toLocalFile() : m_scene.url();
    m_tangramMap->loadSceneAsync(sceneFile.toStdString().c_str(),
                                 useScenePos, std::bind(&QTangramMap::sceneChanged, this));
}

QUrl QTangramMap::scene()
{
    return m_scene;
}

void QTangramMap::setPixelScale(qreal pixelScale)
{
    if (qAbs(pixelScale - this->pixelScale()) < 1e-6)
        return;

    m_tangramMap->setPixelScale(pixelScale);
    emit pixelScaleChanged(pixelScale);
}

qreal QTangramMap::pixelScale()
{
    return (qreal)m_tangramMap->getPixelScale();
}

QTangramMapController* QTangramMap::mapController()
{
    return m_controller;
}

// std::shared_ptr<Tangram::Platform>& QTangramMap::platform()
// {
//     return std::shared_ptr<Tangram::Platform>& m_tangramMap->getPlatform();
// }

QGeoCoordinate QTangramMap::itemPositionToCoordinate(const QPointF &pos, bool clipToViewport) const
{
    Q_UNUSED(clipToViewport)
    double lat, lon;
    m_tangramMap->screenPositionToLngLat(pos.x(), pos.y(), &lon, &lat);

    return QGeoCoordinate(lat, lon);
}

QPointF QTangramMap::coordinateToItemPosition(const QGeoCoordinate &coordinate, bool clipToViewport) const
{
    Q_UNUSED(clipToViewport)
    double x, y;
    m_tangramMap->lngLatToScreenPosition(coordinate.longitude(), coordinate.latitude(), &x, &y);

    return QPointF(x, y);
}

void QTangramMap::update()
{
    emit updateRequired();
}

Tangram::Map* QTangramMap::tangramObject()
{
    return m_tangramMap;
}

void QTangramMap::setClickable(QTangramGeometry *item, bool clickable)
{
    if (clickable)
        m_clickableItems.insert(item);
    else
        m_clickableItems.remove(item);
}

void QTangramMap::setDraggable(QTangramPoint *item, bool draggable)
{
    if (draggable)
        m_draggableItems.insert(item);
    else
        m_draggableItems.remove(item);

}
