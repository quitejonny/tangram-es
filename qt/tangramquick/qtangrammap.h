#ifndef QTANGRAMMAP_H
#define QTANGRAMMAP_H

#include <QtCore/QObject>
#include <QMutex>
#include <QUrl>

class QTangramMapController;
class TangramQuickRenderer;
class QDeclarativeTangramMap;
class QTangramGestureArea;
class QGeoCoordinate;

namespace Tangram {
class Map;
}

class QTangramMap : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl scene READ scene WRITE setScene NOTIFY sceneChanged)

public:
    QTangramMap(QObject *parent = 0);
    virtual ~QTangramMap();

    QTangramMapController *mapController();

    void resize(int width, int height);
    int width() const;
    int height() const;

    void setScene(QUrl scene, bool useScenePos = false);
    QUrl scene();

    //void setActiveMapType(const QGeoMapType mapType);
    //const QGeoMapType activeMapType() const;

    QGeoCoordinate itemPositionToCoordinate(const QPointF &pos, bool clipToViewport = true) const;
    QPointF coordinateToItemPosition(const QGeoCoordinate &coordinate, bool clipToViewport = true) const;
    // virtual void prefetchData();
    // void clearData();


protected:
    // void setCameraData(const QGeoCameraData &cameraData);
    // virtual QSGNode *updateSceneGraph(QSGNode *node, QQuickWindow *window) = 0;
    Tangram::Map* tangramObject();

public Q_SLOTS:
    void update();

Q_SIGNALS:
    // void cameraDataChanged(const QGeoCameraData &cameraData);
    void updateRequired();
    void sceneChanged();
    // void activeMapTypeChanged();
    // void copyrightsChanged(const QImage &copyrightsImage);
    // void copyrightsChanged(const QString &copyrightsHtml);

private:
    Tangram::Map* m_tangramMap;
    QTangramMapController* m_controller;
    QMutex m_mutex;
    QUrl m_scene;

    Q_DISABLE_COPY(QTangramMap)
    friend class TangramQuickRenderer;
    friend class QDeclarativeTangramMap;
    friend class QTangramGestureArea;
    friend class QTangramGeometry;
    friend class QTangramMapController; //setCameraData
    //friend class QDeclarativeGeoMap; //updateSceneGraph
};

#endif // QTANGRAMMAP_H
