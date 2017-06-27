#include "contentdownloader.h"
#include <QNetworkRequest>
#include <QDebug>
#include <vector>

ContentDownloader::ContentDownloader(QObject *parent)
    : QObject(parent),
      m_maximumWorkers(3),
      m_networkManager(parent)
{
    qRegisterMetaType<std::string>();
    qRegisterMetaType<Tangram::UrlCallback>();
    connect(this, SIGNAL(addTask(std::string,Tangram::UrlCallback)), this, SLOT(queueTask(std::string,Tangram::UrlCallback)));
    connect(this, SIGNAL(cancelTask(std::string)), this, SLOT(onTaskCanceled(std::string)));
    connect(this, SIGNAL(finishTasks()), this, SLOT(onTasksFinished()));
}

void ContentDownloader::queueTask(const std::string &url, const Tangram::UrlCallback &callback)
{
    std::shared_ptr<DownloadTask> task(new DownloadTask{url, callback});
    m_taskQueue.enqueue(task);
    processQueue();
}

void ContentDownloader::processQueue()
{
    while (!m_taskQueue.isEmpty() && m_replies.count() < m_maximumWorkers) {
        auto task = m_taskQueue.dequeue();
        QNetworkRequest request(QUrl(QString::fromStdString(task->url)));
        QNetworkReply *reply = m_networkManager.get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(processReply()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processError(QNetworkReply::NetworkError)));
        m_replies.insert(reply, task);
    }
}

void ContentDownloader::processError(QNetworkReply::NetworkError error)
{
    if (error != QNetworkReply::OperationCanceledError)
        qDebug() << Q_FUNC_INFO << error;
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    m_replies.remove(reply);
    reply->deleteLater();
}

void ContentDownloader::onTaskCanceled(const std::string &url)
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

void ContentDownloader::onTasksFinished()
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
        std::vector<char> content(bytes.begin(), bytes.end());
        task->callback(std::move(content));
    }

    if (reply)
        reply->deleteLater();
    processQueue();
}
