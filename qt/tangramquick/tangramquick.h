#ifndef TANGRAMQUICK_H
#define TANGRAMQUICK_H

#include <QQuickFramebufferObject>
#include <QQmlParserStatus>
#include <QElapsedTimer>
#include <QQuickItem>
#include <QMutex>
#include <data/clientGeoJsonSource.h>
#include <QGeoCoordinate>
#include "tangram.h"

class TangramQuickRenderer;
class QTangramMap;
class QTangramGestureArea;
class QDeclarativeTangramMap : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QUrl sceneConfiguration READ sceneConfiguration WRITE setSceneConfiguration NOTIFY sceneConfigurationChanged)
    Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(int moveAnimationDuration READ moveAnimationDuration WRITE setMoveAnimationDuration NOTIFY moveAnimationDurationChanged)
    Q_PROPERTY(qreal heading READ heading WRITE setHeading NOTIFY headingChanged)
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(float tilt READ tilt WRITE setTilt NOTIFY tiltChanged)
    Q_PROPERTY(bool continuousRendering READ continuousRendering WRITE setContinuousRendering NOTIFY continuousRenderingChanged)
public:
    explicit QDeclarativeTangramMap(QQuickItem *parent = 0);
    ~QDeclarativeTangramMap();

    qreal zoomLevel() const;

    void setCenter(const QGeoCoordinate &center);
    QGeoCoordinate center() const;

    Renderer *createRenderer() const;

    QUrl sceneConfiguration() const;
    qreal heading() const;
    float tilt() const;
    int moveAnimationDuration() const;
    bool continuousRendering() const;

public slots:
    void setZoomLevel(qreal zoomLevel);
    void setSceneConfiguration(const QUrl scene);
    void setHeading(const qreal heading);
    void setTilt(const float tilt);
    void setMoveAnimationDuration(const int duration);
    void setContinuousRendering(const bool continuousRendering);

    // TODO: The documents uses a comma separated list on path. Maybe change
    // the to QMap<QString, QString>. But how will that work in QML?
    Q_INVOKABLE void queueSceneUpdate(const QString path, const QString value);
    Q_INVOKABLE void applySceneUpdates();

signals:
    void sceneConfigurationChanged();
    void centerChanged(const QGeoCoordinate &center);
    void moveAnimationDurationChanged();
    void headingChanged();
    void zoomLevelChanged(qreal);
    void tiltChanged(qreal);
    void continuousRenderingChanged(bool);

protected:
    bool event(QEvent *ev);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE ;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE ;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE ;
    void mouseUngrabEvent() Q_DECL_OVERRIDE ;
    void touchUngrabEvent() Q_DECL_OVERRIDE;
    void touchEvent(QTouchEvent *event) Q_DECL_OVERRIDE ;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE ;


private:
    void setMap(QTangramMap* map);

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
    int m_moveAnimationDuration;
    qreal m_heading;
    float m_zoomLevel;
    float m_tilt;
    bool m_continuousRendering;

    int m_syncState = NothingNeedsSync;
    QTangramMap* m_map;
    QTangramGestureArea* m_gestureArea;
    bool m_tangramMapInitialized;

    friend class TangramQuickRenderer;
    friend class QTangramGestureArea;
    Q_DISABLE_COPY(QDeclarativeTangramMap)
};

class TangramQuickRenderer : public QQuickFramebufferObject::Renderer
{
public:
    TangramQuickRenderer();
    virtual ~TangramQuickRenderer();

    void render();
    void synchronize(QQuickFramebufferObject *item);
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size);

private:
    void initializeGL();

    std::shared_ptr<Tangram::ClientGeoJsonSource> m_dataSource;
    QUrl m_sceneUrl;
    QElapsedTimer m_elapsedTimer;
    bool m_glInitialized;
    QMutex m_renderMutex;

    QGeoCoordinate m_center;
    qreal m_heading;
    qreal m_zoomLevel;
    QSize m_size;
    float m_tilt;
    int m_moveAnimationDuration;
    QTangramMap* m_map;
};

QML_DECLARE_TYPE(QDeclarativeTangramMap)

#endif // TANGRAMQUICK_H

