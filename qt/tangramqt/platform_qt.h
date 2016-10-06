#ifndef PLATFORM_QT_H
#define PLATFORM_QT_H

#include "platform.h"
#include <QEvent>
#include <QOpenGLContext>

#define TANGRAM_REQ_RENDER_EVENT_TYPE   (QEvent::Type)(QEvent::User + 1)

void processNetworkQueue();

void finishUrlRequests();

void setQtGlFunctions(QOpenGLContext* context);

#endif // PLATFORM_QT_H

