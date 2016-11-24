#ifndef QTANGRAMPOLYLINE_H
#define QTANGRAMPOLYLINE_H

#include <QObject>
#include <QColor>
#include "qtangramgeometry.h"

class QJSValue;
class QGeoCoordinate;

class QTangramPolylineProperties : public QTangramGeometryProperties
{
    Q_OBJECT

    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit QTangramPolylineProperties(QObject *parent = 0);

    QColor color() const;
    void setColor(const QColor &color);

    qreal width() const;
    void setWidth(qreal width);

Q_SIGNALS:
    void widthChanged(qreal width);
    void colorChanged(const QColor &color);

protected:
    virtual void updateProperty(QString key);

private:
    qreal m_width;
    QColor m_color;
};


class QTangramPolyline : public QTangramGeometry
{
    Q_OBJECT

    Q_PROPERTY(QJSValue path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QTangramPolylineProperties *line READ line CONSTANT)

public:
    explicit QTangramPolyline(QObject *parent = 0);
    ~QTangramPolyline();

    Q_INVOKABLE int pathLength() const;

    QJSValue path() const;
    virtual void setPath(const QJSValue &value);

    QTangramPolylineProperties* line();

Q_SIGNALS:
    void pathChanged();

protected:
    virtual void initGeometry();

private:
    QGeoCoordinate parseCoordinate(const QJSValue &value, bool *ok);
    void setPathFromGeoList(const QList<QGeoCoordinate> &path);
    QList<QGeoCoordinate> m_path;
};

#endif // QTANGRAMPOLYLINE_H
