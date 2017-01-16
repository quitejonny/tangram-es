#ifndef PLATFORM_QT_H
#define PLATFORM_QT_H

#include "platform.h"
#include <QEvent>

#define TANGRAM_REQ_RENDER_EVENT_TYPE   (QEvent::Type)(QEvent::User + 1)

class QObject;
class QOpenGLExtraFunctions;

void finishUrlRequests();

void registerItem(QObject *quickItem);

void setQtGlFunctions(QOpenGLExtraFunctions *gl_funcs);

#endif // PLATFORM_QT_H

