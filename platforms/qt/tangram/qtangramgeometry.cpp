#include "qtangramgeometry.h"
#include "qtangrampoint.h"
#include "qtangrammap.h"
#include "tangram.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJSValue>
#include <QDebug>

QTangramGeometry::QTangramGeometry(QObject *parent)
    : QObject(parent),
      m_markerId(-1),
      m_clickable(false),
      m_map(0),
      m_visible(true),
      m_drawOrder(-1),
      m_styling()
{
    connect(this, SIGNAL(stylingChanged()), this, SLOT(setTangramStyling()));
}

QTangramGeometry::~QTangramGeometry()
{
    if (m_tangramMap)
        m_tangramMap->markerRemove(m_markerId);
}

QString QTangramGeometry::colorToHex(const QColor color) const
{
    return color.name(QColor::HexArgb);
}

void QTangramGeometry::setStyling(QVariantMap styling)
{
    m_styling = styling;
    emit stylingChanged();
}

QVariantMap QTangramGeometry::styling()
{
    return m_styling;
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
    if (map == m_map)
        return;

    if (!map && m_map) {
        m_map->setClickable(this, false);
    }

    m_map = map;
    if (m_map) {
        m_map->setClickable(this, m_clickable);
        m_tangramMap = m_map->tangramObject();
        m_markerId = m_tangramMap->markerAdd();
        m_tangramMap->markerSetVisible(m_markerId, m_visible);
        m_tangramMap->markerSetDrawOrder(m_markerId, m_visible);
        setTangramStyling();
        initGeometry();
    } else {
        m_tangramMap->markerRemove(m_markerId);
        m_tangramMap = 0;
        m_markerId = -1;
    }
}

bool QTangramGeometry::isInteractive()
{
    return m_clickable;
}

QTangramMap *QTangramGeometry::map()
{
    return m_map;
}

void QTangramGeometry::setTangramStyling()
{
    if (m_markerId == -1)
        return;

    QVariantMap allStylings = m_defaultStyling;
    for (const auto &key : m_styling.keys())
        allStylings[key] = m_styling[key];

    auto obj = QJsonObject::fromVariantMap(allStylings);
    QJsonDocument doc(obj);
    QByteArray stylings = doc.toJson(QJsonDocument::Compact);
    m_tangramMap->markerSetStylingFromString(m_markerId, stylings.toStdString().c_str());
}

void QTangramGeometry::initGeometry()
{
}

void QTangramGeometry::setClickable(bool clickable)
{
    if (clickable == m_clickable)
        return;

    bool interactive = isInteractive();
    m_clickable = clickable;
    if (m_markerId != -1)
        m_map->setClickable(this, m_clickable);
    emit clickableChanged();

    if (interactive != isInteractive()) {
        m_defaultStyling["interactive"] = isInteractive();
        setTangramStyling();
    }
}

bool QTangramGeometry::clickable()
{
    return m_clickable;
}

int QTangramGeometry::markerId()
{
    return m_markerId;
}
