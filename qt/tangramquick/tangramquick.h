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
    Q_PROPERTY(bool continuousRendering READ continuousRendering WRITE setContinuousRendering NOTIFY continuousRenderingChanged)
public:
    TangramQuick(QQuickItem *parent = 0);
    ~TangramQuick();

    Renderer *createRenderer() const;

    QUrl sceneConfiguration() const;
    QPointF position() const;
    qreal heading() const;
    int moveAnimationDuration() const;
    bool continuousRendering() const;

public slots:
    void setSceneConfiguration(const QUrl scene);
    void setPosition(const QPointF position);
    void setHeading(const qreal heading);
    void setMoveAnimationDuration(const int duration);
    void setContinuousRendering(const bool continuousRendering);

signals:
    void sceneConfigurationChanged();
    void positionChanged();
    void moveAnimationDurationChanged();
    void headingChanged();
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
    int m_moveAnimationDuration;
};

#endif // TANGRAM_H

