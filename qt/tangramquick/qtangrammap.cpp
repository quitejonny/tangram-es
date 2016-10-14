#include "qtangrammap.h"
#include "qtangrammapcontroller.h"
#include "tangram.h"
#include "platform_qt.h"
#include <QGeoCoordinate>
#include <QPointF>

QTangramMap::QTangramMap(QObject *parent)
    : QObject(parent),
      m_tangramMap(0),
      m_controller(0),
      m_mutex()
{
    m_tangramMap = new Tangram::Map();
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

QTangramMapController* QTangramMap::mapController()
{
    return m_controller;
}

QGeoCoordinate QTangramMap::itemPositionToCoordinate(const QPointF &pos, bool clipToViewport) const
{
    Q_UNUSED(clipToViewport)
    double *lat = (double*)malloc(sizeof(double));
    double *lon = (double*)malloc(sizeof(double));
    m_tangramMap->screenPositionToLngLat(pos.x(), pos.y(), lon, lat);

    QGeoCoordinate coordinate(*lat, *lon);
    free(lat);
    free(lon);
    return coordinate;
}

QPointF QTangramMap::coordinateToItemPosition(const QGeoCoordinate &coordinate, bool clipToViewport) const
{
    Q_UNUSED(clipToViewport)
    double *x = (double*)malloc(sizeof(double));
    double *y = (double*)malloc(sizeof(double));
    m_tangramMap->lngLatToScreenPosition(coordinate.longitude(), coordinate.latitude(), x, y);

    QPointF point(*x, *y);
    free(x);
    free(y);
    return point;
}

void QTangramMap::update()
{
    emit updateRequired();
}

Tangram::Map* QTangramMap::tangramObject()
{
    return m_tangramMap;
}
