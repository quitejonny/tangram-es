#pragma once

#ifdef DEBUG
#define GL_CHECK(STMT) do { STMT; Tangram::Error::glError(#STMT, __FILE__, __LINE__); } while (0)
#else
#define GL_CHECK(STMT) STMT;
#endif

#include "platform_qt.h"
#include <memory>

class QOpenGLFunctions;


extern QOpenGLFunctions *__qt_gl_funcs;

extern std::unique_ptr<OpenGLExtraFunctions> __qt_gl_extra_funcs;
