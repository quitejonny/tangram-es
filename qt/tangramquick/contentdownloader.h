#ifndef CONTENTDOWNLOADER_H
#define CONTENTDOWNLOADER_H

#include <QObject>
#include <QQueue>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "platform.h"
#include <memory>


struct DownloadTask {

    const std::string url;
    UrlCallback callback;

    bool operator=(const DownloadTask &other) {
        return other.url == this->url;
    }
};


class ContentDownloader : public QObject
{
    Q_OBJECT
public:
    explicit ContentDownloader(QObject *parent = 0);

    void addTask(const std::string &url, const UrlCallback &callback);
    void cancelTask(const std::string &url);
    void finishTasks();

    void setMaximumWorkers(int maximumWorkers);
    int maximumWorkers();

signals:
    void updateQueue();

public slots:
    void processReply();
    void processError(QNetworkReply::NetworkError error);

private slots:
    void processQueue();

private:
    int m_maximumWorkers;
    QNetworkAccessManager m_networkManager;
    QQueue<std::shared_ptr<DownloadTask>> m_taskQueue;
    QMap<QNetworkReply*, std::shared_ptr<DownloadTask>> m_replies;
};

#endif // CONTENTDOWNLOADER_H
