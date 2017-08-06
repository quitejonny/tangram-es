#ifndef TANGRAMQUICK_H
#define TANGRAMQUICK_H

#include <QQuickFramebufferObject>
#include <QQmlParserStatus>
#include <QElapsedTimer>
#include <QQuickItem>
#include <QSet>
#include <QMutex>
#include <data/clientGeoJsonSource.h>
#include <QGeoCoordinate>
#include <memory>
#include "map.h"

#include <QScopedPointer>
#include <QOpenGLShaderProgram>

namespace Tangram {
class Map;
class QtPlatform;
}

class TangramQuickRenderer;
class QTangramMarkerManager;
class QTangramGestureArea;
class QTangramGeometry;
class QTangramPoint;
class ContentDownloader;

class QDeclarativeTangramMap : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QTangramGestureArea *gesture READ gesture CONSTANT)
    Q_PROPERTY(QUrl sceneConfiguration READ sceneConfiguration WRITE setSceneConfiguration NOTIFY sceneConfigurationChanged)
    Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(qreal heading READ heading WRITE setHeading NOTIFY headingChanged)
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(qreal tilt READ tilt WRITE setTilt NOTIFY tiltChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(qreal pixelScale READ pixelScale WRITE setPixelScale NOTIFY pixelScaleChanged)
public:
    explicit QDeclarativeTangramMap(QQuickItem *parent = 0);
    ~QDeclarativeTangramMap();

    QTangramGestureArea *gesture();

    void setZoomLevel(const qreal zoomLevel);
    qreal zoomLevel() const;

    void setCenter(const QGeoCoordinate &center);
    QGeoCoordinate center() const;

    void setSceneConfiguration(const QUrl &scene);
    QUrl sceneConfiguration() const;

    void setHeading(const qreal heading);
    qreal heading() const;

    void setTilt(const qreal tilt);
    qreal tilt() const;

    void setRotation(const qreal rotation);
    qreal rotation() const;

    void setPixelScale(const qreal pixelScale);
    qreal pixelScale() const;

    Q_INVOKABLE void removeMapItem(QTangramGeometry *item);
    Q_INVOKABLE void addMapItem(QTangramGeometry *item);

    Renderer *createRenderer() const;

    Q_INVOKABLE void queueSceneUpdate(const QString path, const QString value);
    Q_INVOKABLE void applySceneUpdates();

    void itemchangedData(QTangramGeometry *item);

Q_SIGNALS:
    void zoomLevelChanged(qreal zoomLevel);
    void centerChanged(const QGeoCoordinate &center);
    void sceneConfigurationChanged();
    void headingChanged();
    void tiltChanged(qreal tilt);
    void rotationChanged(qreal rotation);
    void pixelScaleChanged(qreal pixelScale);

    void sceneChanged();
    void queueSceneUpdateSignal(const QString path, const QString value);
    void applySceneUpdatesSignal();
    void mapReady();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE ;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE ;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE ;
    void mouseUngrabEvent() Q_DECL_OVERRIDE ;
    void touchUngrabEvent() Q_DECL_OVERRIDE;
    void touchEvent(QTouchEvent *event) Q_DECL_OVERRIDE ;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE ;
    bool event(QEvent *ev) Q_DECL_OVERRIDE;

    void componentComplete() Q_DECL_OVERRIDE;

    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void updateScene();
    void populateMap();

private:
    qreal m_zoomLevel;
    QGeoCoordinate m_center;
    QUrl m_sceneUrl;
    qreal m_heading;
    qreal m_tilt;
    qreal m_rotation;
    qreal m_pixelScale;

    enum SyncState {
        NothingNeedsSync = 0,
        ZoomNeedsSync = 1 << 0,
        CenterNeedsSync = 1 << 1,
        SceneConfigurationNeedsSync = 1 << 2,
        HeadingNeedsSync = 1 << 3,
        TiltNeedsSync = 1 << 4,
        RotationNeedsSync = 1 << 5,
        PixelScaleNeedsSync = 1 << 6,
    };

    int m_syncState = NothingNeedsSync;

    QSet<QTangramGeometry *> m_changedItems;
    QVector<int> m_removedMarkerIds;

    QTangramGestureArea* m_gestureArea;
    QVector<QTangramGeometry *> m_mapItems;

    ContentDownloader *m_downloader;

    friend class TangramQuickRenderer;
    friend class QTangramGestureArea;
    Q_DISABLE_COPY(QDeclarativeTangramMap)
};

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
    void queueSceneUpdate(const QString path, const QString value);
    void applySceneUpdates();

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

QML_DECLARE_TYPE(QDeclarativeTangramMap)

#endif // TANGRAMQUICK_H

