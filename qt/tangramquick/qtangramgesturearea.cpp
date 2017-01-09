#include "qtangramgesturearea.h"
#include "tangramquick.h"
#include "qtangrammap.h"
#include "qtangrammapcontroller.h"
#include "tangram.h"
#include "qtangramgeometry.h"
#include "qtangrampoint.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QWheelEvent>
#include <QtGui/QStyleHints>
#include <QtQuick/QQuickWindow>
#include <QDebug>

#define QML_MAP_FLICK_DEFAULTMAXVELOCITY 2500

#define QML_MAP_FLICK_VELOCITY_SAMPLE_PERIOD 50


QTangramGestureArea::QTangramGestureArea(QDeclarativeTangramMap *map)
    : QQuickItem(map),
      m_map(0),
      m_declarativeMap(map),
      m_enabled(true),
      m_acceptedGestures(PinchGesture | PanGesture | FlickGesture),
      m_preventStealing(false),
      m_panEnabled(true)
{
    setMap(m_declarativeMap->m_map);
    m_flick.m_enabled = true,
    m_flick.m_maxVelocity = QML_MAP_FLICK_DEFAULTMAXVELOCITY;
    m_touchPointState = touchPoints0;
    m_pinchState = pinchInactive;
    m_actionState = actionInactive;
}

void QTangramGestureArea::setMap(QTangramMap *map)
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

    setPanEnabled(acceptedGestures & PanGesture);
    setFlickEnabled(acceptedGestures & FlickGesture);
    setPinchEnabled(acceptedGestures & PinchGesture);

    emit acceptedGesturesChanged();
}

bool QTangramGestureArea::isPinchActive() const
{
    return m_pinchState == pinchActive;
}

bool QTangramGestureArea::isPanActive() const
{
    return m_actionState == actionPan;
}

bool QTangramGestureArea::isDragActive() const
{
    return m_actionState == actionDrag;
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
    } else {
        setPanEnabled(false);
        setFlickEnabled(false);
        setPinchEnabled(false);
    }

    emit enabledChanged();
}


bool QTangramGestureArea::pinchEnabled() const
{
    return m_pinch.m_enabled;
}

void QTangramGestureArea::setPinchEnabled(bool enabled)
{
    if (enabled == m_pinch.m_enabled)
        return;
    m_pinch.m_enabled = enabled;
}

bool QTangramGestureArea::panEnabled() const
{
    return m_panEnabled;
}

void QTangramGestureArea::setPanEnabled(bool enabled)
{
    if (enabled == m_flick.m_enabled)
        return;
    m_panEnabled = enabled;

    // unlike the pinch, the pan existing functionality is to stop immediately
    if (!enabled)
        stopPan();
}

bool QTangramGestureArea::flickEnabled() const
{
    return m_flick.m_enabled;
}

void QTangramGestureArea::setFlickEnabled(bool enabled)
{
    if (enabled == m_flick.m_enabled)
        return;
    m_flick.m_enabled = enabled;
}

void QTangramGestureArea::setMinimumZoomLevel(qreal min)
{
    if (min >= 0)
        m_pinch.m_zoom.m_minimum = min;
}

qreal QTangramGestureArea::minimumZoomLevel() const
{
    return m_pinch.m_zoom.m_minimum;
}

void QTangramGestureArea::setMaximumZoomLevel(qreal max)
{
    if (max >= 0)
        m_pinch.m_zoom.m_maximum = max;
}

qreal QTangramGestureArea::maximumZoomLevel() const
{
    return m_pinch.m_zoom.m_maximum;
}

qreal QTangramGestureArea::maximumZoomLevelChange() const
{
    return m_pinch.m_zoom.maximumChange;
}

void QTangramGestureArea::setMaximumZoomLevelChange(qreal maxChange)
{
    if (maxChange == m_pinch.m_zoom.maximumChange || maxChange < 0.1 || maxChange > 10.0)
        return;
    m_pinch.m_zoom.maximumChange = maxChange;
    emit maximumZoomLevelChangeChanged();
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
        m_mousePoint.reset();
        if (m_touchPoints.isEmpty()) update();
    }
    event->accept();
}

void QTangramGestureArea::handleMouseUngrabEvent()
{
    m_mousePoint.reset();
    if (m_touchPoints.isEmpty() && !m_mousePoint.isNull()) {
        update();
    }
}

void QTangramGestureArea::handleTouchUngrabEvent()
{
    m_touchPoints.clear();
    //this is needed since in some cases mouse release is not delivered
    //(second touch point brakes mouse synthesized events)
    m_mousePoint.reset();
    update();
}

void QTangramGestureArea::handleTouchEvent(QTouchEvent *event)
{
    m_touchPoints.clear();
    m_mousePoint.reset();
    for (auto &touchPoint : event->touchPoints())
        if ( touchPoint.state() != Qt::TouchPointReleased)
            m_touchPoints << touchPoint;
    if (m_touchPoints.count() >= 2) {
        event->accept();
    } else {
        event->ignore();
    }
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
    m_velocityX = 0;
    m_velocityY = 0;
    m_sceneCenter.setX(0);
    m_sceneCenter.setY(0);
    m_touchCenterCoord.setLongitude(0);
    m_touchCenterCoord.setLatitude(0);
    m_startCoord.setLongitude(0);
    m_startCoord.setLatitude(0);
}


void QTangramGestureArea::updateVelocityList(const QPointF &pos)
{
    // Take velocity samples every sufficient period of time, used later to determine the flick
    // duration and speed (when mouse is released).
    qreal elapsed = qreal(m_lastPosTime.elapsed());

    if (elapsed >= QML_MAP_FLICK_VELOCITY_SAMPLE_PERIOD) {
        elapsed /= 1000.;
        int dyFromLastPos = pos.y() - m_lastPos.y();
        int dxFromLastPos = pos.x() - m_lastPos.x();
        m_lastPos = pos;
        m_lastPosTime.restart();
        m_velocityX = qreal(dxFromLastPos) / elapsed;
        m_velocityY = qreal(dyFromLastPos) / elapsed;
    }
}


bool QTangramGestureArea::isActive() const
{
    return isPanActive() || isPinchActive();
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
    m_sceneCenterLast = m_sceneCenter;

    touchPointStateMachine();

    // Parallel state machine for pinch
    if (isPinchActive() || (m_enabled && m_pinch.m_enabled && (m_acceptedGestures & (PinchGesture))))
        pinchStateMachine();

    // Parallel state machine for pan (since you can pan at the same time as pinching)
    // The stopPan function ensures that pan stops immediately when disabled,
    // but the line below allows pan continue its current gesture if you disable
    // the whole gesture (enabled_ flag), this keeps the enabled_ consistent with the pinch
    actionStateMachine();
}

void QTangramGestureArea::onDragFeatures(const Tangram::FeaturePickResult *result)
{
    if (!result || m_touchPointState != touchPoints1)
        return;

    int id = result->properties->getNumber("id");
    auto kind = result->properties->getString("kind");
    if (kind == "dynamicMarker") {
        for (auto &marker : m_map->m_draggableItems) {
            if (id == marker->markerId()) {
                m_drag.m_item = marker;
                m_actionState = actionDownItem;
                break;
            }
        }
    }
}

void QTangramGestureArea::onClickedFeatures(const Tangram::FeaturePickResult *result)
{
    if (!result)
        return;

    int id = result->properties->getNumber("id");
    auto kind = result->properties->getString("kind");
    if (kind == "dynamicMarker") {
        for (auto &marker : m_declarativeMap->m_map->m_clickableItems) {
            if (id == marker->markerId()) {
                emit marker->clicked();
                break;
            }
        }
    }
}

void QTangramGestureArea::touchPointStateMachine()
{
    // Transitions:
    switch (m_touchPointState) {
    case touchPoints0:
        if (m_allPoints.count() == 1) {
            clearTouchData();
            startOneTouchPoint();
            m_touchPointState = touchPoints1;
        } else if (m_allPoints.count() >= 2) {
            clearTouchData();
            startTwoTouchPoints();
            m_touchPointState = touchPoints2;
        }
        break;
    case touchPoints1:
        if (m_allPoints.count() == 0) {
            m_touchPointState = touchPoints0;
        } else if (m_allPoints.count() == 2) {
            m_touchCenterCoord = m_map->itemPositionToCoordinate(m_sceneCenter);
            startTwoTouchPoints();
            m_touchPointState = touchPoints2;
        }
        break;
    case touchPoints2:
        if (m_allPoints.count() == 0) {
            m_touchPointState = touchPoints0;
        } else if (m_allPoints.count() == 1) {
            m_touchCenterCoord = m_map->itemPositionToCoordinate(m_sceneCenter);
            startOneTouchPoint();
            m_touchPointState = touchPoints1;
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
    m_sceneStartPoint1 = mapFromScene(m_allPoints.first().scenePos());
    m_lastPos = m_sceneStartPoint1;
    m_sceneCenterLast = m_lastPos;
    m_lastPosTime.start();
    QGeoCoordinate startCoord = m_map->itemPositionToCoordinate(m_sceneStartPoint1);
    // ensures a smooth transition for panning
    m_startCoord.setLongitude(m_startCoord.longitude() + startCoord.longitude() -
                             m_touchCenterCoord.longitude());
    m_startCoord.setLatitude(m_startCoord.latitude() + startCoord.latitude() -
                            m_touchCenterCoord.latitude());
}

void QTangramGestureArea::updateOneTouchPoint()
{
    m_sceneCenter = mapFromScene(m_allPoints.first().scenePos());
    updateVelocityList(m_sceneCenter);
}


void QTangramGestureArea::startTwoTouchPoints()
{
    m_sceneStartPoint1 = mapFromScene(m_allPoints.at(0).scenePos());
    m_sceneStartPoint2 = mapFromScene(m_allPoints.at(1).scenePos());
    QPointF startPos = (m_sceneStartPoint1 + m_sceneStartPoint2) * 0.5;
    m_lastPos = startPos;
    m_sceneCenterLast = m_lastPos;
    m_lastPosTime.start();
    QGeoCoordinate startCoord = m_map->itemPositionToCoordinate(startPos);
    m_startCoord.setLongitude(m_startCoord.longitude() + startCoord.longitude() -
                             m_touchCenterCoord.longitude());
    m_startCoord.setLatitude(m_startCoord.latitude() + startCoord.latitude() -
                            m_touchCenterCoord.latitude());
}

void QTangramGestureArea::updateTwoTouchPoints()
{
    QPointF p1 = mapFromScene(m_allPoints.at(0).scenePos());
    QPointF p2 = mapFromScene(m_allPoints.at(1).scenePos());
    qreal dx = p1.x() - p2.x();
    qreal dy = p1.y() - p2.y();
    m_distanceBetweenTouchPoints = sqrt(dx * dx + dy * dy);
    m_velocityBetweenTouchPoints = m_distanceBetweenTouchPoints/qreal(m_lastPosTime.elapsed());
    m_sceneCenter = (p1 + p2) / 2;
    updateVelocityList(m_sceneCenter);

    m_twoTouchAngle = QLineF(p1, p2).angle();
    if (m_twoTouchAngle > 180)
        m_twoTouchAngle -= 360;
}

void QTangramGestureArea::startDrag()
{
    m_actionState = actionDrag;
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
    m_actionState = actionInactive;
    QGeoCoordinate coordinate = m_map->itemPositionToCoordinate(m_lastPos);
    coordinate.setLatitude(coordinate.latitude() + m_drag.m_latitudeDistance);
    coordinate.setLongitude(coordinate.longitude() + m_drag.m_longitudeDistance);
    m_drag.m_item->setCoordinate(coordinate);
    m_drag.m_item = 0;
    m_drag.m_id = 0;
    m_drag.m_latitudeDistance = 0;
    m_drag.m_longitudeDistance = 0;
}

void QTangramGestureArea::pinchStateMachine()
{
    PinchState lastState = m_pinchState;
    // Transitions:
    switch (m_pinchState) {
    case pinchInactive:
        if (m_allPoints.count() >= 2) {
            if (canStartPinch()) {
                m_declarativeMap->setKeepMouseGrab(true);
                m_declarativeMap->setKeepTouchGrab(true);
                startPinch();
                m_pinchState = pinchActive;
            } else {
                m_pinchState = pinchInactiveTwoPoints;
            }
        }
        break;
    case pinchInactiveTwoPoints:
        if (m_allPoints.count() <= 1) {
            m_pinchState = pinchInactive;
        } else {
            if (canStartPinch()) {
                m_declarativeMap->setKeepMouseGrab(true);
                m_declarativeMap->setKeepTouchGrab(true);
                startPinch();
                m_pinchState = pinchActive;
            }
        }
        break;
    case pinchActive:
        if (m_allPoints.count() <= 1) {
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
    const int startDragDistance = qApp->styleHints()->startDragDistance();

    if (m_allPoints.count() >= 2) {
        QPointF p1 = mapFromScene(m_allPoints.at(0).scenePos());
        QPointF p2 = mapFromScene(m_allPoints.at(1).scenePos());
        if (qAbs(p1.x()-m_sceneStartPoint1.x()) > startDragDistance
         || qAbs(p1.y()-m_sceneStartPoint1.y()) > startDragDistance
         || qAbs(p2.x()-m_sceneStartPoint2.x()) > startDragDistance
         || qAbs(p2.y()-m_sceneStartPoint2.y()) > startDragDistance) {
            m_pinch.m_event.setCenter(mapFromScene(m_sceneCenter));
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
    m_pinch.m_lastDistance = m_distanceBetweenTouchPoints;

    m_pinch.m_lastPoint1 = mapFromScene(m_allPoints.at(0).scenePos());
    m_pinch.m_lastPoint2 = mapFromScene(m_allPoints.at(1).scenePos());

    m_pinch.m_zoom.m_start = m_map->mapController()->zoom();
}

void QTangramGestureArea::updatePinch()
{
    // Calculate the new zoom level if we have distance ( >= 2 touchpoints), otherwise stick with old.
    //qreal newZoomLevel = m_pinch.m_zoom.m_previous;
    qreal scale = 1 + 0.005*(m_distanceBetweenTouchPoints - m_pinch.m_lastDistance);
    m_map->tangramObject()->handlePinchGesture(m_sceneCenter.x(), m_sceneCenter.y(), scale, 0.f);
    m_pinch.m_lastAngle = m_twoTouchAngle;
    m_pinch.m_lastDistance = m_distanceBetweenTouchPoints;
    m_pinch.m_zoom.m_previous = m_map->mapController()->zoom();
    emit m_map->mapController()->zoomChanged(m_pinch.m_zoom.m_previous);
    m_pinch.m_lastPoint1 = mapFromScene(m_allPoints.at(0).scenePos());
    m_pinch.m_lastPoint2 = mapFromScene(m_allPoints.at(1).scenePos());
}

void QTangramGestureArea::endPinch()
{
    m_pinchState = pinchInactive;
    m_declarativeMap->setKeepMouseGrab(m_preventStealing);
    m_declarativeMap->setKeepTouchGrab(m_preventStealing);
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

void QTangramGestureArea::actionStateMachine()
{
    switch (m_actionState) {
    case actionInactive:
        if (m_enabled && m_touchPointState == touchPoints1) {
            m_actionState = actionDown;
            auto pos = m_allPoints.first().pos();
            m_map->tangramObject()->pickFeatureAt(pos.x(), pos.y(),
                                                   std::bind(&QTangramGestureArea::onDragFeatures,
                                                             this, std::placeholders::_1));
        }
        break;
    case actionDown:
        if (m_allPoints.isEmpty())
            m_actionState = actionClick;
        else if (m_flick.m_enabled && (m_acceptedGestures & (PanGesture | FlickGesture))
                 && canStartPan())
            startPan();
        break;
    case actionDownItem:
        if (m_allPoints.isEmpty())
            m_actionState = actionClick;
        else if (m_allPoints.count() == 1 && canStartPan())
            startDrag();
        break;
    case actionPan:
        if (m_allPoints.isEmpty()) {
            tryStartFlick();
            stopPan();
        }
        break;
    case actionDrag:
        if (m_allPoints.isEmpty())
            endDrag();
        break;
    case actionClick:
        break;
    case actionFlick:
        break;
    }

    switch (m_actionState) {
    case actionPan:
        updatePan();
        break;
    case actionDrag:
        updateDrag();
        break;
    case actionClick:
        m_actionState = actionInactive;
        // mouse should have been released without paning or pinch. So this is a CLICK event!
        m_map->tangramObject()->pickFeatureAt(m_lastPos.x(), m_lastPos.y(),
                                               std::bind(&QTangramGestureArea::onClickedFeatures,
                                                         this, std::placeholders::_1));
        break;
    default:
        break;
    }
}

void QTangramGestureArea::startPan()
{
    // Update startCoord_ to ensure smooth start for panning when going over startDragDistance
    QGeoCoordinate newStartCoord = m_map->itemPositionToCoordinate(m_sceneCenter);
    m_startCoord.setLongitude(newStartCoord.longitude());
    m_startCoord.setLatitude(newStartCoord.latitude());
    m_declarativeMap->setKeepMouseGrab(true);
    m_actionState = actionPan;
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
    m_map->tangramObject()->handlePanGesture(m_sceneCenterLast.x(), m_sceneCenterLast.y(),
                                             m_sceneCenter.x(), m_sceneCenter.y());
}

void QTangramGestureArea::tryStartFlick()
{
    if ((m_acceptedGestures & FlickGesture) == 0)
        return;

    if (m_lastPosTime.elapsed() < QML_MAP_FLICK_VELOCITY_SAMPLE_PERIOD) {
        m_map->tangramObject()->handleFlingGesture(m_sceneCenter.x(), m_sceneCenter.y(), m_velocityX, m_velocityY);
    }
}

void QTangramGestureArea::stopPan()
{
    m_velocityX = 0;
    m_velocityY = 0;
    if (m_actionState == actionPan) {
        m_actionState = actionInactive;
        m_declarativeMap->setKeepMouseGrab(m_preventStealing);
        emit panFinished();
        emit panActiveChanged();
        //m_map->prefetchData();
    }
}
