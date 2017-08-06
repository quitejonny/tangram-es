#ifndef PLATFORM_QT_H
#define PLATFORM_QT_H

#include "platform.h"
#include <QEvent>
#include <QtGui/qopengl.h>


#define TANGRAM_REQ_RENDER_EVENT_TYPE   (QEvent::Type)(QEvent::User + 1)

class QObject;
class ContentDownloader;
class QOpenGLContext;

namespace Tangram {

class QtPlatform : public Platform {

public:
    QtPlatform(QObject *item = 0);
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

    void setDownloader(ContentDownloader *downloader);

    void setItem(QObject *item);

protected:
    bool bytesFromFileSystem(const char* _path, std::function<char*(size_t)> _allocator) const;

private:
    QObject *m_item;
    ContentDownloader *m_downloader;
    bool m_isContiuous;

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

