#include <QImage>
#include "qtangrammarkermanager.h"
#include "qtangramgeometry.h"
#include "qtangrampoint.h"
#include "qtangrampolyline.h"
#include "map.h"
#include <QDebug>

QTangramMarkerManager::QTangramMarkerManager(Tangram::Map *map, QObject *parent)
    : QObject(parent),
      m_map(map)
{
}

void QTangramMarkerManager::sync(QTangramGeometry *marker)
{
    auto syncState  = marker->m_syncState;
    marker->m_syncState = QTangramGeometry::NothingNeedsSync;

    if (syncState & QTangramGeometry::MarkerIdNeedsSync) {
        if (marker->m_markerId != -1)
            remove(marker->m_markerId);

        if (marker->m_map) {
            // add marker to map
            marker->m_markerId = m_map->markerAdd();
            m_markers.insert(marker->m_markerId, marker);
            qDebug() << Q_FUNC_INFO << "ADD_MARKER" << marker->m_markerId;
        } else {
            marker->m_markerId = -1;
            return;
        }
    }

    if (syncState & QTangramGeometry::StylingNeedsSync) {
        m_map->markerSetStylingFromString(marker->m_markerId,
                                          marker->m_allStylings.toStdString().c_str());
    }

    if (syncState & QTangramGeometry::BitmapNeedsSync) {
        setBitmap(marker);
    }

    if (syncState & QTangramGeometry::PointNeedsSync) {
        auto point = static_cast<QTangramPoint *>(marker);
        auto lngLat = Tangram::LngLat(point->m_coordinate.longitude(),
                                      point->m_coordinate.latitude());
        m_map->markerSetPoint(point->m_markerId, lngLat);
    }

    if (syncState & QTangramGeometry::PolylineNeedsSync) {
        auto polyline = static_cast<QTangramPolyline *>(marker);
        std::vector<Tangram::LngLat> coord;
        for (int i = 0; i < polyline->m_path.size(); ++i)
            coord.push_back(Tangram::LngLat(polyline->m_path.at(i).longitude(),
                                            polyline->m_path.at(i).latitude()));
        m_map->markerSetPolyline(polyline->m_markerId, coord.data(), coord.size());
    }

    if (syncState & QTangramGeometry::PolygonNeedsSync) {
    }

    if (syncState & QTangramGeometry::VisibleNeedsSync) {
        m_map->markerSetVisible(marker->m_markerId, marker->m_visible);
    }

    if (syncState & QTangramGeometry::DrawOrderNeedsSync) {
        m_map->markerSetDrawOrder(marker->m_markerId, marker->m_drawOrder);
    }

    if (syncState & QTangramGeometry::ClickableNeedsSync) {
        if (marker->m_clickable)
            m_clickableMarkers.insert(marker->markerId());
        else
            m_clickableMarkers.remove(marker->markerId());
    }

    if (syncState & QTangramGeometry::DraggableNeedsSync) {
        auto point = static_cast<QTangramPoint *>(marker);
        if (point->m_draggable)
            m_draggableMarkers.insert(point->markerId());
        else
            m_draggableMarkers.remove(point->markerId());
    }
}

void QTangramMarkerManager::setBitmap(QTangramGeometry *marker)
{
    auto point = static_cast<QTangramPoint *>(marker);
    auto imageSource = point->m_imageSource;
    QString source = imageSource.startsWith("qrc:") ? imageSource.remove(0, 3) : imageSource;
    if (source.isEmpty())
        return;

    QImage image(source);
    if (image.isNull())
        return;

    image = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    const uint *data = reinterpret_cast<const uint*>(image.constBits());
    m_map->markerSetBitmap(point->m_markerId, image.width(), image.height(), data);
}

void QTangramMarkerManager::remove(int markerId)
{
    m_markers.remove(markerId);
    m_clickableMarkers.remove(markerId);
    m_draggableMarkers.remove(markerId);
    m_map->markerRemove(markerId);
}

void QTangramMarkerManager::syncDrag()
{
    if (m_drag.position.isValid() && m_markers.contains(m_drag.m_id)) {
        auto point = static_cast<QTangramPoint *>(m_markers.value(m_drag.m_id));
        point->setCoordinate(m_drag.position);
    }
}

void QTangramMarkerManager::tryToClick(qreal posX, qreal posY)
{
    if (m_clickableMarkers.empty())
        return;

    m_map->pickMarkerAt(posX, posY, [&](const Tangram::MarkerPickResult *result) {
        if (result && m_clickableMarkers.contains(result->id)) {
            double lat, lon;
            m_map->screenPositionToLngLat(result->position[0], result->position[1], &lon, &lat);
            QGeoCoordinate coordinate(lat, lon);
            emit m_markers.value(result->id)->clicked(coordinate);
        }
    });
}

void QTangramMarkerManager::tryToDrag(qreal posX, qreal posY)
{
    if (m_draggableMarkers.empty())
        return;

    m_map->pickMarkerAt(posX, posY, [&](const Tangram::MarkerPickResult *result) {
        if (result && m_draggableMarkers.contains(result->id)) {
            m_drag.m_id = result->id;
            m_map->lngLatToScreenPosition(result->coordinates.longitude,
                                          result->coordinates.latitude,
                                          &m_drag.m_deltaX, &m_drag.m_deltaY);
            m_drag.m_deltaX -= result->position[0];
            m_drag.m_deltaY -= result->position[1];
            emit startDrag();
        }
    });
}

void QTangramMarkerManager::drag(qreal posX, qreal posY)
{
    Tangram::LngLat lngLat(0, 0);
    m_map->screenPositionToLngLat(m_drag.m_deltaX + posX, m_drag.m_deltaY + posY,
                                  &lngLat.longitude, &lngLat.latitude);
    //m_map->markerSetPoint(m_drag.m_id, lngLat);
    m_drag.position.setLongitude(lngLat.longitude);
    m_drag.position.setLatitude(lngLat.latitude);
}

void QTangramMarkerManager::endDrag()
{
    if (m_draggableMarkers.contains(m_drag.m_id)) {
        auto point = static_cast<QTangramPoint *>(m_markers.value(m_drag.m_id));
        m_drag.m_id = 0;
        m_drag.position = QGeoCoordinate();
        emit point->dragged();
    }
}
