#ifndef TANGRAMQUICK_H
#define TANGRAMQUICK_H

#include <QQuickFramebufferObject>
#include <QQmlParserStatus>
#include <QElapsedTimer>
#include <QQuickItem>
#include <QMutex>
#include <data/clientGeoJsonSource.h>
#include <QGeoCoordinate>

#include <QScopedPointer>
#include <QOpenGLShaderProgram>

class TangramQuickRenderer;
class QTangramMap;
class QTangramGestureArea;
class QTangramGeometry;

class QDeclarativeTangramMap : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QUrl sceneConfiguration READ sceneConfiguration WRITE setSceneConfiguration NOTIFY sceneConfigurationChanged)
    Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(qreal heading READ heading WRITE setHeading NOTIFY headingChanged)
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(qreal tilt READ tilt WRITE setTilt NOTIFY tiltChanged)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation NOTIFY rotationChanged)
    Q_PROPERTY(bool continuousRendering READ continuousRendering WRITE setContinuousRendering NOTIFY continuousRenderingChanged)
    Q_PROPERTY(qreal pixelScale READ pixelScale WRITE setPixelScale NOTIFY pixelScaleChanged)
public:
    explicit QDeclarativeTangramMap(QQuickItem *parent = 0);
    ~QDeclarativeTangramMap();

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

    void setContinuousRendering(const bool continuousRendering);
    bool continuousRendering() const;

    void setRotation(const qreal rotation);
    qreal rotation() const;

    void setPixelScale(const qreal pixelScale);
    qreal pixelScale() const;

    Q_INVOKABLE void removeMapItem(QTangramGeometry *item);
    Q_INVOKABLE void addMapItem(QTangramGeometry *item);

    Renderer *createRenderer() const;

    // TODO: The documents uses a comma separated list on path. Maybe change
    // the to QMap<QString, QString>. But how will that work in QML?
    Q_INVOKABLE void queueSceneUpdate(const QString path, const QString value);
    Q_INVOKABLE void applySceneUpdates();

Q_SIGNALS:
    void sceneConfigurationChanged();
    void centerChanged(const QGeoCoordinate &center);
    void headingChanged();
    void zoomLevelChanged(qreal);
    void tiltChanged(qreal);
    void rotationChanged(qreal rotation);
    void continuousRenderingChanged(bool);
    void pixelScaleChanged(qreal pixelScale);

protected:
    bool event(QEvent *ev);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE ;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE ;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE ;
    void mouseUngrabEvent() Q_DECL_OVERRIDE ;
    void touchUngrabEvent() Q_DECL_OVERRIDE;
    void touchEvent(QTouchEvent *event) Q_DECL_OVERRIDE ;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE ;

    void componentComplete() Q_DECL_OVERRIDE;

    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void updateScene();
    void populateMap();
    void mapZoomLevelChanged(qreal zoom);
    void mapTiltChanged(qreal tilt);
    void mapRotationChanged(qreal rotation);
    void mapPixelScaleChanged(qreal pixelScale);

private:
    enum SyncState {
        NothingNeedsSync = 0,
        ZoomNeedsSync    = 1 << 0,
        CenterNeedsSync  = 1 << 1,
        StyleNeedsSync   = 1 << 2,
        PanNeedsSync     = 1 << 3,
        BearingNeedsSync = 1 << 4,
        PitchNeedsSync   = 1 << 5,
    };

    QUrl m_sceneUrl;
    QGeoCoordinate m_center;
    qreal m_heading;
    qreal m_zoomLevel;
    qreal m_tilt;
    qreal m_rotation;
    qreal m_pixelScale;
    bool m_continuousRendering;

    int m_syncState = NothingNeedsSync;
    QTangramMap* m_map;
    QTangramGestureArea* m_gestureArea;

    QVector<QTangramGeometry *> m_mapItems;

    friend class TangramQuickRenderer;
    friend class QTangramGestureArea;
    Q_DISABLE_COPY(QDeclarativeTangramMap)
};

class TangramQuickRenderer : public QQuickFramebufferObject::Renderer
{
public:
    TangramQuickRenderer(QTangramMap *map);
    virtual ~TangramQuickRenderer();

    void render();
    void synchronize(QQuickFramebufferObject *item);
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size);

private:
    void initializeGL();

    QUrl m_sceneUrl;
    QElapsedTimer m_elapsedTimer;
    bool m_glInitialized;
    bool m_useScenePosition;
    QMutex m_renderMutex;

    QTangramMap* m_map;

    QScopedPointer<QOpenGLShaderProgram> m_program;
};

QML_DECLARE_TYPE(QDeclarativeTangramMap)

#endif // TANGRAMQUICK_H

