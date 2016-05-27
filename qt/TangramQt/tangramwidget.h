#ifndef TANGELGLWIDGET_H
#define TANGELGLWIDGET_H

#include <QOpenGLWidget>
#include <QFile>
#include <data/clientGeoJsonSource.h>
#include <QMouseEvent>
#include <QDateTime>
#include "platform.h"

class TangramWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    TangramWidget(QWidget *parent = Q_NULLPTR);
    virtual ~TangramWidget();

    static void startUrlRequest(const std::string &url, UrlCallback callback);
    static void cancelUrlRequest(const std::string &url);

public slots:
    void handleCallback(UrlCallback callback);

protected:
    void timerEvent(QTimerEvent *ev);

    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QFile m_sceneFile;
    std::shared_ptr<Tangram::ClientGeoJsonSource> data_source;

    QPoint m_startPoint;
    QPoint m_lastPoint;
    QDateTime m_lastRendering;
};

#endif // TANGELGLWIDGET_H
