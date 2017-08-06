#ifndef QTANGRAMPOINT_H
#define QTANGRAMPOINT_H

#include <QObject>
#include <QGeoCoordinate>
#include "qtangramgeometry.h"

class QVariant;

class QTangramPoint : public QTangramGeometry
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(bool draggable READ draggable WRITE setDraggable NOTIFY draggableChanged)
    Q_PROPERTY(QString imageSource READ imageSource WRITE setImageSource NOTIFY imageSourceChanged)
public:
    explicit QTangramPoint(QObject *parent = 0);
    ~QTangramPoint();

    void setCoordinate(const QGeoCoordinate &coordinate);
    QGeoCoordinate coordinate() const;

    void setDraggable(bool draggable);
    bool draggable() const;

    void setImageSource(const QString &imageSource);
    QString imageSource() const;

    void setMap(QDeclarativeTangramMap *map);

Q_SIGNALS:
    void visualChanged();
    void coordinateChanged();
    void draggableChanged();
    void imageSourceChanged();
    void dragged();

public slots:

protected:
    virtual bool isInteractive();

private:
    QGeoCoordinate m_coordinate;
    bool m_draggable;
    QString m_imageSource;

    friend class QTangramMarkerManager;
};

#endif // QTANGRAMPOINT_H
