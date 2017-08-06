#ifndef QTANGRAMPOLYLINE_H
#define QTANGRAMPOLYLINE_H

#include <QObject>
#include <QColor>
#include "qtangramgeometry.h"

class QJSValue;
class QGeoCoordinate;

class QTangramPolyline : public QTangramGeometry
{
    Q_OBJECT

    Q_PROPERTY(QJSValue path READ path WRITE setPath NOTIFY pathChanged)

public:
    explicit QTangramPolyline(QObject *parent = 0);
    ~QTangramPolyline();

    Q_INVOKABLE int pathLength() const;

    QJSValue path() const;
    virtual void setPath(const QJSValue &value);

    void setMap(QDeclarativeTangramMap *map);

Q_SIGNALS:
    void pathChanged();

private:
    QGeoCoordinate parseCoordinate(const QJSValue &value, bool *ok);
    void setPathFromGeoList(const QList<QGeoCoordinate> &path);
    QList<QGeoCoordinate> m_path;

    friend class QTangramMarkerManager;
};

#endif // QTANGRAMPOLYLINE_H
