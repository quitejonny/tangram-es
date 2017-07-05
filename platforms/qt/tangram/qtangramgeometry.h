#ifndef QTANGRAMGEOMETRY_H
#define QTANGRAMGEOMETRY_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QGeoCoordinate>
#include <QColor>

namespace Tangram {
class Map;
}

class QTangramMap;

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

    virtual void setMap(QTangramMap *map);
    QTangramMap *map();

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
    virtual void initGeometry();
    virtual bool isInteractive();
    int m_markerId;
    bool m_clickable;
    Tangram::Map *m_tangramMap;
    QTangramMap *m_map;
    QVariantMap m_defaultStyling;

protected slots:
    virtual void setTangramStyling();

private:
    bool m_visible;
    int m_drawOrder;
    QVariantMap m_styling;
};

#endif // QTANGRAMGEOMETRY_H
