#ifndef QTANGRAMMAPCONTROLLER_H
#define QTANGRAMMAPCONTROLLER_H

#include <QObject>
#include <QGeoCoordinate>

class QTangramMap;
namespace Tangram {
class Map;
}

class QTangramMapController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(qreal bearing READ bearing WRITE setBearing NOTIFY bearingChanged)
    Q_PROPERTY(qreal tilt READ tilt WRITE setTilt NOTIFY tiltChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

public:
    QTangramMapController(QTangramMap *map);
    ~QTangramMapController();

    QGeoCoordinate center() const;
    void setCenter(const QGeoCoordinate &center);

    void setLatitude(qreal latitude);
    void setLongitude(qreal longitude);
    void setAltitude(qreal altitude);

    qreal bearing() const;
    void setBearing(qreal bearing);

    qreal tilt() const;
    void setTilt(qreal tilt);

    qreal rotation() const;
    void setRotation(qreal rotation);

    qreal zoom() const;
    void setZoom(qreal zoom);

    void pan(qreal dx, qreal dy);

Q_SIGNALS:
    void centerChanged(const QGeoCoordinate &center);
    void bearingChanged(qreal bearing);
    void tiltChanged(qreal tilt);
    void rotationChanged(qreal rotation);
    void zoomChanged(qreal zoom);

private:
    QTangramMap *m_map;
    Tangram::Map* m_tangramObj;
};

#endif // QTANGRAMMAPCONTROLLER_H
