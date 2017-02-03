#ifndef PLATFORM_QT_H
#define PLATFORM_QT_H

#include "platform.h"
#include <QEvent>
#include <QtGui/qopengl.h>

#define TANGRAM_REQ_RENDER_EVENT_TYPE   (QEvent::Type)(QEvent::User + 1)

class QObject;
class QOpenGLContext;

void finishUrlRequests();

void registerItem(QObject *quickItem);

void setQtGlFunctions(QOpenGLContext *gl_context);

class OpenGLExtraFunctions
{
public:
    OpenGLExtraFunctions(QOpenGLContext *context);

    GLboolean (QOPENGLF_APIENTRYP unmapBuffer)(GLenum target);
    GLvoid* (QOPENGLF_APIENTRYP mapBuffer)(GLenum target, GLenum access);
    void (QOPENGLF_APIENTRYP genVertexArrays)(GLsizei n, GLuint *arrays);
    void (QOPENGLF_APIENTRYP deleteVertexArrays)(GLsizei n, const GLuint *arrays);
    void (QOPENGLF_APIENTRYP bindVertexArray)(GLuint array);
};

#endif // PLATFORM_QT_H

