#include "qtangramgeometry.h"
#include "qtangrampoint.h"
#include "tangramquick.h"
#include "map.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJSValue>
#include <QDebug>

QTangramGeometry::QTangramGeometry(QObject *parent)
    : QObject(parent),
      m_markerId(-1),
      m_clickable(false),
      m_map(),
      m_visible(true),
      m_drawOrder(-1),
      m_styling()
{
    connect(this, SIGNAL(stylingChanged()), this, SLOT(setTangramStyling()));
}

QTangramGeometry::~QTangramGeometry()
{
    if (m_map)
        m_map->removeMapItem(this);
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
    addSyncState(VisibleNeedsSync);
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
    addSyncState(DrawOrderNeedsSync);
    emit drawOrderChanged(drawOrder);
}

int QTangramGeometry::drawOrder()
{
    return m_drawOrder;
}

void QTangramGeometry::setMap(QDeclarativeTangramMap *map)
{
    if (map == m_map.data())
        return;

    addSyncState(MarkerIdNeedsSync);

    if (map) {
        if (m_drawOrder > 0)
            addSyncState(DrawOrderNeedsSync);
        addSyncState(VisibleNeedsSync | ClickableNeedsSync | StylingNeedsSync);
        map->itemchangedData(this);
    } else if (m_map) {
        m_map->itemchangedData(this);
    }

    m_map = QPointer<QDeclarativeTangramMap>(map);
}

bool QTangramGeometry::isInteractive()
{
    return m_clickable;
}

void QTangramGeometry::addSyncState(int state)
{
    m_syncState |= state;

    if (m_map)
        m_map->itemchangedData(this);
}

QDeclarativeTangramMap *QTangramGeometry::map()
{
    return m_map.data();
}

void QTangramGeometry::setTangramStyling()
{
    QVariantMap allStylings = m_defaultStyling;
    for (const auto &key : m_styling.keys())
        allStylings[key] = m_styling[key];

    auto obj = QJsonObject::fromVariantMap(allStylings);
    QJsonDocument doc(obj);
    m_allStylings = doc.toJson(QJsonDocument::Compact);
    addSyncState(StylingNeedsSync);
}

void QTangramGeometry::setClickable(bool clickable)
{
    if (clickable == m_clickable)
        return;

    bool interactive = isInteractive();
    m_clickable = clickable;
    addSyncState(ClickableNeedsSync);
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
