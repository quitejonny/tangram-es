#ifndef QTANGRAMMARKERMANAGER_H
#define QTANGRAMMARKERMANAGER_H

#include <QObject>
#include <QSet>
#include <QGeoCoordinate>

namespace Tangram {
class Map;
}

class QDeclarativeTangramMap;
class QTangramGeometry;

class QTangramMarkerManager : public QObject
{
    Q_OBJECT
public:
    explicit QTangramMarkerManager(Tangram::Map *map, QObject *parent = 0);

    void sync(QTangramGeometry *marker);
    void syncDrag();

    void setBitmap(QTangramGeometry *marker);
    void remove(int markerId);

signals:
    void startDrag();

public slots:
    void tryToClick(qreal posX, qreal posY);
    void tryToDrag(qreal posX, qreal posY);
    void drag(qreal posX, qreal posY);
    void endDrag();

private:
    Tangram::Map *m_map;
    QSet<int> m_clickableMarkers;
    QSet<int> m_draggableMarkers;
    QHash<int, QTangramGeometry *> m_markers;

    struct Drag
    {
        Drag() : m_id(0), m_latitudeDistance(0), m_longitudeDistance(0) {}
        int m_id;
        qreal m_latitudeDistance;
        qreal m_longitudeDistance;
        qreal m_deltaX;
        qreal m_deltaY;
        QGeoCoordinate position;

    } m_drag;
};

#endif // QTANGRAMMARKERMANAGER_H
