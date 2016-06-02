#include "tangramquick_plugin.h"
#include "tangramquick.h"

#include <qqml.h>

void TangramquickPlugin::registerTypes(const char *uri)
{
    // @uri com.mapzen.tangram
    qmlRegisterType<TangramQuick>(uri, 1, 0, "Tangram");
}


