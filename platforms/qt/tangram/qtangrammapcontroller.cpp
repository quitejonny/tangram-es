#include "qtangrammapcontroller.h"
#include "qtangrammap.h"
#include "tangram.h"
#include <QDebug>

QTangramMapController::QTangramMapController(QTangramMap *map)
    : QObject(map),
      m_map(map),
      m_tangramObj(0)
{
    m_tangramObj = m_map->tangramObject();
}

QTangramMapController::~QTangramMapController() {}

void QTangramMapController::setCenter(const QGeoCoordinate &center)
{
    qDebug() << Q_FUNC_INFO;
    if (center == this->center())
        return;

    m_tangramObj->setPosition(center.longitude(), center.latitude());
    emit centerChanged(center);

}

QGeoCoordinate QTangramMapController::center() const
{
    double lat;
    double lon;
    m_tangramObj->getPosition(lon, lat);

    return QGeoCoordinate(lat, lon);
}

void QTangramMapController::setZoom(qreal zoom)
{
    if (qAbs(zoom - this->zoom()) < 1e-6)
        return;

    m_tangramObj->setZoom(zoom);
    emit zoomChanged(zoom);
}

qreal QTangramMapController::zoom() const
{
    return (qreal)m_tangramObj->getZoom();
}

void QTangramMapController::setBearing(qreal bearing)
{
    if (bearing == this->bearing())
        return;

    m_tangramObj->setRotation(bearing);
    emit bearingChanged(bearing);
}

qreal QTangramMapController::bearing() const
{
    return (qreal)m_tangramObj->getRotation();
}

void QTangramMapController::setTilt(qreal tilt)
{
    if (tilt == this->tilt())
        return;

    m_tangramObj->setTilt(tilt);
    emit tiltChanged(tilt);
}

qreal QTangramMapController::tilt() const
{
    return (qreal)m_tangramObj->getTilt();
}

void QTangramMapController::pan(qreal dx, qreal dy)
{
    m_tangramObj->handlePanGesture(0, 0, dx, dy);
}

void QTangramMapController::setLatitude(qreal latitude)
{
    double lat;
    double lon;
    m_tangramObj->getPosition(lon, lat);
    if ((qreal)lat == latitude)
        return;

    m_tangramObj->setPosition(lon, latitude);
    emit centerChanged(this->center());
}

void QTangramMapController::setLongitude(qreal longitude)
{
    double lat;
    double lon;
    m_tangramObj->getPosition(lon, lat);
    if ((qreal)lon == longitude)
        return;

    m_tangramObj->setPosition(longitude, lat);
    emit centerChanged(this->center());
}

void QTangramMapController::setAltitude(qreal altitude)
{
    Q_UNUSED(altitude)
    return;
}

void QTangramMapController::setRotation(qreal rotation)
{
    if (qAbs(rotation - this->rotation()) < 1e-6)
        return;

    m_tangramObj->setRotation(rotation);
    emit rotationChanged(rotation);
}

qreal QTangramMapController::rotation() const
{
    return (qreal)m_tangramObj->getRotation();
}
