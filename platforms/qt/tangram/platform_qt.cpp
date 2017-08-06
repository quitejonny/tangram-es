#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <QVariant>
#include <QFile>

#include "platform_qt.h"
#include "platform_gl.h"
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QDebug>

#include <libgen.h>
#include <unistd.h>
#include <memory>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <QCoreApplication>
#include <QWindow>

#include "contentdownloader.h"

#include <QObject>

#define NUM_WORKERS 5

#define DEFAULT "fonts/NotoSans-Regular.ttf"
#define FONT_AR "fonts/NotoNaskh-Regular.ttf"
#define FONT_HE "fonts/NotoSansHebrew-Regular.ttf"
#define FONT_JA "fonts/DroidSansJapanese.ttf"
#define FALLBACK "fonts/DroidSansFallback.ttf"

namespace Tangram {

QOpenGLFunctions *__qt_gl_funcs = NULL;
std::unique_ptr<OpenGLExtraFunctions> __qt_gl_extra_funcs;


void logMsg(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void initGLExtensions()
{
}

void setCurrentThreadPriority(int priority){
    int tid = syscall(SYS_gettid);
    //int  p1 = getpriority(PRIO_PROCESS, tid);

    setpriority(PRIO_PROCESS, tid, priority);

    //int  p2 = getpriority(PRIO_PROCESS, tid);
    //logMsg("set niceness: %d -> %d\n", p1, p2);
}

QtPlatform::QtPlatform(QObject *item)
    : m_item(item),
      m_downloader(nullptr)
{
}

QtPlatform::~QtPlatform()
{
}

void QtPlatform::requestRender() const
{
    if (m_item)
        QCoreApplication::postEvent(m_item, new QEvent(TANGRAM_REQ_RENDER_EVENT_TYPE));
}

void QtPlatform::setContinuousRendering(bool _isContinuous)
{
    if (_isContinuous == m_isContiuous)
        return;

    m_isContiuous = _isContinuous;
}

bool QtPlatform::isContinuousRendering() const
{
    return m_isContiuous;
}

bool QtPlatform::bytesFromFileSystem(const char* _path, std::function<char*(size_t)> _allocator) const
{
    auto path = QString::fromUtf8(_path);
    if (path.startsWith("qrc:"))
        path = path.remove(0, 3);

    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Failed to read file at path:" << _path;
        return false;
    }

    size_t size = file.size();
    char* cdata = _allocator(size);

    file.read(cdata, size);
    file.close();

    return true;
}

std::string QtPlatform::stringFromFile(const char* _path) const {
    std::string out;
    if (!_path || strlen(_path) == 0) { return out; }

    std::string data;

    auto allocator = [&](size_t size) {
        data.resize(size);
        return &data[0];
    };

    bytesFromFileSystem(_path, allocator);

    return data;
}

std::vector<char> QtPlatform::bytesFromFile(const char* _path) const {
    if (!_path || strlen(_path) == 0) { return {}; }

    std::vector<char> data;

    auto allocator = [&](size_t size) {
        data.resize(size);
        return data.data();
    };

    bytesFromFileSystem(_path, allocator);

    return data;
}

bool QtPlatform::startUrlRequest(const std::string& _url, UrlCallback _callback) {
    if (m_downloader) {
        emit m_downloader->addTask(_url, _callback);
        return true;
    } else {
        return false;
    }
}

void QtPlatform::cancelUrlRequest(const std::string& _url) {
    if (m_downloader)
        emit m_downloader->cancelTask(_url);
}

void QtPlatform::finishUrlRequests() {
    if (m_downloader)
        emit m_downloader->finishTasks();
}

void QtPlatform::setDownloader(ContentDownloader *downloader)
{
    m_downloader = downloader;
    m_downloader->setMaximumWorkers(NUM_WORKERS);
}

void QtPlatform::setItem(QObject *item)
{
    m_item = item;
}

std::vector<FontSourceHandle> QtPlatform::systemFontFallbacksHandle() const {
    std::vector<FontSourceHandle> handles;

    handles.emplace_back(DEFAULT);
    handles.emplace_back(FONT_AR);
    handles.emplace_back(FONT_HE);
    handles.emplace_back(FONT_JA);
    handles.emplace_back(FALLBACK);

    return handles;
}

OpenGLExtraFunctions::OpenGLExtraFunctions(QOpenGLContext *context)
{
    qDebug() << Q_FUNC_INFO;
    if (context->hasExtension("GL_OES_mapbuffer")) {
        unmapBuffer = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glUnmapBufferOES"));
        mapBuffer = reinterpret_cast<GLvoid* (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glMapBufferOES"));
    } else {
        unmapBuffer = reinterpret_cast<GLboolean (QOPENGLF_APIENTRYP)(GLenum )>(context->getProcAddress("glUnmapBuffer"));
        mapBuffer = reinterpret_cast<GLvoid* (QOPENGLF_APIENTRYP)(GLenum , GLenum )>(context->getProcAddress("glMapBuffer"));
    }
    if (context->hasExtension("GL_OES_vertex_array_object")) {
        genVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenVertexArraysOES"));
        deleteVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteVertexArraysOES"));
        bindVertexArray = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glBindVertexArrayOES"));
    } else {
        genVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , GLuint *)>(context->getProcAddress("glGenVertexArrays"));
        deleteVertexArrays = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLsizei , const GLuint *)>(context->getProcAddress("glDeleteVertexArrays"));
        bindVertexArray = reinterpret_cast<void (QOPENGLF_APIENTRYP)(GLuint )>(context->getProcAddress("glBindVertexArray"));
    }
}

void setQtGlFunctions(QOpenGLContext *gl_context)
{
    __qt_gl_extra_funcs = std::unique_ptr<OpenGLExtraFunctions> (new OpenGLExtraFunctions(gl_context));
    __qt_gl_funcs = gl_context->functions();
    __qt_gl_funcs->initializeOpenGLFunctions();
}

} // namespace Tangram
