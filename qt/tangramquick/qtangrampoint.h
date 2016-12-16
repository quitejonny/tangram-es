#ifndef QTANGRAMPOINT_H
#define QTANGRAMPOINT_H

#include <QObject>
#include <QColor>
#include <QGeoCoordinate>
#include "qtangramgeometry.h"

class QVariant;

class QTangramPointProperties : public QTangramGeometryProperties
{
    Q_OBJECT

    Q_PROPERTY(QVariant size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation NOTIFY rotationChanged)

public:
    explicit QTangramPointProperties(QObject *parent = 0);

    void setColor(const QColor &color);
    QColor color() const;

    void setSize(const QVariant &size);
    QVariant size() const;

    void setRotation(const qreal);
    qreal rotation() const;

Q_SIGNALS:
    void sizeChanged();
    void colorChanged();
    void rotationChanged();

protected:
    virtual void updateProperty(QString key);

private:
    QVariant m_size;
    QColor m_color;
    qreal m_rotation;
};

class QTangramPoint : public QTangramGeometry
{
    Q_OBJECT
    Q_PROPERTY(QTangramPointProperties *visual READ visual NOTIFY visualChanged)
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(bool draggable READ draggable WRITE setDraggable NOTIFY draggableChanged)
public:
    explicit QTangramPoint(QObject *parent = 0);
    ~QTangramPoint();

    void setCoordinate(const QGeoCoordinate &coordinate);
    QGeoCoordinate coordinate() const;

    void setDraggable(bool draggable);
    bool draggable();

    void setMap(QTangramMap *map);

    QTangramPointProperties *visual();

Q_SIGNALS:
    void visualChanged();
    void coordinateChanged();
    void draggableChanged();

public slots:

protected:
    virtual void initGeometry();
    virtual bool isInteractive();

private:
    QGeoCoordinate m_coordinate;
    bool m_draggable;

};

#endif // QTANGRAMPOINT_H
