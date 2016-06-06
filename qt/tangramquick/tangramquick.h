#ifndef TANGRAM_H
#define TANGRAM_H

#include <QQuickFramebufferObject>
#include <QElapsedTimer>
#include <QQuickItem>
#include <QMutex>
#include <data/clientGeoJsonSource.h>

class TangramQuickRenderer;
class TangramQuick : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TangramQuick)
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QUrl sceneConfiguration READ sceneConfiguration WRITE setSceneConfiguration NOTIFY sceneConfigurationChanged)
    Q_PROPERTY(QPointF position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(int moveAnimationDuration READ moveAnimationDuration WRITE setMoveAnimationDuration NOTIFY moveAnimationDurationChanged)
    Q_PROPERTY(qreal heading READ heading WRITE setHeading NOTIFY headingChanged)
    Q_PROPERTY(float zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(bool continuousRendering READ continuousRendering WRITE setContinuousRendering NOTIFY continuousRenderingChanged)
public:
    TangramQuick(QQuickItem *parent = 0);
    ~TangramQuick();

    Renderer *createRenderer() const;

    QUrl sceneConfiguration() const;
    QPointF position() const;
    qreal heading() const;
    float zoom() const;
    int moveAnimationDuration() const;
    bool continuousRendering() const;

public slots:
    void setSceneConfiguration(const QUrl scene);
    void setPosition(const QPointF position);
    void setHeading(const qreal heading);
    void setZoom(const float zoom);
    void setMoveAnimationDuration(const int duration);
    void setContinuousRendering(const bool continuousRendering);

    // TODO: The documents uses a comma separated list on path. Maybe change
    // the to QMap<QString, QString>. But how will that work in QML?
    Q_INVOKABLE void queueSceneUpdate(const QString path, const QString value);
    Q_INVOKABLE void applySceneUpdates();

signals:
    void sceneConfigurationChanged();
    void positionChanged();
    void moveAnimationDurationChanged();
    void headingChanged();
    void zoomChanged();
    void continuousRenderingChanged();

protected:
    bool event(QEvent *ev);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    void setGLInitialized(bool init);

    QUrl m_sceneUrl;
    QPointF m_position;
    int m_moveAnimationDuration;
    qreal m_heading;
    float m_zoom;
    bool m_continuousRendering;

    // Mouse move related stuff
    QPoint m_lastMousePos;
    QPointF m_lastMouseSpeed;
    ulong m_lastMouseEvent;
    bool m_panning;
    bool m_glInit;

    friend class TangramQuickRenderer;
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
    QQuickItem *m_item;
    QMutex m_renderMutex;

    QPointF m_position;
    qreal m_heading;
    float m_zoom;
    int m_moveAnimationDuration;
};

#endif // TANGRAM_H

