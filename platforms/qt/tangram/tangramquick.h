#ifndef TANGRAMQUICK_H
#define TANGRAMQUICK_H

#include <QQuickFramebufferObject>
#include <QQmlParserStatus>
#include <QSet>
#include <QGeoCoordinate>

class QQuickItem;
class QTangramGestureArea;
class QTangramGeometry;
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

    Q_INVOKABLE void updateScene(const QString path, const QString value);

    void itemchangedData(QTangramGeometry *item);
    void update();

Q_SIGNALS:
    void zoomLevelChanged(qreal zoomLevel);
    void centerChanged(const QGeoCoordinate &center);
    void sceneConfigurationChanged();
    void headingChanged();
    void tiltChanged(qreal tilt);
    void rotationChanged(qreal rotation);
    void pixelScaleChanged(qreal pixelScale);

    void sceneChanged();
    void updateSceneSignal(const QString path, const QString value);
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

    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void updateDeclarativeScene();
    void populateMap();

private:
    void addSyncState(int syncState);

    qreal m_zoomLevel;
    QGeoCoordinate m_center;
    QUrl m_sceneUrl;
    qreal m_heading;
    qreal m_tilt;
    qreal m_rotation;
    qreal m_pixelScale;
    bool m_isMapReady;
    bool m_isUpdateRequested;

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

QML_DECLARE_TYPE(QDeclarativeTangramMap)

#endif // TANGRAMQUICK_H

