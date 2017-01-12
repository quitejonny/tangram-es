#ifndef QTANGRAMGESTUREAREA_H
#define QTANGRAMGESTUREAREA_H

#include <QtQuick/QQuickItem>
#include <QTouchEvent>
#include <QDebug>
#include <QElapsedTimer>
#include <QGeoCoordinate>
#include "tangram.h"

class QDeclarativeTangramMap;
class QTouchEvent;
class QWheelEvent;
class QTangramMap;
class QTangramPoint;

class QTangramMapPinchEvent : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QPointF center READ center)
    Q_PROPERTY(qreal angle READ angle)
    Q_PROPERTY(QPointF point1 READ point1)
    Q_PROPERTY(QPointF point2 READ point2)
    Q_PROPERTY(int pointCount READ pointCount)
    Q_PROPERTY(bool accepted READ accepted WRITE setAccepted)

public:
    QTangramMapPinchEvent(const QPointF &center, qreal angle,
                                 const QPointF &point1, const QPointF &point2,
                                 int pointCount = 0, bool accepted = true)
        : QObject(), m_center(center), m_angle(angle),
          m_point1(point1), m_point2(point2),
        m_pointCount(pointCount), m_accepted(accepted) {}
    QTangramMapPinchEvent()
        : QObject(),
          m_angle(0.0),
          m_pointCount(0),
          m_accepted(true) {}

    QPointF center() const { return m_center; }
    void setCenter(const QPointF &center) { m_center = center; }
    qreal angle() const { return m_angle; }
    void setAngle(qreal angle) { m_angle = angle; }
    QPointF point1() const { return m_point1; }
    void setPoint1(const QPointF &p) { m_point1 = p; }
    QPointF point2() const { return m_point2; }
    void setPoint2(const QPointF &p) { m_point2 = p; }
    int pointCount() const { return m_pointCount; }
    void setPointCount(int count) { m_pointCount = count; }
    bool accepted() const { return m_accepted; }
    void setAccepted(bool a) { m_accepted = a; }

private:
    QPointF m_center;
    qreal m_angle;
    QPointF m_point1;
    QPointF m_point2;
    int m_pointCount;
    bool m_accepted;
};

class QTangramGestureArea: public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(TangramMapGesture)
    Q_FLAGS(AcceptedGestures)

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool pinchActive READ isPinchActive NOTIFY pinchActiveChanged)
    Q_PROPERTY(bool panActive READ isPanActive NOTIFY panActiveChanged)
    Q_PROPERTY(AcceptedGestures acceptedGestures READ acceptedGestures WRITE setAcceptedGestures NOTIFY acceptedGesturesChanged)
    Q_PROPERTY(qreal maximumZoomLevelChange READ maximumZoomLevelChange WRITE setMaximumZoomLevelChange NOTIFY maximumZoomLevelChangeChanged)
    Q_PROPERTY(bool preventStealing READ preventStealing WRITE setPreventStealing NOTIFY preventStealingChanged REVISION 1)

public:
    QTangramGestureArea(QDeclarativeTangramMap *map);
    ~QTangramGestureArea();

    enum TangramMapGesture {
        NoGesture = 0x0000,
        PinchGesture = 0x0001,
        PanGesture = 0x0002,
        FlickGesture = 0x004
    };

    Q_DECLARE_FLAGS(AcceptedGestures, TangramMapGesture)

    AcceptedGestures acceptedGestures() const;
    void setAcceptedGestures(AcceptedGestures acceptedGestures);

    bool isPinchActive() const;
    bool isPanActive() const;
    bool isDragActive() const;
    bool isActive() const;

    bool enabled() const;
    void setEnabled(bool enabled);

    qreal maximumZoomLevelChange() const;
    void setMaximumZoomLevelChange(qreal maxChange);

    void handleTouchEvent(QTouchEvent *event);
    void handleWheelEvent(QWheelEvent *event);
    void handleMousePressEvent(QMouseEvent *event);
    void handleMouseMoveEvent(QMouseEvent *event);
    void handleMouseReleaseEvent(QMouseEvent *event);
    void handleMouseUngrabEvent();
    void handleTouchUngrabEvent();

    void onClickedFeatures(const Tangram::MarkerPickResult *result);
    void onDragFeatures(const Tangram::MarkerPickResult *result);

    void setMinimumZoomLevel(qreal min);
    qreal minimumZoomLevel() const;

    void setMaximumZoomLevel(qreal max);
    qreal maximumZoomLevel() const;

    void setMap(QTangramMap *map);

    bool preventStealing() const;
    void setPreventStealing(bool prevent);

Q_SIGNALS:
    void panActiveChanged();
    void pinchActiveChanged();
    void enabledChanged();
    void maximumZoomLevelChangeChanged();
    void acceptedGesturesChanged();
    void pinchStarted(QTangramMapPinchEvent *pinch);
    void pinchUpdated(QTangramMapPinchEvent *pinch);
    void pinchFinished(QTangramMapPinchEvent *pinch);
    void panStarted();
    void panFinished();
    void flickStarted();
    void flickFinished();
    void preventStealingChanged();

    void clicked();
private:
    void update();

    // Create general data relating to the touch points
    void touchPointStateMachine();
    void startOneTouchPoint();
    void updateOneTouchPoint();
    void startTwoTouchPoints();
    void updateTwoTouchPoints();

    void actionStateMachine();

    // All pinch related code, which encompasses zoom
    void pinchStateMachine();
    bool canStartPinch();
    void startPinch();
    void updatePinch();
    void endPinch();

    // Pan related code (regardles of number of touch points),
    // includes the flick based panning after letting go
    void panStateMachine();
    bool canStartPan();
    void startPan();
    void updatePan();
    void tryStartFlick();

    // Drag related code
    void startDrag();
    void updateDrag();
    void endDrag();

    bool pinchEnabled() const;
    void setPinchEnabled(bool enabled);
    bool panEnabled() const;
    void setPanEnabled(bool enabled);
    bool flickEnabled() const;
    void setFlickEnabled(bool enabled);


private:
    void stopPan();
    void clearTouchData();
    void updateVelocityList(const QPointF &pos);

private:
    QTangramMap* m_map;
    QDeclarativeTangramMap *m_declarativeMap;
    bool m_enabled;

    struct Drag
    {
        Drag() : m_id(0), m_latitudeDistance(0), m_longitudeDistance(0) {}
        QTangramPoint* m_item;
        int m_id;
        qreal m_latitudeDistance;
        qreal m_longitudeDistance;

    } m_drag;

    struct Pinch
    {
        Pinch() : m_enabled(true), m_startDist(0), m_lastAngle(0.0), m_lastDistance(0.0) {}

        QTangramMapPinchEvent m_event;
        bool m_enabled;
        struct Zoom
        {
            Zoom() : m_minimum(1.0), m_maximum(20), m_start(0.0), m_previous(0.0),
                     maximumChange(4.0) {}
            qreal m_minimum;
            qreal m_maximum;
            qreal m_start;
            qreal m_previous;
            qreal maximumChange;
        } m_zoom;

        QPointF m_lastPoint1;
        QPointF m_lastPoint2;
        qreal m_startDist;
        qreal m_lastAngle;
        qreal m_lastDistance;
     } m_pinch;

    AcceptedGestures m_acceptedGestures;

    struct Pan
    {
        qreal m_maxVelocity;
        bool m_enabled;
    } m_flick;


    // these are calculated regardless of gesture or number of touch points
    qreal m_velocityX;
    qreal m_velocityY;
    QElapsedTimer m_lastPosTime;
    QPointF m_lastPos;
    QList<QTouchEvent::TouchPoint> m_allPoints;
    QList<QTouchEvent::TouchPoint> m_touchPoints;
    QScopedPointer<QTouchEvent::TouchPoint> m_mousePoint;
    QPointF m_sceneStartPoint1;

    // only set when two points in contact
    QPointF m_sceneStartPoint2;
    QGeoCoordinate m_startCoord;
    QGeoCoordinate m_touchCenterCoord;
    qreal m_twoTouchAngle;
    qreal m_distanceBetweenTouchPoints;
    qreal m_velocityBetweenTouchPoints;
    QPointF m_sceneCenter;
    QPointF m_sceneCenterLast;
    bool m_preventStealing;
    bool m_panEnabled;

    // prototype state machine...
    enum TouchPointState
    {
        touchPoints0,
        touchPoints1,
        touchPoints2
    } m_touchPointState;

    enum PinchState
    {
        pinchInactive,
        pinchInactiveTwoPoints,
        pinchActive
    } m_pinchState;

    enum ActionState
    {
        actionInactive,
        actionDown,
        actionDownItem,
        actionPan,
        actionDrag,
        actionClick,
        actionFlick
    } m_actionState;
};

QML_DECLARE_TYPE(QTangramGestureArea)

#endif // QTANGRAMGESTUREAREA_H
