#include "qtangrampoint.h"
#include <QDebug>
#include <QVariant>
#include <QtMath>
#include "util/types.h"
#include "tangramquick.h"
#include "map.h"

QTangramPoint::QTangramPoint(QObject *parent)
    : QTangramGeometry(parent),
      m_coordinate(),
      m_draggable(false)
{
    m_defaultStyling["size"] = "32px";
    m_defaultStyling["style"] = "points";
}

QTangramPoint::~QTangramPoint()
{
}

void QTangramPoint::setCoordinate(const QGeoCoordinate &coordinate)
{
    if (!coordinate.isValid() || coordinate == m_coordinate)
        return;

    m_coordinate = coordinate;
    addSyncState(PointNeedsSync);

    emit coordinateChanged();
}

QGeoCoordinate QTangramPoint::coordinate() const
{
    return m_coordinate;
}

void QTangramPoint::setMap(QDeclarativeTangramMap *map)
{
    if (map && !m_imageSource.isEmpty())
        addSyncState(BitmapNeedsSync);

    if (map && m_draggable)
        addSyncState(DraggableNeedsSync);

    if (map && m_coordinate.isValid())
        addSyncState(PointNeedsSync);

    QTangramGeometry::setMap(map);
}

bool QTangramPoint::isInteractive()
{
    return m_clickable || m_draggable;
}

void QTangramPoint::setDraggable(bool draggable)
{
    if (draggable == m_draggable)
        return;

    bool interactive = isInteractive();
    m_draggable = draggable;
    addSyncState(DraggableNeedsSync);
    emit draggableChanged();

    if (interactive != isInteractive()) {
        m_defaultStyling["interactive"] = isInteractive();
        setTangramStyling();
    }
}

bool QTangramPoint::draggable() const
{
    return m_draggable;
}

void QTangramPoint::setImageSource(const QString &imageSource)
{
    if (m_imageSource == imageSource)
        return;

    m_imageSource = imageSource;
    addSyncState(BitmapNeedsSync);

    emit imageSourceChanged();
}

QString QTangramPoint::imageSource() const
{
    return m_imageSource;
}
