#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <QVariant>

#include "urlWorker.h"
#include "platform_qt.h"
#include "platform_gl.h"
#include <QOpenGLFunctions>

#include <libgen.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <QCoreApplication>
#include <QObject>
#include <QWindow>

#define NUM_WORKERS 3

#define DEFAULT "fonts/NotoSans-Regular.ttf"
#define FONT_AR "fonts/NotoNaskh-Regular.ttf"
#define FONT_HE "fonts/NotoSansHebrew-Regular.ttf"
#define FONT_JA "fonts/DroidSansJapanese.ttf"
#define FALLBACK "fonts/DroidSansFallback.ttf"

QOpenGLExtraFunctions *__qt_gl_funcs;

static std::string s_resourceRoot;

static UrlWorker s_Workers[NUM_WORKERS];
static std::list<std::unique_ptr<UrlTask>> s_urlTaskQueue;

static QObject *s_quickItem = NULL;

void setQtGlFunctions(QOpenGLContext *context)
{
    __qt_gl_funcs = context->extraFunctions();
    __qt_gl_funcs->initializeOpenGLFunctions();
    __qt_gl_funcs->glLinkProgram(0);
}

void logMsg(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

void processNetworkQueue() {
    // attach workers to NetWorkerData
    auto taskItr = s_urlTaskQueue.begin();
    for(auto& worker : s_Workers) {
        if(taskItr == s_urlTaskQueue.end()) {
            break;
        }
        if(worker.isAvailable()) {
            worker.perform(std::move(*taskItr));
            taskItr = s_urlTaskQueue.erase(taskItr);
        }
    }
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

    std::string path = _path;
    std::string start = "/";
    if (path.compare(0, start.length(), start) == 0)
        path = "." + path;
    std::ifstream resource(path, std::ifstream::ate | std::ifstream::binary);

    if(!resource.is_open()) {
        logMsg("Failed to read file at path: %s\n", path);
        _size = 0;
        return nullptr;
    }

    _size = resource.tellg();

    resource.seekg(std::ifstream::beg);

    char* cdata = (char*) malloc(sizeof(char) * (_size));

    resource.read(cdata, _size);
    resource.close();

    return reinterpret_cast<unsigned char *>(cdata);
}

FontSourceHandle getFontHandle(const char* _path) {
    FontSourceHandle fontSourceHandle = [_path](size_t* _size) -> unsigned char* {
        logMsg("Loading font %s", _path);

        auto cdata = bytesFromFile(_path, *_size);

        return cdata;
    };

    return fontSourceHandle;
}

std::vector<FontSourceHandle> systemFontFallbacksHandle() {
    std::vector<FontSourceHandle> handles;

    handles.push_back(getFontHandle(DEFAULT));
    handles.push_back(getFontHandle(FONT_AR));
    handles.push_back(getFontHandle(FONT_HE));
    handles.push_back(getFontHandle(FONT_JA));
    handles.push_back(getFontHandle(FALLBACK));

    return handles;
}

// System fonts are not available on linux yet, we will possibly use FontConfig in the future, for
// references see the tizen platform implementation of system fonts
unsigned char* systemFont(const std::string& _name, const std::string& _weight, const std::string& _face, size_t* _size) {
    return nullptr;
}

bool startUrlRequest(const std::string& _url, UrlCallback _callback) {
    std::unique_ptr<UrlTask> task(new UrlTask(_url, _callback));
    for(auto& worker : s_Workers) {
        if(worker.isAvailable()) {
            worker.perform(std::move(task));
            return true;
        }
    }
    s_urlTaskQueue.push_back(std::move(task));
    return true;

    /*if (widget) {
        widget->startUrlRequest(_url, _calback);
    }*/
}

void cancelUrlRequest(const std::string& _url) {

    // Only clear this request if a worker has not started operating on it!!
    // otherwise it gets too convoluted with curl!
    auto itr = s_urlTaskQueue.begin();
    while(itr != s_urlTaskQueue.end()) {
        if((*itr)->url == _url) {
            itr = s_urlTaskQueue.erase(itr);
        } else {
            itr++;
        }
    }
}

void finishUrlRequests() {
    for(auto& worker : s_Workers) {
        worker.join();
    }
    s_urlTaskQueue.clear();
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
    if (!s_quickItem)
        s_quickItem = quickItem;
}

void initGLExtensions()
{
}
