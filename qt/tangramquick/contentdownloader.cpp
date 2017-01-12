#include "contentdownloader.h"
#include <QDebug>

ContentDownloader::ContentDownloader(QObject *parent)
    : QObject(parent),
      m_maximumWorkers(3),
      m_networkManager(parent)
{

}

void ContentDownloader::addTask(const std::string &url, const UrlCallback &callback)
{
    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    m_networkManager.get(request);

    std::shared_ptr<DownloadTask> task(new DownloadTask{url, callback, std::move(request)});
    m_taskQueue.enqueue(task);

    processQueue();
}

void ContentDownloader::processQueue()
{
    while (!m_taskQueue.isEmpty() && m_replies.count() < m_maximumWorkers) {
        auto task = m_taskQueue.dequeue();
        QNetworkReply *reply = m_networkManager.get(task->request);
        connect(reply, SIGNAL(finished()), this, SLOT(processReply()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
        m_replies.insert(reply, task);
    }
}

void ContentDownloader::processError(QNetworkReply::NetworkError error)
{
    qDebug() << error;
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    m_replies.remove(reply);
    reply->deleteLater();
}

void ContentDownloader::cancelTask(const std::string &url)
{
    for (int i = 0; i < m_taskQueue.count(); ++i) {
        if (m_taskQueue.at(i)->url == url) {
            m_taskQueue.removeAt(i);
            return;
        }
    }

    for (auto *reply : m_replies.keys()) {
        auto task = m_replies.value(reply);
        if (task->url == url) {
            m_replies.remove(reply);
            reply->abort();
            reply->deleteLater();
            return;
        }
    }
}

void ContentDownloader::finishTasks()
{
    m_taskQueue.clear();
    for (auto *reply : m_replies.keys()) {
        reply->abort();
        reply->deleteLater();
    }

    m_replies.clear();
}

void ContentDownloader::setMaximumWorkers(int maximumWorkers)
{
    m_maximumWorkers = maximumWorkers;
}

int ContentDownloader::maximumWorkers()
{
    return m_maximumWorkers;
}

void ContentDownloader::processReply()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());

    if (m_replies.contains(reply)) {
        auto task = m_replies.value(reply);
        m_replies.remove(reply);

        QByteArray bytes = reply->readAll();
        std::vector<char> content(bytes.constData(), bytes.constData() + bytes.size());
        task->callback(std::move(content));
    }

    processQueue();
    reply->deleteLater();
}
