#ifndef TANGRAMQUICKRENDERER_H
#define TANGRAMQUICKRENDERER_H

#include <QQuickFramebufferObject>
#include <QElapsedTimer>
#include <QGeoCoordinate>
#include <memory>

namespace Tangram {
class Map;
class QtPlatform;
}

class QQuickItem;
class TangramQuickRenderer;
class QTangramMarkerManager;
class QDeclarativeTangramMap;

class TangramQuickRenderer : public QObject, public QQuickFramebufferObject::Renderer
{
    Q_OBJECT
public:
    TangramQuickRenderer(QQuickItem *mapItem = 0);
    virtual ~TangramQuickRenderer();

    void render();
    void synchronize(QQuickFramebufferObject *item);
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size);

    void setContinuousRendering(bool contiuous);
    bool continuousRendering();

    QGeoCoordinate itemPositionToCoordinate(const QPointF &pos) const;
    QPointF coordinateToItemPosition(const QGeoCoordinate &coordinate) const;

Q_SIGNALS:
    void sceneChanged();

public slots:
    void updateScene(const QString path, const QString value);

protected:

private:
    void initMap();
    void syncTo(QDeclarativeTangramMap *map);
    int popSyncState();

    QUrl m_sceneUrl;
    QElapsedTimer m_elapsedTimer;
    bool m_initialized;
    bool m_glInitialized;
    bool m_useScenePosition;

    std::shared_ptr<Tangram::QtPlatform> m_platform;
    Tangram::Map* m_map;
    QTangramMarkerManager* m_markerManager;
    QQuickFramebufferObject* m_item;

    enum SyncState {
        NothingNeedsSync = 0x0000,
        ZoomNeedsSync = 0x0001,
        CenterNeedsSync = 0x0002,
        TiltNeedsSync = 0x0004,
        RotationNeedsSync = 0x0008
    };

    int m_syncState = NothingNeedsSync;

    friend class QtPlatform;
};

#endif // TANGRAMQUICKRENDERER_H

