#include "qtangrampoint.h"
#include <QDebug>
#include <QVariant>
#include "util/types.h"
#include "qtangrammap.h"
#include "tangram.h"

QTangramPointProperties::QTangramPointProperties(QObject *parent)
    : QTangramGeometryProperties(parent),
      m_size(),
      m_color()
{
    setSize(QVariant::fromValue(QStringLiteral("32px")));
    setColor(Qt::white);
    setStyle("points");
    setStyling(QStringLiteral("order"), QVariant::fromValue(2));
}

void QTangramPointProperties::setColor(const QColor &color)
{
    if (color == m_color)
        return;
    m_color = color;
    setStyling(QStringLiteral("color"), QVariant::fromValue(m_color.name(QColor::HexArgb)));
}

QColor QTangramPointProperties::color() const
{
    return m_color;
}

void QTangramPointProperties::setSize(const QVariant &size)
{
    if (size == m_size)
        return;
    m_size = size;
    setStyling(QStringLiteral("size"), size);
}

QVariant QTangramPointProperties::size() const
{
    return m_size;
}

void QTangramPointProperties::updateProperty(QString key)
{
    if (key == QStringLiteral("color")) {
        emit colorChanged();
    } else if (key == QStringLiteral("size")) {
        emit sizeChanged();
    }
}

QTangramPoint::QTangramPoint(QObject *parent)
    : QTangramGeometry(parent, new QTangramPointProperties(parent)),
      m_coordinate(),
      m_draggable(false)
{
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



QTangramPointProperties *QTangramPoint::visual()
{
    return qobject_cast<QTangramPointProperties *>(m_properties);
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
}

void QTangramPoint::setDraggable(bool draggable)
{
    if (draggable == m_draggable)
        return;

    m_draggable = draggable;
    if (m_markerId != -1)
        m_map->setDraggable(this, m_draggable);
    emit draggableChanged();
}

bool QTangramPoint::draggable()
{
    return m_draggable;
}
