#pragma once

#ifdef DEBUG
#define GL_CHECK(STMT) do { STMT; Tangram::Error::glError(#STMT, __FILE__, __LINE__); } while (0)
#else
#define GL_CHECK(STMT) STMT;
#endif


#include "gl.h"
#include <QOpenGLExtraFunctions>

extern QOpenGLExtraFunctions *__qt_gl_funcs;
