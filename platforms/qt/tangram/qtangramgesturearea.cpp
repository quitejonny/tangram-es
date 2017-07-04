#include "qtangramgesturearea.h"
#include "qtangrammapcontroller.h"
#include "qtangrammap.h"
#include <tangram.h>
#include "tangramquick.h"
#include "qtangrampoint.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QWheelEvent>
#include <QtGui/QStyleHints>
#include <QtQuick/QQuickWindow>
#include <QDebug>
#include "math.h"
#include <cmath>


#define QML_MAP_FLICK_VELOCITY_SAMPLE_PERIOD 38
// Tolerance for detecting two finger sliding start
static const qreal MaximumParallelPosition = 40.0; // in degrees
// Tolerance for detecting parallel sliding
static const qreal MaximumParallelSlidingAngle = 6.0; // in degrees
// Tolerance for starting rotation
static const qreal MinimumRotationStartingAngle = 20.0; // in degrees
// Tolerance for starting pinch
static const qreal MinimumPinchDelta = 40; // in pixels
// Tolerance for starting tilt when sliding vertical
static const qreal MinimumPanToTiltDelta = 40; // in pixels;

static qreal distanceBetweenTouchPoints(const QPointF &p1, const QPointF &p2)
{
    return QLineF(p1, p2).length();
}

static qreal angleFromPoints(const QPointF &p1, const QPointF &p2)
{
    return QLineF(p1, p2).angle();
}

// Keeps it in +- 180
static qreal touchAngle(const QPointF &p1, const QPointF &p2)
{
    qreal angle = angleFromPoints(p1, p2);
    if (angle > 180)
        angle -= 360;
    return angle;
}

// Deals with angles crossing the +-180 edge, assumes that the delta can't be > 180
static qreal angleDelta(const qreal angle1, const qreal angle2)
{
    qreal delta = angle1 - angle2;
    if (delta > 180.0) // detect crossing angle1 positive, angle2 negative, rotation counterclockwise, difference negative
        delta = angle1 - angle2 - 360.0;
    else if (delta < -180.0) // detect crossing angle1 negative, angle2 positive, rotation clockwise, difference positive
        delta = angle1 - angle2 + 360.0;

    return delta;
}

static bool pointDragged(const QPointF &pOld, const QPointF &pNew)
{
    static const int startDragDistance = qApp->styleHints()->startDragDistance();
    return ( qAbs(pNew.x() - pOld.x()) > startDragDistance
             || qAbs(pNew.y() - pOld.y()) > startDragDistance);
}

static qreal vectorSize(const QPointF &vector)
{
    return std::sqrt(vector.x() * vector.x() + vector.y() * vector.y());
}

// This linearizes the angles around 0, and keep it linear around 180, allowing to differentiate
// touch angles that are supposed to be parallel (0 or 180 depending on what finger goes first)
static qreal touchAngleTilting(const QPointF &p1, const QPointF &p2)
{
    qreal angle = angleFromPoints(p1, p2);
    if (angle > 270)
        angle -= 360;
    return angle;
}

static bool movingParallelVertical(const QPointF &p1old, const QPointF &p1new, const QPointF &p2old, const QPointF &p2new)
{
    if (!pointDragged(p1old, p1new) || !pointDragged(p2old, p2new))
        return false;

    QPointF v1 = p1new - p1old;
    QPointF v2 = p2new - p2old;
    qreal v1v2size = vectorSize(v1 + v2);

    if (v1v2size < vectorSize(v1) || v1v2size < vectorSize(v2)) // going in opposite directions
        return false;

    const qreal newAngle = touchAngleTilting(p1new, p2new);
    const qreal oldAngle = touchAngleTilting(p1old, p2old);
    const qreal angleDiff = angleDelta(newAngle, oldAngle);

    if (qAbs(angleDiff) > MaximumParallelSlidingAngle)
        return false;

    return true;
}





QTangramGestureArea::QTangramGestureArea(QDeclarativeTangramMap *map)
    : QQuickItem(map),
      m_map(0),
      m_declarativeMap(map),
      m_enabled(true),
      m_acceptedGestures(PinchGesture | PanGesture | FlickGesture | RotationGesture | TiltGesture),
      m_preventStealing(false)
{
    setMap(m_declarativeMap->m_map);
    m_touchPointState = touchPoints0;
    m_pinchState = pinchInactive;
    m_flickState = flickInactive;
    m_rotationState = rotationInactive;
    m_tiltState = tiltInactive;
    m_dragState = dragInactive;
}

void QTangramGestureArea::setMap(QPointer<QTangramMap> map)
{
    if (m_map || !map)
        return;

    m_map = map;
}

bool QTangramGestureArea::preventStealing() const
{
    return m_preventStealing;
}

void QTangramGestureArea::setPreventStealing(bool prevent)
{
    if (prevent != m_preventStealing) {
        m_preventStealing = prevent;
        m_declarativeMap->setKeepMouseGrab(m_preventStealing && m_enabled);
        m_declarativeMap->setKeepTouchGrab(m_preventStealing && m_enabled);
        emit preventStealingChanged();
    }
}

QTangramGestureArea::~QTangramGestureArea()
{
}

QTangramGestureArea::AcceptedGestures QTangramGestureArea::acceptedGestures() const
{
    return m_acceptedGestures;
}


void QTangramGestureArea::setAcceptedGestures(AcceptedGestures acceptedGestures)
{
    if (acceptedGestures == m_acceptedGestures)
        return;
    m_acceptedGestures = acceptedGestures;

    if (enabled()) {
        setPanEnabled(acceptedGestures & PanGesture);
        setFlickEnabled(acceptedGestures & FlickGesture);
        setPinchEnabled(acceptedGestures & PinchGesture);
        setRotationEnabled(acceptedGestures & RotationGesture);
        setTiltEnabled(acceptedGestures & TiltGesture);
    }

    emit acceptedGesturesChanged();
}

bool QTangramGestureArea::isPinchActive() const
{
    return m_pinchState == pinchActive;
}

bool QTangramGestureArea::isRotationActive() const
{
    return m_rotationState == rotationActive;
}

bool QTangramGestureArea::isTiltActive() const
{
    return m_tiltState == tiltActive;
}

bool QTangramGestureArea::isPanActive() const
{
    return m_flickState == panActive;
}

bool QTangramGestureArea::isDragActive() const
{
    return m_dragState == dragActive || m_dragState == dragReady;
}

bool QTangramGestureArea::enabled() const
{
    return m_enabled;
}

void QTangramGestureArea::setEnabled(bool enabled)
{
    if (enabled == m_enabled)
        return;
    m_enabled = enabled;

    if (enabled) {
        setPanEnabled(m_acceptedGestures & PanGesture);
        setFlickEnabled(m_acceptedGestures & FlickGesture);
        setPinchEnabled(m_acceptedGestures & PinchGesture);
        setRotationEnabled(m_acceptedGestures & RotationGesture);
        setTiltEnabled(m_acceptedGestures & TiltGesture);
    } else {
        setPanEnabled(false);
        setFlickEnabled(false);
        setPinchEnabled(false);
        setRotationEnabled(false);
        setTiltEnabled(false);
    }

    emit enabledChanged();
}

bool QTangramGestureArea::pinchEnabled() const
{
    return m_pinch.m_pinchEnabled;
}

void QTangramGestureArea::setPinchEnabled(bool enabled)
{
    m_pinch.m_pinchEnabled = enabled;
}

bool QTangramGestureArea::rotationEnabled() const
{
    return m_pinch.m_rotationEnabled;
}

void QTangramGestureArea::setRotationEnabled(bool enabled)
{
    m_pinch.m_rotationEnabled = enabled;
}

bool QTangramGestureArea::tiltEnabled() const
{
    return m_pinch.m_tiltEnabled;
}

void QTangramGestureArea::setTiltEnabled(bool enabled)
{
    m_pinch.m_tiltEnabled = enabled;
}

bool QTangramGestureArea::panEnabled() const
{
    return m_flick.m_panEnabled;
}

void QTangramGestureArea::setPanEnabled(bool enabled)
{
    if (enabled == m_flick.m_panEnabled)
        return;
    m_flick.m_panEnabled = enabled;

    // unlike the pinch, the pan existing functionality is to stop immediately
    if (!enabled) {
        stopPan();
        m_flickState = flickInactive;
    }
}

bool QTangramGestureArea::flickEnabled() const
{
    return m_flick.m_flickEnabled;
}

void QTangramGestureArea::setFlickEnabled(bool enabled)
{
    if (enabled == m_flick.m_flickEnabled)
        return;
    m_flick.m_flickEnabled = enabled;
    // unlike the pinch, the flick existing functionality is to stop immediately
    if (!enabled) {
        bool stateActive = (m_flickState != flickInactive);
        if (stateActive) {
            if (m_flick.m_panEnabled)
                m_flickState = panActive;
            else
                m_flickState = flickInactive;
        }
    }
}

QTouchEvent::TouchPoint* createTouchPointFromMouseEvent(QMouseEvent *event, Qt::TouchPointState state)
{
    // this is only partially filled. But since it is only partially used it works
    // more robust would be to store a list of QPointFs rather than TouchPoints
    QTouchEvent::TouchPoint* newPoint = new QTouchEvent::TouchPoint();
    newPoint->setPos(event->localPos());
    newPoint->setScenePos(event->windowPos());
    newPoint->setScreenPos(event->screenPos());
    newPoint->setState(state);
    newPoint->setId(0);
    return newPoint;
}

void QTangramGestureArea::handleMousePressEvent(QMouseEvent *event)
{
    m_mousePoint.reset(createTouchPointFromMouseEvent(event, Qt::TouchPointPressed));
    if (m_touchPoints.isEmpty()) update();
    event->accept();
}

void QTangramGestureArea::handleMouseMoveEvent(QMouseEvent *event)
{
    m_mousePoint.reset(createTouchPointFromMouseEvent(event, Qt::TouchPointMoved));
    if (m_touchPoints.isEmpty()) update();
    event->accept();
}

void QTangramGestureArea::handleMouseReleaseEvent(QMouseEvent *event)
{
    if (!m_mousePoint.isNull()) {
        //this looks super ugly , however is required in case we do not get synthesized MouseReleaseEvent
        //and we reset the point already in handleTouchUngrabEvent
        m_mousePoint.reset(createTouchPointFromMouseEvent(event, Qt::TouchPointReleased));
        if (m_touchPoints.isEmpty()) update();
    }
    event->accept();
}

void QTangramGestureArea::handleMouseUngrabEvent()
{

    if (m_touchPoints.isEmpty() && !m_mousePoint.isNull()) {
        m_mousePoint.reset();
        update();
    } else {
        m_mousePoint.reset();
    }
}

void QTangramGestureArea::handleTouchUngrabEvent()
{
    m_touchPoints.clear();
    //this is needed since in some cases mouse release is not delivered
    //(second touch point breaks mouse synthesized events)
    m_mousePoint.reset();
    update();
}

void QTangramGestureArea::handleTouchEvent(QTouchEvent *event)
{
    m_touchPoints.clear();
    m_mousePoint.reset();

    for (int i = 0; i < event->touchPoints().count(); ++i) {
        auto point = event->touchPoints().at(i);
        if (point.state() != Qt::TouchPointReleased)
            m_touchPoints << point;
    }
    if (event->touchPoints().count() >= 2)
        event->accept();
    else
        event->ignore();
    update();
}

void QTangramGestureArea::handleWheelEvent(QWheelEvent *event)
{
    if (!m_map)
        return;

    qreal scaleDelta = event->angleDelta().y();
    if (qAbs(scaleDelta) > 20)
        scaleDelta = 20*scaleDelta/abs(scaleDelta);
    qreal scale = 1 + qreal(0.003)*scaleDelta;
    m_map->tangramObject()->handlePinchGesture(event->pos().x(), event->pos().y(), scale, 0.f);

    emit m_map->mapController()->zoomChanged(m_map->mapController()->zoom());
    event->accept();
}

void QTangramGestureArea::clearTouchData()
{
    m_flickVector = QVector2D();
    m_touchPointsCentroid.setX(0);
    m_touchPointsCentroid.setY(0);
}


void QTangramGestureArea::updateFlickParameters(const QPointF &pos)
{
    // Take velocity samples every sufficient period of time, used later to determine the flick
    // duration and speed (when mouse is released).
    qreal elapsed = qreal(m_lastPosTime.elapsed());

    if (elapsed >= QML_MAP_FLICK_VELOCITY_SAMPLE_PERIOD) {
        elapsed /= 1000.;
        m_flickVector = (QVector2D(pos) - QVector2D(m_lastPos)) / elapsed;

        m_lastPos = pos;
        m_lastPosTime.restart();
    }
}

void QTangramGestureArea::setTouchPointState(const QTangramGestureArea::TouchPointState state)
{
    m_touchPointState = state;
}

void QTangramGestureArea::setFlickState(const QTangramGestureArea::FlickState state)
{
    m_flickState = state;
}

void QTangramGestureArea::setTiltState(const QTangramGestureArea::TiltState state)
{
    m_tiltState = state;
}

void QTangramGestureArea::setRotationState(const QTangramGestureArea::RotationState state)
{
    m_rotationState = state;
}

void QTangramGestureArea::setPinchState(const QTangramGestureArea::PinchState state)
{
    m_pinchState = state;
}

void QTangramGestureArea::setDragState(const QTangramGestureArea::DragState state)
{
    m_dragState = state;
}

bool QTangramGestureArea::isActive() const
{
    return isPanActive() || isPinchActive() || isRotationActive() || isTiltActive();
}

// simplify the gestures by using a state-machine format (easy to move to a future state machine)
void QTangramGestureArea::update()
{
    if (!m_map)
        return;
    // First state machine is for the number of touch points

    //combine touch with mouse event
    m_allPoints.clear();
    m_allPoints << m_touchPoints;
    if (m_allPoints.isEmpty() && !m_mousePoint.isNull())
        m_allPoints << *m_mousePoint.data();

    TouchPointState touchPointStateLast = m_touchPointState;
    touchPointStateMachine();
    if (m_touchPointState != touchPointStateLast)
        m_touchPointsCentroidLast = m_touchPointsCentroid;

    // Parallel state machine for tilt. Tilt goes first as it blocks anything else, when started.
    // But tilting can also only start if nothing else is active.
    if (isTiltActive() || m_pinch.m_tiltEnabled)
        tiltStateMachine();

    // Parallel state machine for pinch
    if (isPinchActive() || m_pinch.m_pinchEnabled)
        pinchStateMachine();

    // Parallel state machine for rotation.
    if (isRotationActive() || m_pinch.m_rotationEnabled)
        rotationStateMachine();

    // Parallel state machine for dragging tangram items
    if (isDragActive() || (touchPointStateLast == touchPoints0 && m_touchPointState == touchPoints1))
        dragStateMachine();

    // Parallel state machine for pan (since you can pan at the same time as pinching)
    // The stopPan function ensures that pan stops immediately when disabled,
    // but the isPanActive() below allows pan continue its current gesture if you disable
    // the whole gesture.
    // Pan goes last because it does reanchoring in updatePan()  which makes the map
    // properly rotate around the touch point centroid.
    if (isPanActive() || m_flick.m_flickEnabled || m_flick.m_panEnabled)
        panStateMachine();
}

void QTangramGestureArea::touchPointStateMachine()
{
    // Transitions:
    switch (m_touchPointState) {
    case touchPoints0:
        if (m_allPoints.count() == 1) {
            clearTouchData();
            startOneTouchPoint();
            setTouchPointState(touchPoints1);
        } else if (m_allPoints.count() >= 2) {
            clearTouchData();
            startTwoTouchPoints();
            setTouchPointState(touchPoints2);
        }
        break;
    case touchPoints1:
        if (m_allPoints.count() == 0) {
            tryToClick();
            setTouchPointState(touchPoints0);
        } else if (m_allPoints.count() == 2) {
            startTwoTouchPoints();
            setTouchPointState(touchPoints2);
        }
        break;
    case touchPoints2:
        if (m_allPoints.count() == 0) {
            setTouchPointState(touchPoints0);
        } else if (m_allPoints.count() == 1) {
            startOneTouchPoint();
            setTouchPointState(touchPoints1);
        }
        break;
    };

    // Update
    switch (m_touchPointState) {
    case touchPoints0:
        break; // do nothing if no touch points down
    case touchPoints1:
        updateOneTouchPoint();
        break;
    case touchPoints2:
        updateTwoTouchPoints();
        break;
    }
}

void QTangramGestureArea::startOneTouchPoint()
{
    m_sceneStartPoint1 = mapFromScene(m_allPoints.at(0).scenePos());
    m_lastPos = m_sceneStartPoint1;
    m_lastPosTime.start();
}

void QTangramGestureArea::updateOneTouchPoint()
{
    m_touchPointsCentroid = mapFromScene(m_allPoints.at(0).scenePos());
    updateFlickParameters(m_touchPointsCentroid);
}

void QTangramGestureArea::startTwoTouchPoints()
{
    m_sceneStartPoint1 = mapFromScene(m_allPoints.at(0).scenePos());
    m_sceneStartPoint2 = mapFromScene(m_allPoints.at(1).scenePos());
    QPointF startPos = (m_sceneStartPoint1 + m_sceneStartPoint2) * 0.5;
    m_lastPos = startPos;
    m_lastPosTime.start();
    m_twoTouchAngleStart = touchAngle(m_sceneStartPoint1, m_sceneStartPoint2); // Initial angle used for calculating rotation
    m_distanceBetweenTouchPointsStart = distanceBetweenTouchPoints(m_sceneStartPoint1, m_sceneStartPoint2);
    m_twoTouchPointsCentroidStart = (m_sceneStartPoint1 + m_sceneStartPoint2) / 2;
}

void QTangramGestureArea::updateTwoTouchPoints()
{
    QPointF p1 = mapFromScene(m_allPoints.at(0).scenePos());
    QPointF p2 = mapFromScene(m_allPoints.at(1).scenePos());
    m_distanceBetweenTouchPoints = distanceBetweenTouchPoints(p1, p2);
    m_touchPointsCentroid = (p1 + p2) / 2;
    updateFlickParameters(m_touchPointsCentroid);

    m_twoTouchAngle = touchAngle(p1, p2);
}

void QTangramGestureArea::tiltStateMachine()
{
    TiltState lastState = m_tiltState;
    // Transitions:
    switch (m_tiltState) {
    case tiltInactive:
        if (m_allPoints.count() >= 2) {
            if (!isRotationActive() && !isPinchActive() && canStartTilt()) { // only gesture that can be overridden: pan/flick
                m_declarativeMap->setKeepMouseGrab(true);
                m_declarativeMap->setKeepTouchGrab(true);
                startTilt();
                setTiltState(tiltActive);
            } else {
                setTiltState(tiltInactiveTwoPoints);
            }
        }
        break;
    case tiltInactiveTwoPoints:
        if (m_allPoints.count() <= 1) {
            setTiltState(tiltInactive);
        } else {
            if (!isRotationActive() && !isPinchActive() && canStartTilt()) { // only gesture that can be overridden: pan/flick
                m_declarativeMap->setKeepMouseGrab(true);
                m_declarativeMap->setKeepTouchGrab(true);
                startTilt();
                setTiltState(tiltActive);
            }
        }
        break;
    case tiltActive:
        if (m_allPoints.count() <= 1) {
            setTiltState(tiltInactive);
            m_declarativeMap->setKeepMouseGrab(m_preventStealing);
            m_declarativeMap->setKeepTouchGrab(m_preventStealing);
            endTilt();
        }
        break;
    }
    // This line implements an exclusive state machine, where the transitions and updates don't
    // happen on the same frame
    if (m_tiltState != lastState) {
        emit tiltActiveChanged();
        return;
    }

    // Update
    switch (m_tiltState) {
    case tiltInactive:
    case tiltInactiveTwoPoints:
        break; // do nothing
    case tiltActive:
        updateTilt();
        break;
    }
}

bool validateTouchAngleForTilting(const qreal angle)
{
    return ((qAbs(angle) - 180.0) < MaximumParallelPosition) || (qAbs(angle) < MaximumParallelPosition);
}

bool QTangramGestureArea::canStartTilt()
{
    if (m_allPoints.count() >= 2) {
        QPointF p1 = mapFromScene(m_allPoints.at(0).scenePos());
        QPointF p2 = mapFromScene(m_allPoints.at(1).scenePos());
        if (validateTouchAngleForTilting(m_twoTouchAngle)
                && movingParallelVertical(m_sceneStartPoint1, p1, m_sceneStartPoint2, p2)
                && qAbs(m_twoTouchPointsCentroidStart.y() - m_touchPointsCentroid.y()) > MinimumPanToTiltDelta) {
            m_pinch.m_event.setCenter(mapFromScene(m_touchPointsCentroid));
            m_pinch.m_event.setAngle(m_twoTouchAngle);
            m_pinch.m_event.setPoint1(p1);
            m_pinch.m_event.setPoint2(p2);
            m_pinch.m_event.setPointCount(m_allPoints.count());
            m_pinch.m_event.setAccepted(true);
            emit tiltStarted(&m_pinch.m_event);
            return true;
        }
    }
    return false;
}

void QTangramGestureArea::startTilt()
{
    if (isPanActive()) {
        stopPan();
        setFlickState(flickInactive);
    }

    m_pinch.m_tilt.m_lastTouchCentroid = m_touchPointsCentroid;
}

void QTangramGestureArea::updateTilt()
{
    // Calculate the new tilt
    qreal verticalDisplacement = (m_touchPointsCentroid - m_pinch.m_tilt.m_lastTouchCentroid).y();
    m_map->tangramObject()->handleShoveGesture(verticalDisplacement);

    m_pinch.m_tilt.m_lastTouchCentroid = m_touchPointsCentroid;
    m_pinch.m_event.setCenter(mapFromScene(m_touchPointsCentroid));
    m_pinch.m_event.setAngle(m_twoTouchAngle);
    m_pinch.m_lastPoint1 = mapFromScene(m_allPoints.at(0).scenePos());
    m_pinch.m_lastPoint2 = mapFromScene(m_allPoints.at(1).scenePos());
    m_pinch.m_event.setPoint1(m_pinch.m_lastPoint1);
    m_pinch.m_event.setPoint2(m_pinch.m_lastPoint2);
    m_pinch.m_event.setPointCount(m_allPoints.count());
    m_pinch.m_event.setAccepted(true);

    emit tiltUpdated(&m_pinch.m_event);
}

void QTangramGestureArea::endTilt()
{
    QPointF p1 = mapFromScene(m_pinch.m_lastPoint1);
    QPointF p2 = mapFromScene(m_pinch.m_lastPoint2);
    m_pinch.m_event.setCenter((p1 + p2) / 2);
    m_pinch.m_event.setAngle(m_pinch.m_lastAngle);
    m_pinch.m_event.setPoint1(p1);
    m_pinch.m_event.setPoint2(p2);
    m_pinch.m_event.setAccepted(true);
    m_pinch.m_event.setPointCount(0);
    emit tiltFinished(&m_pinch.m_event);
}

void QTangramGestureArea::rotationStateMachine()
{
    RotationState lastState = m_rotationState;
    // Transitions:
    switch (m_rotationState) {
    case rotationInactive:
        if (m_allPoints.count() >= 2) {
            if (!isTiltActive() && canStartRotation()) {
                m_declarativeMap->setKeepMouseGrab(true);
                m_declarativeMap->setKeepTouchGrab(true);
                startRotation();
                setRotationState(rotationActive);
            } else {
                setRotationState(rotationInactiveTwoPoints);
            }
        }
        break;
    case rotationInactiveTwoPoints:
        if (m_allPoints.count() <= 1) {
            setRotationState(rotationInactive);
        } else {
            if (!isTiltActive() && canStartRotation()) {
                m_declarativeMap->setKeepMouseGrab(true);
                m_declarativeMap->setKeepTouchGrab(true);
                startRotation();
                setRotationState(rotationActive);
            }
        }
        break;
    case rotationActive:
        if (m_allPoints.count() <= 1) {
            setRotationState(rotationInactive);
            m_declarativeMap->setKeepMouseGrab(m_preventStealing);
            m_declarativeMap->setKeepTouchGrab(m_preventStealing);
            endRotation();
        }
        break;
    }
    // This line implements an exclusive state machine, where the transitions and updates don't
    // happen on the same frame
    if (m_rotationState != lastState) {
        emit rotationActiveChanged();
        return;
    }

    // Update
    switch (m_rotationState) {
    case rotationInactive:
    case rotationInactiveTwoPoints:
        break; // do nothing
    case rotationActive:
        updateRotation();
        break;
    }
}

bool QTangramGestureArea::canStartRotation()
{
    if (m_allPoints.count() >= 2) {
        QPointF p1 = mapFromScene(m_allPoints.at(0).scenePos());
        QPointF p2 = mapFromScene(m_allPoints.at(1).scenePos());
        if (pointDragged(m_sceneStartPoint1, p1) || pointDragged(m_sceneStartPoint2, p2)) {
            qreal delta = angleDelta(m_twoTouchAngleStart, m_twoTouchAngle);
            if (qAbs(delta) < MinimumRotationStartingAngle) {
                return false;
            }
            m_pinch.m_event.setCenter(mapFromScene(m_touchPointsCentroid));
            m_pinch.m_event.setAngle(m_twoTouchAngle);
            m_pinch.m_event.setPoint1(p1);
            m_pinch.m_event.setPoint2(p2);
            m_pinch.m_event.setPointCount(m_allPoints.count());
            m_pinch.m_event.setAccepted(true);
            emit rotationStarted(&m_pinch.m_event);
            return m_pinch.m_event.accepted();
        }
    }
    return false;
}

void QTangramGestureArea::startRotation()
{
    m_pinch.m_rotation.m_previousTouchAngle = m_twoTouchAngle;
}

void QTangramGestureArea::updateRotation()
{
    // Calculate the new bearing
    qreal angle = angleDelta(m_pinch.m_rotation.m_previousTouchAngle, m_twoTouchAngle);
    if (qAbs(angle) < 0.2) // avoiding too many updates
        return;

    m_map->tangramObject()->handleRotateGesture(m_touchPointsCentroid.x(), m_touchPointsCentroid.y(), angle/180.0*M_PI);
    m_pinch.m_rotation.m_previousTouchAngle = m_twoTouchAngle;

    m_pinch.m_event.setCenter(mapFromScene(m_touchPointsCentroid));
    m_pinch.m_event.setAngle(m_twoTouchAngle);
    m_pinch.m_lastPoint1 = mapFromScene(m_allPoints.at(0).scenePos());
    m_pinch.m_lastPoint2 = mapFromScene(m_allPoints.at(1).scenePos());
    m_pinch.m_event.setPoint1(m_pinch.m_lastPoint1);
    m_pinch.m_event.setPoint2(m_pinch.m_lastPoint2);
    m_pinch.m_event.setPointCount(m_allPoints.count());
    m_pinch.m_event.setAccepted(true);

    emit rotationUpdated(&m_pinch.m_event);
}

void QTangramGestureArea::endRotation()
{
    QPointF p1 = mapFromScene(m_pinch.m_lastPoint1);
    QPointF p2 = mapFromScene(m_pinch.m_lastPoint2);
    m_pinch.m_event.setCenter((p1 + p2) / 2);
    m_pinch.m_event.setAngle(m_pinch.m_lastAngle);
    m_pinch.m_event.setPoint1(p1);
    m_pinch.m_event.setPoint2(p2);
    m_pinch.m_event.setAccepted(true);
    m_pinch.m_event.setPointCount(0);
    emit rotationFinished(&m_pinch.m_event);
}

void QTangramGestureArea::pinchStateMachine()
{
    PinchState lastState = m_pinchState;
    // Transitions:
    switch (m_pinchState) {
    case pinchInactive:
        if (m_allPoints.count() >= 2) {
            if (!isTiltActive() && canStartPinch()) {
                m_declarativeMap->setKeepMouseGrab(true);
                m_declarativeMap->setKeepTouchGrab(true);
                startPinch();
                setPinchState(pinchActive);
            } else {
                setPinchState(pinchInactiveTwoPoints);
            }
        }
        break;
    case pinchInactiveTwoPoints:
        if (m_allPoints.count() <= 1) {
            setPinchState(pinchInactive);
        } else {
            if (!isTiltActive() && canStartPinch()) {
                m_declarativeMap->setKeepMouseGrab(true);
                m_declarativeMap->setKeepTouchGrab(true);
                startPinch();
                setPinchState(pinchActive);
            }
        }
        break;
    case pinchActive:
        if (m_allPoints.count() <= 1) { // Once started, pinch goes off only when finger(s) are release
            setPinchState(pinchInactive);
            m_declarativeMap->setKeepMouseGrab(m_preventStealing);
            m_declarativeMap->setKeepTouchGrab(m_preventStealing);
            endPinch();
        }
        break;
    }
    // This line implements an exclusive state machine, where the transitions and updates don't
    // happen on the same frame
    if (m_pinchState != lastState) {
        emit pinchActiveChanged();
        return;
    }

    // Update
    switch (m_pinchState) {
    case pinchInactive:
    case pinchInactiveTwoPoints:
        break; // do nothing
    case pinchActive:
        updatePinch();
        break;
    }
}

bool QTangramGestureArea::canStartPinch()
{
    if (m_allPoints.count() >= 2) {
        QPointF p1 = mapFromScene(m_allPoints.at(0).scenePos());
        QPointF p2 = mapFromScene(m_allPoints.at(1).scenePos());
        if (qAbs(m_distanceBetweenTouchPoints - m_distanceBetweenTouchPointsStart) > MinimumPinchDelta) {
            m_pinch.m_event.setCenter(mapFromScene(m_touchPointsCentroid));
            m_pinch.m_event.setAngle(m_twoTouchAngle);
            m_pinch.m_event.setPoint1(p1);
            m_pinch.m_event.setPoint2(p2);
            m_pinch.m_event.setPointCount(m_allPoints.count());
            m_pinch.m_event.setAccepted(true);
            emit pinchStarted(&m_pinch.m_event);
            return m_pinch.m_event.accepted();
        }
    }
    return false;
}

void QTangramGestureArea::startPinch()
{
    m_pinch.m_startDist = m_distanceBetweenTouchPoints;
    m_pinch.m_zoom.m_previous = m_map->mapController()->zoom();
    m_pinch.m_lastAngle = m_twoTouchAngle;
    m_distanceBetweenTouchPointsLast = m_distanceBetweenTouchPoints;

    m_pinch.m_lastPoint1 = mapFromScene(m_allPoints.at(0).scenePos());
    m_pinch.m_lastPoint2 = mapFromScene(m_allPoints.at(1).scenePos());

    m_pinch.m_zoom.m_start = m_map->mapController()->zoom();
}

void QTangramGestureArea::updatePinch()
{
    // Calculate the new zoom level if we have distance ( >= 2 touchpoints), otherwise stick with old.
    //qreal newZoomLevel = m_pinch.m_zoom.m_previous;

    qreal scale = 1 + 0.005*(m_distanceBetweenTouchPoints - m_distanceBetweenTouchPointsLast);
    m_map->tangramObject()->handlePinchGesture(m_touchPointsCentroid.x(), m_touchPointsCentroid.y(), scale, 0.f);

    m_pinch.m_event.setCenter(mapFromScene(m_touchPointsCentroid));
    m_pinch.m_event.setAngle(m_twoTouchAngle);

    m_pinch.m_lastPoint1 = mapFromScene(m_allPoints.at(0).scenePos());
    m_pinch.m_lastPoint2 = mapFromScene(m_allPoints.at(1).scenePos());
    m_pinch.m_event.setPoint1(m_pinch.m_lastPoint1);
    m_pinch.m_event.setPoint2(m_pinch.m_lastPoint2);
    m_pinch.m_event.setPointCount(m_allPoints.count());
    m_pinch.m_event.setAccepted(true);

    m_pinch.m_lastAngle = m_twoTouchAngle;
    emit pinchUpdated(&m_pinch.m_event);

    if (m_acceptedGestures & PinchGesture) {
        m_distanceBetweenTouchPointsLast = m_distanceBetweenTouchPoints;
        m_pinch.m_zoom.m_previous = m_map->mapController()->zoom();
        emit m_map->mapController()->zoomChanged(m_pinch.m_zoom.m_previous);
    }
}

void QTangramGestureArea::endPinch()
{
    QPointF p1 = mapFromScene(m_pinch.m_lastPoint1);
    QPointF p2 = mapFromScene(m_pinch.m_lastPoint2);
    m_pinch.m_event.setCenter((p1 + p2) / 2);
    m_pinch.m_event.setAngle(m_pinch.m_lastAngle);
    m_pinch.m_event.setPoint1(p1);
    m_pinch.m_event.setPoint2(p2);
    m_pinch.m_event.setAccepted(true);
    m_pinch.m_event.setPointCount(0);
    emit pinchFinished(&m_pinch.m_event);
    m_pinch.m_startDist = 0;
}

void QTangramGestureArea::panStateMachine()
{
    FlickState lastState = m_flickState;

    // Transitions
    switch (m_flickState) {
    case flickInactive:
        if (!isTiltActive() && !isDragActive() && canStartPan()) {
            // Update startCoord_ to ensure smooth start for panning when going over startDragDistance
            m_declarativeMap->setKeepMouseGrab(true);
            setFlickState(panActive);
        }
        break;
    case panActive:
        if (m_allPoints.count() == 0) {
            setFlickState(tryStartFlick() ? flickTried : flickInactive);
            if (m_pinchState == pinchInactive && m_rotationState == rotationInactive && m_tiltState == tiltInactive)
                m_declarativeMap->setKeepMouseGrab(m_preventStealing);
            emit panFinished();
        }
        break;
    case flickTried:
        if (m_allPoints.count() > 0) { // re touched before movement ended
            m_declarativeMap->setKeepMouseGrab(true);
            setFlickState(panActive);
        }
        break;
    }

    if (m_flickState != lastState)
        emit panActiveChanged();

    // Update
    switch (m_flickState) {
    case flickInactive: // do nothing
        break;
    case panActive:
        updatePan();
        // this ensures 'panStarted' occurs after the pan has actually started
        if (lastState != panActive)
            emit panStarted();
        break;
    case flickTried:
        break;
    }
}

bool QTangramGestureArea::canStartPan()
{
    if (m_allPoints.count() == 0 || (m_acceptedGestures & PanGesture) == 0)
        return false;

    // Check if thresholds for normal panning are met.
    // (normal panning vs flicking: flicking will start from mouse release event).
    const int startDragDistance = qApp->styleHints()->startDragDistance() * 2;
    QPointF p1 = mapFromScene(m_allPoints.at(0).scenePos());
    int dyFromPress = int(p1.y() - m_sceneStartPoint1.y());
    int dxFromPress = int(p1.x() - m_sceneStartPoint1.x());
    if ((qAbs(dyFromPress) >= startDragDistance || qAbs(dxFromPress) >= startDragDistance))
        return true;
    return false;
}

void QTangramGestureArea::updatePan()
{
    m_map->tangramObject()->handlePanGesture(m_touchPointsCentroidLast.x(), m_touchPointsCentroidLast.y(),
                                             m_touchPointsCentroid.x(), m_touchPointsCentroid.y());
    m_touchPointsCentroidLast = m_touchPointsCentroid;
}

bool QTangramGestureArea::tryStartFlick()
{
    if ((m_acceptedGestures & FlickGesture) == 0)
        return false;

    m_map->tangramObject()->handleFlingGesture(m_lastPos.x(), m_lastPos.y(),
                                               m_flickVector.x(), m_flickVector.y());
    return true;
}

void QTangramGestureArea::stopPan()
{
    if (m_flickState == panActive) {
        m_flickVector = QVector2D();
        setFlickState(flickInactive);
        m_declarativeMap->setKeepMouseGrab(m_preventStealing);
        emit panFinished();
        emit panActiveChanged();
    }
}

void QTangramGestureArea::dragStateMachine()
{
    switch (m_dragState) {
    case dragInactive:
        if (m_enabled && m_touchPointState == touchPoints1) {
            auto pos = m_allPoints.first().pos();
            m_map->tangramObject()->pickMarkerAt(pos.x(), pos.y(),
                                                   std::bind(&QTangramGestureArea::onDragFeatures,
                                                             this, std::placeholders::_1));
        }
        break;
    case dragReady:
        if (m_allPoints.count() != 1) {
            m_declarativeMap->setKeepMouseGrab(m_preventStealing);
            setDragState(dragInactive);
        } else if (canStartDrag()) {
            if (isPanActive())
                stopPan();
            m_declarativeMap->setKeepMouseGrab(true);
            setDragState(dragActive);
            startDrag();
        }
        break;
    case dragActive:
        if (m_allPoints.count() != 1) { // Once started, pinch goes off only when finger(s) are release
            setDragState(dragInactive);
            m_declarativeMap->setKeepMouseGrab(m_preventStealing);
            endDrag();
        }
        break;
    }

    // Update
    switch (m_dragState) {
    case dragInactive:
    case dragReady:
        break; // do nothing
    case dragActive:
        updateDrag();
        break;
    }
}

bool QTangramGestureArea::canStartDrag()
{
    return m_allPoints.count() == 1 && canStartPan();
}

void QTangramGestureArea::startDrag()
{
    m_drag.m_id = m_drag.m_item->markerId();
    QGeoCoordinate itemCoord = m_drag.m_item->coordinate();
    QGeoCoordinate curCoord = m_map->itemPositionToCoordinate(m_sceneStartPoint1);
    m_drag.m_latitudeDistance = itemCoord.latitude() - curCoord.latitude();
    m_drag.m_longitudeDistance = itemCoord.longitude() - curCoord.longitude();
}

void QTangramGestureArea::updateDrag()
{
    QGeoCoordinate curPos = m_map->itemPositionToCoordinate(m_allPoints.first().pos());
    Tangram::LngLat lngLat(curPos.longitude() + m_drag.m_longitudeDistance,
                           curPos.latitude() + m_drag.m_latitudeDistance);
    m_map->tangramObject()->markerSetPoint(m_drag.m_id, lngLat);
}

void QTangramGestureArea::endDrag()
{
    QGeoCoordinate coordinate = m_map->itemPositionToCoordinate(m_lastPos);
    coordinate.setLatitude(coordinate.latitude() + m_drag.m_latitudeDistance);
    coordinate.setLongitude(coordinate.longitude() + m_drag.m_longitudeDistance);
    m_drag.m_item->setCoordinate(coordinate);
    emit m_drag.m_item->dragged();
    m_drag.m_item = 0;
    m_drag.m_id = 0;
    m_drag.m_latitudeDistance = 0;
    m_drag.m_longitudeDistance = 0;
}

void QTangramGestureArea::onDragFeatures(const Tangram::MarkerPickResult *result)
{
    if (!result || m_touchPointState != touchPoints1)
        return;

    int id = result->id;
    for (auto &marker : m_map->m_draggableItems) {
        if (id == marker->markerId()) {
            m_drag.m_item = marker;
            setDragState(dragReady);
            break;
        }
    }
}

void QTangramGestureArea::onClickedFeatures(const Tangram::MarkerPickResult *result)
{
    if (!result)
        return;

    int id = result->id;
    for (auto &marker : m_declarativeMap->m_map->m_clickableItems) {
        if (id == marker->markerId()) {
            emit marker->clicked(m_map->itemPositionToCoordinate(m_sceneStartPoint1));
            break;
        }
    }
}

void QTangramGestureArea::tryToClick()
{
    if (!isPanActive() || !isDragActive()) {
        m_map->tangramObject()->pickMarkerAt(m_lastPos.x(), m_lastPos.y(),
                                             std::bind(&QTangramGestureArea::onClickedFeatures,
                                                       this, std::placeholders::_1));
    }
}
