#include "qtangrampoint.h"
#include <QDebug>
#include <QVariant>
#include <QtMath>
#include <QImage>
#include "util/types.h"
#include "qtangrammap.h"
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
    emit coordinateChanged();
    if (m_markerId == -1)
        return;

    auto lngLat = Tangram::LngLat(coordinate.longitude(), coordinate.latitude());
    m_tangramMap->markerSetPoint(m_markerId, lngLat);
}

QGeoCoordinate QTangramPoint::coordinate() const
{
    return m_coordinate;
}

void QTangramPoint::initGeometry()
{
    if (!m_coordinate.isValid())
        return;

    auto lngLat = Tangram::LngLat(m_coordinate.longitude(), m_coordinate.latitude());
    m_tangramMap->markerSetPoint(m_markerId, lngLat);
}

void QTangramPoint::setMap(QTangramMap *map)
{
    if (!m_map && map && m_draggable)
        map->setDraggable(this, m_draggable);
    else if (m_map && !map)
        m_map->setDraggable(this, false);

    QTangramGeometry::setMap(map);
    setImageData();
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
    if (m_markerId != -1)
        m_map->setDraggable(this, m_draggable);
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
    emit imageSourceChanged();
    setImageData();
}

QString QTangramPoint::imageSource() const
{
    return m_imageSource;
}

void QTangramPoint::setImageData()
{
    if (m_markerId == -1)
        return;

    QString source = m_imageSource.startsWith("qrc:") ? m_imageSource.remove(0, 3) : m_imageSource;
    if (source.isEmpty())
            return;

    QImage image(source);
    if (image.isNull())
        return;

    image = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    const uint *data = reinterpret_cast<const uint*>(image.constBits());
    m_tangramMap->markerSetBitmap(m_markerId, image.width(), image.height(), data);
}
