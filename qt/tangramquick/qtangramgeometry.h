#ifndef QTANGRAMGEOMETRY_H
#define QTANGRAMGEOMETRY_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QJSValue>

namespace Tangram {
class Map;
}

class QTangramMap;

class QTangramGeometryProperties : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString style READ style WRITE setStyle NOTIFY styleChanged)
    Q_PROPERTY(QVariantMap extraStyling READ extraStyling WRITE setExtraStyling NOTIFY extraStylingChanged)
public:
    explicit QTangramGeometryProperties(QObject *parent =  0);

    void setStyle(QString style);
    void setExtraStyling(QVariantMap styling);
    QVariantMap extraStyling();
    QString style();

    Q_INVOKABLE void setStyling(QString key, QVariant value);
    Q_INVOKABLE QVariantMap stylings();

Q_SIGNALS:
    void stylingChanged();
    void extraStylingChanged();
    void styleChanged();

protected:
    virtual void updateProperty(QString key);
    QVariantMap m_stylings;
    QVariantMap m_extraStyling;
};


class QTangramGeometry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(int drawOrder READ drawOrder WRITE setDrawOrder NOTIFY drawOrderChanged)
    Q_PROPERTY(bool clickable READ clickable WRITE setClickable NOTIFY clickableChanged)
public:
    explicit QTangramGeometry(QObject *parent = 0, QTangramGeometryProperties *properties = 0);

    virtual void setMap(QTangramMap *map);
    QTangramMap *map();

    void setVisible(bool visible);
    bool visible();

    void setDrawOrder(int drawOrder);
    int drawOrder();

    void setClickable(bool clickable);
    bool clickable();

    int markerId();

signals:
    void visibleChanged(bool visible);
    void drawOrderChanged(int drawOrder);
    void clickableChanged();
    void clicked();

public slots:

protected:
    virtual void initGeometry();
    virtual bool isInteractive();
    int m_markerId;
    bool m_clickable;
    Tangram::Map *m_tangramMap;
    QTangramGeometryProperties *m_properties;
    QTangramMap *m_map;

protected slots:
    virtual void setStyling();

private:
    bool m_visible;
    int m_drawOrder;
};

#endif // QTANGRAMGEOMETRY_H
