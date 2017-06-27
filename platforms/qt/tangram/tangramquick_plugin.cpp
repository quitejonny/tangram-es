#include "tangramquick_plugin.h"
#include "tangramquick.h"
#include "qtangrampolyline.h"
#include "qtangrampoint.h"

#include <qqml.h>

void TangramquickPlugin::registerTypes(const char *uri)
{
    // @uri com.mapzen.tangram
    qmlRegisterType<QDeclarativeTangramMap>(uri, 1, 0, "TangramMap");
    qmlRegisterType<QTangramPolyline>(uri, 1, 0, "TangramPolyline");
    qmlRegisterType<QTangramPolylineProperties>(uri, 1, 0, "TangramPolylineProperties");
    qmlRegisterType<QTangramGeometry>(uri, 1, 0, "TangramGeometry");
    qmlRegisterType<QTangramPoint>(uri, 1, 0, "TangramPoint");
    qmlRegisterType<QTangramPointProperties>(uri, 1, 0, "TangramPointProperties");
}


