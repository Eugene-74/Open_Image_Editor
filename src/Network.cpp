#include "Network.hpp"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/**
 * @brief Check if there is an active internet connection
 * @return true if connected, false otherwise
 */
bool hasConnection() {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl("http://www.google.com"));
    QNetworkReply* reply = manager.get(request);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bool connected = false;

    if (reply->error() == QNetworkReply::NoError) {
        connected = true;
    }

    reply->deleteLater();

    return connected;
}