#ifndef QTANGRAMGEOMETRY_H
#define QTANGRAMGEOMETRY_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QGeoCoordinate>
#include <QColor>
#include <QPointer>
#include "tangramquick.h"

namespace Tangram {
class Map;
}

class QTangramGeometry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(int drawOrder READ drawOrder WRITE setDrawOrder NOTIFY drawOrderChanged)
    Q_PROPERTY(bool clickable READ clickable WRITE setClickable NOTIFY clickableChanged)
    Q_PROPERTY(QVariantMap styling READ styling WRITE setStyling NOTIFY stylingChanged)
public:
    explicit QTangramGeometry(QObject *parent = 0);
    ~QTangramGeometry();

    virtual void setMap(QDeclarativeTangramMap *map);
    QDeclarativeTangramMap *map();

    void setVisible(bool visible);
    bool visible();

    void setDrawOrder(int drawOrder);
    int drawOrder();

    void setClickable(bool clickable);
    bool clickable();

    void setStyling(QVariantMap styling);
    QVariantMap styling();

    int markerId();

    Q_INVOKABLE QString colorToHex(const QColor color) const;

signals:
    void visibleChanged(bool visible);
    void drawOrderChanged(int drawOrder);
    void clickableChanged();
    void clicked(QGeoCoordinate coordinate);
    void stylingChanged();

public slots:

protected:
    virtual bool isInteractive();
    void addSyncState(int state);
    int m_markerId;
    bool m_clickable;
    QPointer<QDeclarativeTangramMap> m_map;
    QVariantMap m_defaultStyling;

    enum SyncState {
        NothingNeedsSync = 0,
        StylingNeedsSync = 1 << 0,
        BitmapNeedsSync = 1 << 1,
        PointNeedsSync = 1 << 2,
        PolylineNeedsSync = 1 << 3,
        PolygonNeedsSync = 1 << 4,
        VisibleNeedsSync = 1 << 5,
        DrawOrderNeedsSync = 1 << 6,
        MarkerIdNeedsSync = 1 << 7,
        ClickableNeedsSync = 1 << 8,
        DraggableNeedsSync = 1 << 9
    };

    int m_syncState = NothingNeedsSync;

protected slots:
    virtual void setTangramStyling();

private:
    bool m_visible;
    int m_drawOrder;
    QVariantMap m_styling;
    QByteArray m_allStylings;

    friend class QTangramMarkerManager;
    Q_DISABLE_COPY(QTangramGeometry)
};

#endif // QTANGRAMGEOMETRY_H
