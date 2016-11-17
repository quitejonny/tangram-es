#include "qtangrampolyline.h"
#include "qtangrammap.h"
#include "tangram.h"
#include "util/types.h"
#include <QJSValue>
#include <QGeoCoordinate>
#include <QQmlEngine>
#include <QQmlContext>
#include <QColor>
#include <QDebug>

QTangramPolylineProperties::QTangramPolylineProperties(QObject *parent)
    : QTangramGeometryProperties(parent),
      m_width(),
      m_color()
{
    setColor(QColor(Qt::black));
    setWidth(2.0);
    setStyle(QStringLiteral("lines"));
    setStyling(QStringLiteral("order"), QVariant::fromValue(2));
}

void QTangramPolylineProperties::setColor(const QColor &color)
{
    if (color == m_color)
        return;
    m_color = color;
    setStyling(QStringLiteral("color"), QVariant::fromValue(color.name(QColor::HexArgb)));
}

QColor QTangramPolylineProperties::color() const
{
    return m_color;
}

void QTangramPolylineProperties::setWidth(qreal width)
{
    if (width == m_width)
        return;
    m_width = width;
    setStyling(QStringLiteral("width"), QVariant::fromValue(QString::number(width) + "px"));
}

qreal QTangramPolylineProperties::width() const
{
    return m_width;
}

void QTangramPolylineProperties::updateProperty(QString key)
{
    qDebug() << Q_FUNC_INFO;
    if (key == QStringLiteral("width")) {
        emit widthChanged(m_width);
    } else if (key == QStringLiteral("color")) {
        emit colorChanged(m_color);
    }
}

QTangramPolyline::QTangramPolyline(QObject *parent)
    : QTangramGeometry(parent)
{
    m_properties = new QTangramPolylineProperties(parent);
    connect(m_properties, SIGNAL(stylingChanged()), this, SLOT(setStyling()));
}

QTangramPolyline::~QTangramPolyline()
{
}

void QTangramPolyline::setPath(const QJSValue &value)
{
    qDebug() << Q_FUNC_INFO;
    if (!value.isArray())
        return;

    QList<QGeoCoordinate> pathList;
    quint32 length = value.property(QStringLiteral("length")).toUInt();
    for (quint32 i = 0; i < length; ++i) {
        bool ok;
        QGeoCoordinate c = parseCoordinate(value.property(i), &ok);

        if (!ok || !c.isValid()) {
            qDebug() << "Unsupported path type";
            return;
        }

        pathList.append(c);
    }

    setPathFromGeoList(pathList);
}

QJSValue QTangramPolyline::path() const
{
    QQmlEngine* engine = QQmlEngine::contextForObject(this)->engine();
    QJSValue array = engine->newArray(m_path.count());

    int validCount = 0;
    for (const auto &coordinate : m_path) {
        if (coordinate.isValid())
            ++validCount;
    }

    for (int i = 0; i < m_path.count(); ++i) {
        const QGeoCoordinate &coordinate = m_path.at(i);
        if (coordinate.isValid()) {
            QJSValue obj = engine->newObject();
            obj.setProperty("latitude", QJSValue(coordinate.latitude()));
            obj.setProperty("longitude", QJSValue(coordinate.longitude()));
            if (coordinate.type() == QGeoCoordinate::Coordinate3D)
                obj.setProperty("altitude", QJSValue(coordinate.altitude()));
            array.setProperty(i, obj);
        }
    }

    return array;
}

QGeoCoordinate QTangramPolyline::parseCoordinate(const QJSValue &value, bool *ok)
{
    QGeoCoordinate c;

    if (value.isObject()) {
        if (value.hasProperty(QStringLiteral("latitude")))
            c.setLatitude(value.property(QStringLiteral("latitude")).toNumber());
        if (value.hasProperty(QStringLiteral("longitude")))
            c.setLongitude(value.property(QStringLiteral("longitude")).toNumber());
        if (value.hasProperty(QStringLiteral("altitude")))
            c.setAltitude(value.property(QStringLiteral("altitude")).toNumber());

        if (ok)
            *ok = true;
    }

    return c;
}

void QTangramPolyline::setPathFromGeoList(const QList<QGeoCoordinate> &path)
{
    qDebug() << Q_FUNC_INFO;
    if (m_path == path)
        return;

    m_path = path;
    emit pathChanged();
    if (m_markerId == -1)
        return;

    std::vector<Tangram::LngLat> coord;
    for (int i = 0; i < m_path.size(); ++i)
        coord.push_back(Tangram::LngLat(m_path.at(i).longitude(), m_path.at(i).latitude()));
    m_tangramMap->markerSetPolyline(m_markerId, coord.data(), coord.size());
}

int QTangramPolyline::pathLength() const
{
    return m_path.size();
}

QTangramPolylineProperties *QTangramPolyline::line()
{
    return qobject_cast<QTangramPolylineProperties *>(m_properties);
}

void QTangramPolyline::initGeometry()
{
    qDebug() << Q_FUNC_INFO << ", m_markerId:" << m_markerId;
    if (m_path.isEmpty())
        return;

    std::vector<Tangram::LngLat> coord;
    for (int i = 0; i < m_path.size(); ++i)
        coord.push_back(Tangram::LngLat(m_path.at(i).longitude(), m_path.at(i).latitude()));
    m_tangramMap->markerSetPolyline(m_markerId, coord.data(), coord.size());
}
