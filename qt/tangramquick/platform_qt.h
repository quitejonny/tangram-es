#ifndef PLATFORM_QT_H
#define PLATFORM_QT_H

#include "platform.h"
#include <QEvent>
#include <QObject>
#include <QOpenGLContext>

#define TANGRAM_REQ_RENDER_EVENT_TYPE   (QEvent::Type)(QEvent::User + 1)

void finishUrlRequests();

void registerItem(QObject *quickItem);

void setQtGlFunctions(QOpenGLContext* context);

#endif // PLATFORM_QT_H

