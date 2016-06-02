#ifndef PLATFORM_QT_H
#define PLATFORM_QT_H

#include "platform.h"
#include <QEvent>
#include <QObject>

#define TANGRAM_REQ_RENDER_EVENT_TYPE   (QEvent::Type)(QEvent::User + 1)

void processNetworkQueue();

void finishUrlRequests();

void registerItem(QObject *quickItem);

#endif // PLATFORM_QT_H

