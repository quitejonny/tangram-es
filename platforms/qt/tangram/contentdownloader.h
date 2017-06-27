#ifndef CONTENTDOWNLOADER_H
#define CONTENTDOWNLOADER_H

#include <QObject>
#include <QQueue>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "platform.h"
#include <memory>


struct DownloadTask {

    const std::string url;
    Tangram::UrlCallback callback;

    bool operator=(const DownloadTask &other) {
        return other.url == this->url;
    }
};

Q_DECLARE_METATYPE(std::string)
Q_DECLARE_METATYPE(Tangram::UrlCallback)


class ContentDownloader : public QObject
{
    Q_OBJECT
public:
    explicit ContentDownloader(QObject *parent = 0);

    void setMaximumWorkers(int maximumWorkers);
    int maximumWorkers();

signals:
    void addTask(const std::string &url, const Tangram::UrlCallback &callback);
    void cancelTask(const std::string &url);
    void finishTasks();

private slots:
    void queueTask(const std::string &url, const Tangram::UrlCallback &callback);
    void onTaskCanceled(const std::string &url);
    void onTasksFinished();
    void processQueue();
    void processReply();
    void processError(QNetworkReply::NetworkError error);

private:
    int m_maximumWorkers;
    QNetworkAccessManager m_networkManager;
    QQueue<std::shared_ptr<DownloadTask>> m_taskQueue;
    QMap<QNetworkReply*, std::shared_ptr<DownloadTask>> m_replies;
};

#endif // CONTENTDOWNLOADER_H
