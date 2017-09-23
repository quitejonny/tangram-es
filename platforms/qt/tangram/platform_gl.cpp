#include "platform_gl.h"
#include "gl.h"
#include <QOpenGLFunctions>

namespace Tangram {

GLenum GL::getError() {
    return __qt_gl_funcs->glGetError();
}

const GLubyte* GL::getString(GLenum name) {
    return __qt_gl_funcs->glGetString(name);
}

void GL::clear(GLbitfield mask) {
    __qt_gl_funcs->glClear(mask);
}
void GL::lineWidth(GLfloat width) {
    __qt_gl_funcs->glLineWidth(width);
}
void GL::viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    __qt_gl_funcs->glViewport(x, y, width, height);
}

void GL::enable(GLenum id) {
    __qt_gl_funcs->glEnable(id);
}
void GL::disable(GLenum id) {
    __qt_gl_funcs->glDisable(id);
}
void GL::depthFunc(GLenum func) {
    __qt_gl_funcs->glDepthFunc(func);
}
void GL::depthMask(GLboolean flag) {
    __qt_gl_funcs->glDepthMask(flag);
}
void GL::depthRange(GLfloat n, GLfloat f) {
    __qt_gl_funcs->glDepthRangef(n, f);
}
void GL::clearDepth(GLfloat d) {
    __qt_gl_funcs->glClearDepthf(d);
}
void GL::blendFunc(GLenum sfactor, GLenum dfactor) {
    __qt_gl_funcs->glBlendFunc(sfactor, dfactor);
}
void GL::stencilFunc(GLenum func, GLint ref, GLuint mask) {
    __qt_gl_funcs->glStencilFunc(func, ref, mask);
}
void GL::stencilMask(GLuint mask) {
    __qt_gl_funcs->glStencilMask(mask);
}
void GL::stencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
    __qt_gl_funcs->glStencilOp(fail, zfail, zpass);
}
void GL::clearStencil(GLint s) {
    __qt_gl_funcs->glClearStencil(s);
}
void GL::colorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    __qt_gl_funcs->glColorMask(red, green, blue, alpha);
}
void GL::cullFace(GLenum mode) {
    __qt_gl_funcs->glCullFace(mode);
}
void GL::frontFace(GLenum mode) {
    __qt_gl_funcs->glFrontFace(mode);
}
void GL::clearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    __qt_gl_funcs->glClearColor(red, green, blue, alpha);
}
void GL::getIntegerv(GLenum pname, GLint *params ) {
    __qt_gl_funcs->glGetIntegerv(pname, params );
}

// Program
void GL::useProgram(GLuint program) {
    __qt_gl_funcs->glUseProgram(program);
}
void GL::deleteProgram(GLuint program) {
    __qt_gl_funcs->glDeleteProgram(program);
}
void GL::deleteShader(GLuint shader) {
    __qt_gl_funcs->glDeleteShader(shader);
}
GLuint GL::createShader(GLenum type) {
    return __qt_gl_funcs->glCreateShader(type);
}
GLuint GL::createProgram() {
    return __qt_gl_funcs->glCreateProgram();
}

void GL::compileShader(GLuint shader) {
    __qt_gl_funcs->glCompileShader(shader);
}
void GL::attachShader(GLuint program, GLuint shader) {
    __qt_gl_funcs->glAttachShader(program, shader);
}
void GL::linkProgram(GLuint program) {
    __qt_gl_funcs->glLinkProgram(program);
}

void GL::shaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length) {
        auto source = const_cast<const GLchar**>(string);
    __qt_gl_funcs->glShaderSource(shader, count, source, length);
}
void GL::getShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
    __qt_gl_funcs->glGetShaderInfoLog(shader, bufSize, length, infoLog);
}
void GL::getProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
    __qt_gl_funcs->glGetProgramInfoLog(program, bufSize, length, infoLog);
}
GLint GL::getUniformLocation(GLuint program, const GLchar *name) {
    return __qt_gl_funcs->glGetUniformLocation(program, name);
}
GLint GL::getAttribLocation(GLuint program, const GLchar *name) {
    return __qt_gl_funcs->glGetAttribLocation(program, name);
}
void GL::getProgramiv(GLuint program, GLenum pname, GLint *params) {
    __qt_gl_funcs->glGetProgramiv(program, pname, params);
}
void GL::getShaderiv(GLuint shader, GLenum pname, GLint *params) {
    __qt_gl_funcs->glGetShaderiv(shader, pname, params);
}

// Buffers
void GL::bindBuffer(GLenum target, GLuint buffer) {
    __qt_gl_funcs->glBindBuffer(target, buffer);
}
void GL::deleteBuffers(GLsizei n, const GLuint *buffers) {
    __qt_gl_funcs->glDeleteBuffers(n, buffers);
}
void GL::genBuffers(GLsizei n, GLuint *buffers) {
    __qt_gl_funcs->glGenBuffers(n, buffers);
}
void GL::bufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
    __qt_gl_funcs->glBufferData(target, size, data, usage);
}
void GL::bufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data) {
    __qt_gl_funcs->glBufferSubData(target, offset, size, data);
}
void GL::readPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                    GLenum format, GLenum type, GLvoid* pixels) {
    __qt_gl_funcs->glReadPixels(x, y, width, height, format, type, pixels);
}

// Texture
void GL::bindTexture(GLenum target, GLuint texture ) {
    __qt_gl_funcs->glBindTexture(target, texture);
}
void GL::activeTexture(GLenum texture) {
    __qt_gl_funcs->glActiveTexture(texture);
}
void GL::genTextures(GLsizei n, GLuint *textures ) {
    __qt_gl_funcs->glGenTextures(n, textures );
}
void GL::deleteTextures(GLsizei n, const GLuint *textures) {
    __qt_gl_funcs->glDeleteTextures(n, textures);
}
void GL::texParameteri(GLenum target, GLenum pname, GLint param ) {
    __qt_gl_funcs->glTexParameteri(target, pname, param );
}
void GL::texImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height,
                    GLint border, GLenum format, GLenum type, const GLvoid *pixels) {
    __qt_gl_funcs->glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
}

void GL::texSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                       GLenum format, GLenum type, const GLvoid *pixels) {
    __qt_gl_funcs->glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

void GL::generateMipmap(GLenum target) {
    __qt_gl_funcs->glGenerateMipmap(target);
}

void GL::enableVertexAttribArray(GLuint index) {
    __qt_gl_funcs->glEnableVertexAttribArray(index);
}
void GL::disableVertexAttribArray(GLuint index) {
    __qt_gl_funcs->glDisableVertexAttribArray(index);
}
void GL::vertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                             GLsizei stride, const void *pointer) {
    __qt_gl_funcs->glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void GL::drawArrays(GLenum mode, GLint first, GLsizei count ) {
    __qt_gl_funcs->glDrawArrays(mode, first, count );
}
void GL::drawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices ) {
    __qt_gl_funcs->glDrawElements(mode, count, type, indices );
}

void GL::uniform1f(GLint location, GLfloat v0) {
    __qt_gl_funcs->glUniform1f(location, v0);
}
void GL::uniform2f(GLint location, GLfloat v0, GLfloat v1) {
    __qt_gl_funcs->glUniform2f(location, v0, v1);
}
void GL::uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    __qt_gl_funcs->glUniform3f(location, v0, v1, v2);
}
void GL::uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    __qt_gl_funcs->glUniform4f(location, v0, v1, v2, v3);
}

void GL::uniform1i(GLint location, GLint v0) {
    __qt_gl_funcs->glUniform1i(location, v0);
}
void GL::uniform2i(GLint location, GLint v0, GLint v1) {
    __qt_gl_funcs->glUniform2i(location, v0, v1);
}
void GL::uniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
    __qt_gl_funcs->glUniform3i(location, v0, v1, v2);
}
void GL::uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    __qt_gl_funcs->glUniform4i(location, v0, v1, v2, v3);
}

void GL::uniform1fv(GLint location, GLsizei count, const GLfloat *value) {
    __qt_gl_funcs->glUniform1fv(location, count, value);
}
void GL::uniform2fv(GLint location, GLsizei count, const GLfloat *value) {
    __qt_gl_funcs->glUniform2fv(location, count, value);
}
void GL::uniform3fv(GLint location, GLsizei count, const GLfloat *value) {
    __qt_gl_funcs->glUniform3fv(location, count, value);
}
void GL::uniform4fv(GLint location, GLsizei count, const GLfloat *value) {
    __qt_gl_funcs->glUniform4fv(location, count, value);
}
void GL::uniform1iv(GLint location, GLsizei count, const GLint *value) {
    __qt_gl_funcs->glUniform1iv(location, count, value);
}
void GL::uniform2iv(GLint location, GLsizei count, const GLint *value) {
    __qt_gl_funcs->glUniform2iv(location, count, value);
}
void GL::uniform3iv(GLint location, GLsizei count, const GLint *value) {
    __qt_gl_funcs->glUniform3iv(location, count, value);
}
void GL::uniform4iv(GLint location, GLsizei count, const GLint *value) {
    __qt_gl_funcs->glUniform4iv(location, count, value);
}

void GL::uniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    __qt_gl_funcs->glUniformMatrix2fv(location, count, transpose, value);
}
void GL::uniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    __qt_gl_funcs->glUniformMatrix3fv(location, count, transpose, value);
}
void GL::uniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
    __qt_gl_funcs->glUniformMatrix4fv(location, count, transpose, value);
}

// mapbuffer
void* GL::mapBuffer(GLenum target, GLenum access) {
    return __qt_gl_extra_funcs->mapBuffer(target, access);
}
GLboolean GL::unmapBuffer(GLenum target) {
    return __qt_gl_extra_funcs->unmapBuffer(target);
}

void GL::finish(void) {
    __qt_gl_funcs->glFinish();
}

// VAO
void GL::bindVertexArray(GLuint array) {
    __qt_gl_extra_funcs->bindVertexArray(array);
}
void GL::deleteVertexArrays(GLsizei n, const GLuint *arrays) {
    __qt_gl_extra_funcs->deleteVertexArrays(n, arrays);
}
void GL::genVertexArrays(GLsizei n, GLuint *arrays) {
    __qt_gl_extra_funcs->genVertexArrays(n, arrays);
}

// Framebuffer
void GL::bindFramebuffer(GLenum target, GLuint framebuffer) {
    __qt_gl_funcs->glBindFramebuffer(target, framebuffer);
}

void GL::genFramebuffers(GLsizei n, GLuint *framebuffers) {
    __qt_gl_funcs->glGenFramebuffers(n, framebuffers);
}

void GL::framebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget,
                              GLuint texture, GLint level) {
    __qt_gl_funcs->glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void GL::renderbufferStorage(GLenum target, GLenum internalformat, GLsizei width,
                             GLsizei height) {
    __qt_gl_funcs->glRenderbufferStorage(target, internalformat, width, height);
}

void GL::framebufferRenderbuffer(GLenum target, GLenum attachment,
                                 GLenum renderbuffertarget, GLuint renderbuffer) {
    __qt_gl_funcs->glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void GL::genRenderbuffers(GLsizei n, GLuint *renderbuffers) {
    __qt_gl_funcs->glGenRenderbuffers(n, renderbuffers);
}

void GL::bindRenderbuffer(GLenum target, GLuint renderbuffer) {
    __qt_gl_funcs->glBindRenderbuffer(target, renderbuffer);
}

void GL::deleteFramebuffers(GLsizei n, const GLuint *framebuffers) {
    __qt_gl_funcs->glDeleteFramebuffers(n, framebuffers);
}

void GL::deleteRenderbuffers(GLsizei n, const GLuint *renderbuffers) {
    __qt_gl_funcs->glDeleteRenderbuffers(n, renderbuffers);
}

GLenum GL::checkFramebufferStatus(GLenum target) {
    return __qt_gl_funcs->glCheckFramebufferStatus(target);
}

}
