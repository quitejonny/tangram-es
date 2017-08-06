#include "qtangrampolyline.h"
#include "map.h"
#include "util/types.h"
#include <QJSValue>
#include <QGeoCoordinate>
#include <QQmlEngine>
#include <QQmlContext>
#include <QColor>
#include <QDebug>
#include "tangramquick.h"

QTangramPolyline::QTangramPolyline(QObject *parent)
    : QTangramGeometry(parent)
{
    m_defaultStyling["color"] = "ffffffff";
    m_defaultStyling["width"] = "2px";
    m_defaultStyling["style"] = "lines";
    m_defaultStyling["order"] = 2;
}

QTangramPolyline::~QTangramPolyline()
{
}

void QTangramPolyline::setPath(const QJSValue &value)
{
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
    if (m_path == path)
        return;

    m_path = path;
    emit pathChanged();

    addSyncState(PolylineNeedsSync);
}

int QTangramPolyline::pathLength() const
{
    return m_path.size();
}

void QTangramPolyline::setMap(QDeclarativeTangramMap *map)
{
    if (map && !m_path.empty())
        addSyncState(PolylineNeedsSync);

    QTangramGeometry::setMap(map);
}
