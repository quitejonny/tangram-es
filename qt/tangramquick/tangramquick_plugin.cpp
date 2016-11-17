#include "tangramquick_plugin.h"
#include "tangramquick.h"
#include "qtangrampolyline.h"

#include <qqml.h>

void TangramquickPlugin::registerTypes(const char *uri)
{
    // @uri com.mapzen.tangram
    qmlRegisterType<QDeclarativeTangramMap>(uri, 1, 0, "TangramMap");
    qmlRegisterType<QTangramPolyline>(uri, 1, 0, "TangramPolyline");
    qmlRegisterType<QTangramPolylineProperties>(uri, 1, 0, "TangramPolylineProperties");
    qmlRegisterType<QTangramGeometry>(uri, 1, 0, "TangramGeometry");
}


