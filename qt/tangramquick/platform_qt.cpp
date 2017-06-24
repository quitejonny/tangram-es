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
#include <QObject>
#include <QWindow>

#include "contentdownloader.h"

#define NUM_WORKERS 5

#define DEFAULT "fonts/NotoSans-Regular.ttf"
#define FONT_AR "fonts/NotoNaskh-Regular.ttf"
#define FONT_HE "fonts/NotoSansHebrew-Regular.ttf"
#define FONT_JA "fonts/DroidSansJapanese.ttf"
#define FALLBACK "fonts/DroidSansFallback.ttf"

QOpenGLFunctions *__qt_gl_funcs = NULL;
std::unique_ptr<OpenGLExtraFunctions> __qt_gl_extra_funcs;

static std::string s_resourceRoot;

static QObject *s_quickItem = NULL;

static ContentDownloader *s_downloader = NULL;


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

void logMsg(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void requestRender() {
    if (s_quickItem)
        QCoreApplication::postEvent(s_quickItem, new QEvent(TANGRAM_REQ_RENDER_EVENT_TYPE));
}

void setContinuousRendering(bool _isContinuous)
{
    if (s_quickItem)
        s_quickItem->setProperty("continuousRendering", QVariant::fromValue(_isContinuous));
}

bool isContinuousRendering()
{
    if (s_quickItem)
        return s_quickItem->property("continuousRendering").toBool();
    return false;
}

std::string setResourceRoot(const char* _path) {

    std::string dir(_path);

    s_resourceRoot = std::string(dirname(&dir[0])) + '/';

    std::string base(_path);

    return std::string(basename(&base[0]));

}

std::string stringFromFile(const char* _path) {

    size_t length = 0;
    unsigned char* bytes = bytesFromFile(_path, length);
    if (!bytes) { return {}; }

    std::string out(reinterpret_cast<char*>(bytes), length);
    free(bytes);

    return out;
}

unsigned char* bytesFromFile(const char* _path, size_t& _size) {

    auto path = QString::fromUtf8(_path);
    if (path.startsWith("qrc:"))
        path = path.remove(0, 3);

    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        _size = 0;
        return nullptr;
    }

    qDebug() << Q_FUNC_INFO << path << file.size();
    _size = file.size();
    char* cdata = (char*) malloc(sizeof(char) * (_size));

    file.read(cdata, _size);
    file.close();

    return reinterpret_cast<unsigned char *>(cdata);
}

std::vector<FontSourceHandle> systemFontFallbacksHandle() {
    std::vector<FontSourceHandle> handles;

    handles.emplace_back(DEFAULT);
    handles.emplace_back(FONT_AR);
    handles.emplace_back(FONT_HE);
    handles.emplace_back(FONT_JA);
    handles.emplace_back(FALLBACK);

    return handles;
}

// System fonts are not available on linux yet, we will possibly use FontConfig in the future, for
// references see the tizen platform implementation of system fonts
unsigned char* systemFont(const std::string& _name, const std::string& _weight, const std::string& _face, size_t* _size) {
    return nullptr;
}

bool startUrlRequest(const std::string& _url, UrlCallback _callback) {
    s_downloader->addTask(_url, _callback);

    return true;

    /*if (widget) {
        widget->startUrlRequest(_url, _calback);
    }*/
}

void cancelUrlRequest(const std::string& _url) {
    s_downloader->cancelTask(_url);
}

void finishUrlRequests() {
    s_downloader->finishTasks();
}

void setCurrentThreadPriority(int priority){
    int tid = syscall(SYS_gettid);
    //int  p1 = getpriority(PRIO_PROCESS, tid);

    setpriority(PRIO_PROCESS, tid, priority);

    //int  p2 = getpriority(PRIO_PROCESS, tid);
    //logMsg("set niceness: %d -> %d\n", p1, p2);
}

void registerItem(QObject *quickItem)
{
    if (!s_quickItem) {
        s_quickItem = quickItem;
        s_downloader = new ContentDownloader(quickItem);
        s_downloader->setMaximumWorkers(NUM_WORKERS);
    }
}

void initGLExtensions()
{
}
