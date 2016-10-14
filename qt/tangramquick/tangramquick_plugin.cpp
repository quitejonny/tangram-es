#include "tangramquick_plugin.h"
#include "tangramquick.h"

#include <qqml.h>

void TangramquickPlugin::registerTypes(const char *uri)
{
    // @uri com.mapzen.tangram
    qmlRegisterType<QDeclarativeTangramMap>(uri, 1, 0, "TangramMap");
}


