#include "qtangramgeometry.h"
#include "qtangrammap.h"
#include "tangram.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

QTangramGeometryProperties::QTangramGeometryProperties(QObject *parent)
    : QObject(parent)
{
}

void QTangramGeometryProperties::setStyling(QString key, QVariant value)
{
    if (m_stylings.contains(key) && m_stylings.value(key) == value)
        return;

    m_stylings[key] = value;
    updateProperty(key);
    emit stylingChanged();
}

QVariantMap QTangramGeometryProperties::stylings()
{
    return m_stylings;
}

void QTangramGeometryProperties::setStyle(QString style)
{
    setStyling(QStringLiteral("style"), QVariant::fromValue(style));
    emit styleChanged();
}

QString QTangramGeometryProperties::style()
{
    if (m_stylings.contains(QStringLiteral("style")))
        return m_stylings[QStringLiteral("style")].value<QString>();
    else
        return QString();
}

void QTangramGeometryProperties::updateProperty(QString key)
{
    Q_UNUSED(key)
}

QTangramGeometry::QTangramGeometry(QObject *parent)
    : QObject(parent),
      m_markerId(-1),
      m_properties(0),
      m_visible(true),
      m_drawOrder(-1),
      m_map(0)
{
}

void QTangramGeometry::setVisible(bool visible)
{
    if (m_visible == visible)
        return;

    m_visible = visible;
    if (m_map) {
        m_tangramMap->markerSetVisible(m_markerId, m_visible);
    }
    emit visibleChanged(visible);
}

bool QTangramGeometry::visible()
{
    return m_visible;
}

void QTangramGeometry::setDrawOrder(int drawOrder)
{
    if (m_drawOrder == drawOrder || drawOrder < 0)
        return;

    m_drawOrder = drawOrder;
    if (m_map) {
        m_tangramMap->markerSetDrawOrder(m_markerId, m_drawOrder);
    }
    emit drawOrderChanged(drawOrder);
}

int QTangramGeometry::drawOrder()
{
    return m_drawOrder;
}

void QTangramGeometry::setMap(QTangramMap *map)
{
    m_map = map;
    if (m_map) {
        m_tangramMap = m_map->tangramObject();
        m_markerId = m_tangramMap->markerAdd();
        m_tangramMap->markerSetVisible(m_markerId, m_visible);
        m_tangramMap->markerSetDrawOrder(m_markerId, m_visible);
        setStyling();
        initGeometry();
    } else {
        m_tangramMap->markerRemove(m_markerId);
        m_tangramMap = 0;
        m_markerId = -1;
    }
}

QTangramMap *QTangramGeometry::map()
{
    return m_map;
}

void QTangramGeometry::setStyling()
{
    if (m_markerId != -1) {
        auto obj = QJsonObject::fromVariantMap(m_properties->stylings());
        QJsonDocument doc(obj);
        QByteArray stylings = doc.toJson(QJsonDocument::Compact);
        qDebug() << Q_FUNC_INFO << stylings;
        m_tangramMap->markerSetStyling(m_markerId, stylings.toStdString().c_str());
    }
}

void QTangramGeometry::initGeometry()
{
}
