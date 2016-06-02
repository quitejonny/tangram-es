#ifndef TANGRAMQUICK_PLUGIN_H
#define TANGRAMQUICK_PLUGIN_H

#include <QQmlExtensionPlugin>

class TangramquickPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri);
};

#endif // TANGRAMQUICK_PLUGIN_H

