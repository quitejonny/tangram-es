#ifndef QTANGRAMGESTUREAREA_H
#define QTANGRAMGESTUREAREA_H

#include <QtQuick/QQuickItem>
#include <QTouchEvent>
#include <QElapsedTimer>

#include <QtGui/QVector2D>
#include <QtCore/QPointer>

class QDeclarativeTangramMap;
class QWheelEvent;
class QTangramMap;
class QTangramPoint;

namespace Tangram {
struct MarkerPickResult;
}

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
    Q_PROPERTY(bool dragActive READ isDragActive NOTIFY dragActiveChanged)
    Q_PROPERTY(bool rotationActive READ isRotationActive NOTIFY rotationActiveChanged)
    Q_PROPERTY(bool tiltActive READ isTiltActive NOTIFY tiltActiveChanged)
    Q_PROPERTY(AcceptedGestures acceptedGestures READ acceptedGestures WRITE setAcceptedGestures NOTIFY acceptedGesturesChanged)
    Q_PROPERTY(bool preventStealing READ preventStealing WRITE setPreventStealing NOTIFY preventStealingChanged REVISION 1)

public:
    QTangramGestureArea(QDeclarativeTangramMap *map = NULL);
    ~QTangramGestureArea();

    enum TangramMapGesture {
        NoGesture = 0x0000,
        PinchGesture = 0x0001,
        PanGesture = 0x0002,
        FlickGesture = 0x0004,
        RotationGesture = 0x0008,
        TiltGesture = 0x0010
    };

    Q_DECLARE_FLAGS(AcceptedGestures, TangramMapGesture)

    AcceptedGestures acceptedGestures() const;
    void setAcceptedGestures(AcceptedGestures acceptedGestures);

    bool isPinchActive() const;
    bool isRotationActive() const;
    bool isTiltActive() const;
    bool isPanActive() const;
    bool isDragActive() const;
    bool isActive() const;

    bool enabled() const;
    void setEnabled(bool enabled);

    void handleTouchEvent(QTouchEvent *event);
    void handleWheelEvent(QWheelEvent *event);
    void handleMousePressEvent(QMouseEvent *event);
    void handleMouseMoveEvent(QMouseEvent *event);
    void handleMouseReleaseEvent(QMouseEvent *event);
    void handleMouseUngrabEvent();
    void handleTouchUngrabEvent();

    bool preventStealing() const;
    void setPreventStealing(bool prevent);

public slots:
    void startDrag();

Q_SIGNALS:
    void panActiveChanged();
    void dragActiveChanged();
    void pinchActiveChanged();
    void rotationActiveChanged();
    void tiltActiveChanged();
    void enabledChanged();
    void acceptedGesturesChanged();
    void pinchStarted(QTangramMapPinchEvent *pinch);
    void pinchUpdated(QTangramMapPinchEvent *pinch);
    void pinchFinished(QTangramMapPinchEvent *pinch);
    void panStarted();
    void panFinished();
    void dragStarted();
    void dragFinished();
    void rotationStarted(QTangramMapPinchEvent *pinch);
    void rotationUpdated(QTangramMapPinchEvent *pinch);
    void rotationFinished(QTangramMapPinchEvent *pinch);
    void tiltStarted(QTangramMapPinchEvent *pinch);
    void tiltUpdated(QTangramMapPinchEvent *pinch);
    void tiltFinished(QTangramMapPinchEvent *pinch);
    void preventStealingChanged();

private:
    void update();

    // Create general data relating to the touch points
    void touchPointStateMachine();
    void startOneTouchPoint();
    void updateOneTouchPoint();
    void startTwoTouchPoints();
    void updateTwoTouchPoints();

    // All two fingers vertical parallel panning related code, which encompasses tilting
    void tiltStateMachine();
    bool canStartTilt();
    void startTilt();
    void updateTilt();
    void endTilt();

    // All two fingers rotation related code, which encompasses rotation
    void rotationStateMachine();
    bool canStartRotation();
    void startRotation();
    void updateRotation();
    void endRotation();

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
    void updatePan();
    void tryStartFlick();

    void dragStateMachine();
    bool canStartDrag();

    bool pinchEnabled() const;
    void setPinchEnabled(bool enabled);
    bool rotationEnabled() const;
    void setRotationEnabled(bool enabled);
    bool tiltEnabled() const;
    void setTiltEnabled(bool enabled);
    bool panEnabled() const;
    void setPanEnabled(bool enabled);
    bool flickEnabled() const;
    void setFlickEnabled(bool enabled);

private:
    void stopPan();
    void clearTouchData();
    void updateFlickParameters(const QPointF &pos);

private:
    QPointer<QTangramMap> m_map;
    QDeclarativeTangramMap *m_declarativeMap;
    bool m_enabled;

    // This should be intended as a "two fingers gesture" struct
    struct Pinch
    {
        Pinch() : m_pinchEnabled(true), m_rotationEnabled(true), m_tiltEnabled(true),
                  m_startDist(0), m_lastAngle(0.0) {}

        QTangramMapPinchEvent m_event;
        bool m_pinchEnabled;
        bool m_rotationEnabled;
        bool m_tiltEnabled;

        struct Rotation
        {
            Rotation() : m_previousTouchAngle(0.0) {}
            qreal m_previousTouchAngle; // needed for detecting crossing +- 180 in a safer way
            qreal m_angle;
        } m_rotation;

        struct Tilt
        {
            Tilt() {}
            qreal m_verticalDisplacement;
        } m_tilt;

        QPointF m_lastPoint1;
        QPointF m_lastPoint2;
        qreal m_startDist;
        qreal m_lastAngle;
        qreal m_scale;
     } m_pinch;

    AcceptedGestures m_acceptedGestures;

    struct Pan
    {
        Pan() : m_flickEnabled(true), m_panEnabled(true) {}

        bool m_flickEnabled;
        bool m_panEnabled;
        QVector2D m_vector;
    } m_flick;

    // these are calculated regardless of gesture or number of touch points
    QElapsedTimer m_lastPosTime;
    QPointF m_lastPos;
    QVector<QTouchEvent::TouchPoint> m_allPoints;
    QVector<QTouchEvent::TouchPoint> m_touchPoints;
    QScopedPointer<QTouchEvent::TouchPoint> m_mousePoint;
    QPointF m_sceneStartPoint1;

    // only set when two points in contact
    QPointF m_sceneStartPoint2;
    qreal m_twoTouchAngle;
    qreal m_twoTouchAngleStart;
    qreal m_distanceBetweenTouchPoints;
    qreal m_distanceBetweenTouchPointsLast;
    qreal m_distanceBetweenTouchPointsStart;
    QPointF m_twoTouchPointsCentroidStart;
    QPointF m_touchPointsCentroidLast;
    QPointF m_touchPointsCentroid;
    bool m_preventStealing;
    bool m_panEnabled;

private:
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

    enum RotationState
    {
        rotationInactive,
        rotationInactiveTwoPoints,
        rotationActive
    } m_rotationState;

    enum TiltState
    {
        tiltInactive,
        tiltInactiveTwoPoints,
        tiltActive
    } m_tiltState;

    enum FlickState
    {
        flickInactive,
        panActive
    } m_flickState;

    enum DragState
    {
        dragInactive,
        dragReady,
        dragActive
    } m_dragState;

    enum SyncState {
        NothingNeedsSync = 0,
        PanNeedsSync = 1 << 0,
        FlingNeedsSync = 1 << 1,
        PinchNeedsSync = 1 << 2,
        RotateNeedsSync = 1 << 3,
        ShoveNeedsSync = 1 << 4,
        TryClickNeedsSync = 1 << 5,
        TryDragNeedsSync = 1 << 6,
        DragNeedsSync = 1 << 7,
        EndDragNeedsSync = 1 << 8,
        StopPointNeedsSync = 1 << 9
    };

    int m_syncState = NothingNeedsSync;

    inline void setTouchPointState(const TouchPointState state);
    inline void setFlickState(const FlickState state);
    inline void setTiltState(const TiltState state);
    inline void setRotationState(const RotationState state);
    inline void setPinchState(const PinchState state);
    inline void setDragState(const DragState state);

    friend class TangramQuickRenderer;
};

QML_DECLARE_TYPE(QTangramGestureArea)

#endif // QTANGRAMGESTUREAREA_H
