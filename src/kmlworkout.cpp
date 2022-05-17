#include "kmlworkout.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include <cstdarg>

//"http://veloroutes.org/elevation/?location=44.50578%2C10.85431&units=m"

QList<trainrow> kmlworkout::load(const QString &filename) {
    QSettings settings;
    // QList<trainrow> list; //NOTE: clazy-unuzed-non-trivial-variable
    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    return load(input.readAll());
}

QList<trainrow> kmlworkout::load(const QByteArray &input) {
    QSettings settings;
    QList<trainrow> list;
    QXmlStreamReader stream(input);
    while (!stream.atEnd()) {
        stream.readNext();
        QString name = stream.name().toString();
        QString text = stream.text().toString();
        QXmlStreamAttributes atts = stream.attributes();
        if (name.toLower().contains(QStringLiteral("coordinates"))) {
            stream.readNext();
            QNetworkAccessManager manager;
            QString text = stream.text().toString();
            QStringList coordianates = text.split('\n');
            foreach (QString c, coordianates) {
                trainrow r;

                QStringList lonlat = c.split(',', Qt::SplitBehaviorFlags::SkipEmptyParts);
                if (lonlat.length() > 1) {
                    r.longitude = lonlat.at(0).toDouble();
                    r.latitude = lonlat.at(1).toDouble();

                    if (r.longitude != 0 && r.latitude != 0) {
                        QString u = QString("http://veloroutes.org/elevation/?location=%1\%2C%2&units=m")
                                        .arg(r.latitude)
                                        .arg(r.longitude);
                        QNetworkRequest request;
                        request.setUrl(u);
                        request.setTransferTimeout(5000);
                        QNetworkReply *reply = manager.get(request);
                        QEventLoop loop;
                        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
                        loop.exec(QEventLoop::ExcludeUserInputEvents);
                        if (reply->error() != QNetworkReply::NoError)
                            text = ("error" + reply->errorString());
                        else
                            text = ("response" + reply->readAll());
                        qDebug() << text;
                        delete reply;

                        list.append(r);
                    }
                }
            }
        }
    }
    return list;
}
