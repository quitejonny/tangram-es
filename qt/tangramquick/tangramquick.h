#ifndef TANGRAM_H
#define TANGRAM_H

#include <QQuickFramebufferObject>
#include <QElapsedTimer>
#include <QQuickItem>
#include <QMutex>
#include <data/clientGeoJsonSource.h>

class TangramQuick : public QQuickFramebufferObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TangramQuick)
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QUrl sceneConfiguration READ sceneConfiguration WRITE setSceneConfiguration NOTIFY sceneConfigurationChanged)

public:
    TangramQuick(QQuickItem *parent = 0);
    ~TangramQuick();

    Renderer *createRenderer() const;

    QUrl sceneConfiguration() const;

public slots:
    void setSceneConfiguration(const QUrl scene);

signals:
    void sceneConfigurationChanged();

protected:
    bool event(QEvent *ev);

private:
    QUrl m_sceneUrl;
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
};

#endif // TANGRAM_H

