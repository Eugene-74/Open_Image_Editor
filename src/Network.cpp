#include "Network.hpp"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/**
 * @brief Check if there is an active internet connection
 * @return true if connected, false otherwise
 */
bool hasConnection() {
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkRequest request(QUrl("https://www.google.com"));
    QNetworkReply* reply = manager->get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bool connected = false;

    if (reply->error() == QNetworkReply::NoError) {
        connected = true;
    }

    reply->deleteLater();
    manager->deleteLater();

    return connected;
}