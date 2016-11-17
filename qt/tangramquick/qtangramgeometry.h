#ifndef QTANGRAMGEOMETRY_H
#define QTANGRAMGEOMETRY_H

#include <QObject>
#include <QMap>
#include <QVariant>

namespace Tangram {
class Map;
}

class QTangramMap;

class QTangramGeometryProperties : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString style READ style WRITE setStyle NOTIFY styleChanged)
public:
    explicit QTangramGeometryProperties(QObject *parent =  0);

    void setStyle(QString style);
    QString style();

    Q_INVOKABLE void setStyling(QString key, QVariant value);
    Q_INVOKABLE QVariantMap stylings();

Q_SIGNALS:
    void stylingChanged();
    void styleChanged();

protected:
    virtual void updateProperty(QString key);
    QVariantMap m_stylings;
};


class QTangramGeometry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(int drawOrder READ drawOrder WRITE setDrawOrder NOTIFY drawOrderChanged)
public:
    explicit QTangramGeometry(QObject *parent = 0);

    void setMap(QTangramMap *map);
    QTangramMap *map();

    void setVisible(bool visible);
    bool visible();

    void setDrawOrder(int drawOrder);
    int drawOrder();

signals:
    void visibleChanged(bool visible);
    void drawOrderChanged(int drawOrder);

public slots:

protected:
    virtual void initGeometry();
    int m_markerId;
    Tangram::Map *m_tangramMap;
    QTangramGeometryProperties* m_properties;

protected slots:
    virtual void setStyling();

private:
    bool m_visible;
    int m_drawOrder;
    QTangramMap *m_map;
};

#endif // QTANGRAMGEOMETRY_H
