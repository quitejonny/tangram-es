#ifndef PLATFORM_QT_H
#define PLATFORM_QT_H

#include "platform.h"
#include <QEvent>
#include <QtGui/qopengl.h>
#include "contentdownloader.h"


#define TANGRAM_REQ_RENDER_EVENT_TYPE   (QEvent::Type)(QEvent::User + 1)

class QObject;
class QDeclarativeTangramMap;
class QOpenGLContext;

namespace Tangram {

class QtPlatform : public Platform {

public:
    QtPlatform();
    QtPlatform(QDeclarativeTangramMap *quickItem);
    ~QtPlatform() override;
    void requestRender() const override;

    void setContinuousRendering(bool _isContinuous) override;
    bool isContinuousRendering() const override;

    std::string stringFromFile(const char* _path) const override;
    std::vector<char> bytesFromFile(const char* _path) const override;

    bool startUrlRequest(const std::string& _url, UrlCallback _callback) override;
    void cancelUrlRequest(const std::string& _url) override;
    void finishUrlRequests();

    std::vector<FontSourceHandle> systemFontFallbacksHandle() const override;

protected:
    bool bytesFromFileSystem(const char* _path, std::function<char*(size_t)> _allocator) const;

private:
    QDeclarativeTangramMap *m_quickItem;
    ContentDownloader *m_downloader;

};


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

void setQtGlFunctions(QOpenGLContext *gl_context);

} // namespace Tangram

#endif // PLATFORM_QT_H

