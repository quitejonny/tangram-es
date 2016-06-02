#include <QDebug>
#include "tangramwidget.h"

#include <tangram.h>
#include <curl/curl.h>
#include <cstdlib>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include "platform_qt.h"
#include <GL/gl.h>
#include <GL/glx.h>
#include <QRunnable>

PFNGLBINDVERTEXARRAYPROC glBindVertexArrayOESEXT = 0;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArraysOESEXT = 0;
PFNGLGENVERTEXARRAYSPROC glGenVertexArraysOESEXT = 0;

TangramWidget::TangramWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_sceneFile("scene.yaml")
    , m_lastMousePos(-1, -1)
    , m_panning(false)
{
    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    grabGesture(Qt::PanGesture);
}

TangramWidget::~TangramWidget()
{
    curl_global_cleanup();
}

void TangramWidget::initializeGL()
{
    qDebug() << Q_FUNC_INFO << ".--------------";
    glBindVertexArrayOESEXT = (PFNGLBINDVERTEXARRAYPROC)context()->getProcAddress("glBindVertexArray");
    glDeleteVertexArraysOESEXT = (PFNGLDELETEVERTEXARRAYSPROC)context()->getProcAddress("glDeleteVertexArrays");
    glGenVertexArraysOESEXT = (PFNGLGENVERTEXARRAYSPROC)context()->getProcAddress("glGenVertexArrays");

    qDebug() << Q_FUNC_INFO << glBindVertexArrayOESEXT << glDeleteVertexArraysOESEXT << glGenVertexArraysOESEXT;
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    qDebug() << "Version:" << f->glGetString(GL_VERSION);
    f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    Tangram::initialize(m_sceneFile.fileName().toStdString().c_str());
    Tangram::setupGL();
    Tangram::resize(width(),
                    height());

    data_source = std::make_shared<Tangram::ClientGeoJsonSource>("touch", "");
    Tangram::addDataSource(data_source);
}

void TangramWidget::paintGL()
{
    QDateTime now = QDateTime::currentDateTime();
    Tangram::update(1.f);
    Tangram::render();
    m_lastRendering = now;
}

void TangramWidget::resizeGL(int w, int h)
{
    Tangram::resize(w,
                    h);
}

void TangramWidget::mousePressEvent(QMouseEvent *event)
{
    Tangram::handlePanGesture(0.0f, 0.0f, 0.0f, 0.0f);
    m_lastMousePos = event->pos();
    m_lastMouseEvent = event->timestamp();
}

void TangramWidget::mouseMoveEvent(QMouseEvent *event)
{
    Tangram::handlePanGesture(m_lastMousePos.x(), m_lastMousePos.y(),
                              event->x(), event->y());

    if (m_panning && (event->timestamp() - m_lastMouseEvent) != 0) {
        m_lastMouseSpeed.setX((event->x() - m_lastMousePos.x()) / ((float)(event->timestamp() - m_lastMouseEvent) / 1000.f));
        m_lastMouseSpeed.setY((event->y() - m_lastMousePos.y()) / ((float)(event->timestamp() - m_lastMouseEvent) / 1000.f));
    }

    m_lastMousePos = event->pos();
    m_lastMouseEvent = event->timestamp();
    m_panning = true;
}

void TangramWidget::mouseReleaseEvent(QMouseEvent *event)
{
    /*if (m_panning) {
        Tangram::handleFlingGesture(event->x(), event->y(),
                                    qBound(-2000.0, m_lastMouseSpeed.x(), 2000.0),
                                    qBound(-2000.0, m_lastMouseSpeed.y(), 2000.0));
        m_panning = false;
        m_lastMouseSpeed.setX(0.);
        m_lastMouseSpeed.setY(0.);
    }*/
}

void TangramWidget::grabGestures(const QList<Qt::GestureType> &gestures)
{
    foreach (Qt::GestureType gesture, gestures)
        grabGesture(gesture);
}

bool TangramWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent*>(e));
    else if (e->type() == QEvent::Wheel)
        return mouseWheelEvent(static_cast<QWheelEvent*>(e));
    else if (e->type() == TANGRAM_REQ_RENDER_EVENT_TYPE)
        return renderRequestEvent();
    return QWidget::event(e);
}

bool TangramWidget::renderRequestEvent()
{
    processNetworkQueue();
    update();
    return true;
}

bool TangramWidget::mouseWheelEvent(QWheelEvent *ev)
{
    double x = ev->posF().x();
    double y = ev->posF().y();
    if (ev->modifiers() & Qt::ControlModifier)
        Tangram::handleShoveGesture(0.05 * ev->angleDelta().y());
    else if (ev->modifiers() & Qt::AltModifier)
        Tangram::handleRotateGesture(x, y, 0.0005 * ev->angleDelta().x());
    else
        Tangram::handlePinchGesture(x, y, 1.0 + 0.0005 * ev->angleDelta().y(), 0.f);
}

bool TangramWidget::gestureEvent(QGestureEvent *ev)
{
    if (QGesture *swipe = ev->gesture(Qt::SwipeGesture))
        swipeTriggered(static_cast<QSwipeGesture*>(swipe));
    else if (QGesture *pan = ev->gesture(Qt::PanGesture))
        panTriggered(static_cast<QPanGesture *>(pan));
    if (QGesture *pinch = ev->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
    return true;
}

void TangramWidget::panTriggered(QPanGesture *gesture)
{
#ifndef QT_NO_CURSOR
    switch (gesture->state()) {
        case Qt::GestureStarted:
        case Qt::GestureUpdated:
            setCursor(Qt::SizeAllCursor);
            break;
        default:
            setCursor(Qt::ArrowCursor);
    }
#endif
    QPointF delta = gesture->delta();
    qDebug() << "panTriggered():" << gesture;
    horizontalOffset += delta.x();
    verticalOffset += delta.y();
    update();
}

void TangramWidget::pinchTriggered(QPinchGesture *gesture)
{
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::RotationAngleChanged) {
        qreal rotationDelta = gesture->rotationAngle() - gesture->lastRotationAngle();
        rotationAngle += rotationDelta;
        qDebug() << "pinchTriggered(): rotate by" <<
            rotationDelta << "->" << rotationAngle;
    }
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        currentStepScaleFactor = gesture->totalScaleFactor();
        qDebug() << "pinchTriggered(): zoom by" <<
            gesture->scaleFactor() << "->" << currentStepScaleFactor;
    }
    if (gesture->state() == Qt::GestureFinished) {
        scaleFactor *= currentStepScaleFactor;
        currentStepScaleFactor = 1;
    }
    update();
}

void TangramWidget::swipeTriggered(QSwipeGesture *gesture)
{
    if (gesture->state() == Qt::GestureFinished) {
        if (gesture->horizontalDirection() == QSwipeGesture::Left
            || gesture->verticalDirection() == QSwipeGesture::Up) {
            qDebug() << "swipeTriggered(): swipe to previous";
        } else {
            qDebug() << "swipeTriggered(): swipe to next";
        }
        update();
    }
}

class UrlRunner : public QRunnable
{
public:
    UrlRunner(QUrl url, UrlCallback callback) {
        m_url = url;
        m_callback = callback;
    }

    void run() {

    }

private:
    QUrl m_url;
    UrlCallback m_callback;
};

void TangramWidget::startUrlRequest(const std::__cxx11::string &url, UrlCallback callback)
{

}

void TangramWidget::cancelUrlRequest(const std::__cxx11::string &url)
{

}

void TangramWidget::handleCallback(UrlCallback callback)
{

}
