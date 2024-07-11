#ifndef GOOGLEFIT_H
#define GOOGLEFIT_H

#if __has_include("secret.h")
#include "secret.h"
#endif
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtNetwork>
#include <QOAuth2AuthorizationCodeFlow>
#include <QDesktopServices>

class GoogleFitAuth : public QObject
{
    Q_OBJECT

  public:
    explicit GoogleFitAuth(QObject *parent = nullptr) : QObject(parent)
    {
        oauth2 = new QOAuth2AuthorizationCodeFlow(this);
        oauth2->setScope("https://www.googleapis.com/auth/fitness.activity.write");

        connect(oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
                &QDesktopServices::openUrl);

        connect(oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &GoogleFitAuth::onGranted);

               // Configura le impostazioni del client OAuth2
        oauth2->setAuthorizationUrl(QUrl("https://accounts.google.com/o/oauth2/auth"));
        oauth2->setAccessTokenUrl(QUrl("https://oauth2.googleapis.com/token"));
        oauth2->setClientIdentifier("YOUR_CLIENT_ID");
        oauth2->setClientIdentifierSharedKey("YOUR_CLIENT_SECRET");
    }

    void authenticate()
    {
        oauth2->grant();
    }

    QString getAccessToken() const
    {
        return oauth2->token();
    }

  signals:
    void authenticated();

  private slots:
    void onGranted()
    {
        qDebug() << "Authentication successful!";
        qDebug() << "Access Token:" << oauth2->token();
        emit authenticated();
    }

  private:
    QOAuth2AuthorizationCodeFlow *oauth2;
};

class GoogleFitUploader : public QObject
{
    Q_OBJECT

  public:
    explicit GoogleFitUploader(QObject *parent = nullptr) : QObject(parent) {
        if(!auth)
            auth = new GoogleFitAuth();
        auth->authenticate();
    }

    struct WorkoutDataPoint {
        qint64 timestamp;  // in seconds
        int cadence;
        int watts;
        double speed;
        double distance;
        int heartRate;
    };

    void uploadWorkout(const QVector<WorkoutDataPoint>& dataPoints)
    {
        QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        connect(manager, &QNetworkAccessManager::finished, this, &GoogleFitUploader::onReplyFinished);

        QUrl url("https://www.googleapis.com/fitness/v1/users/me/dataset:aggregate");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", QString("Bearer %1").arg(auth->getAccessToken()).toUtf8());

        QJsonObject requestBody;
        requestBody["minStartTimeNs"] = QString::number(dataPoints.first().timestamp * 1000000);
        requestBody["maxEndTimeNs"] = QString::number(dataPoints.last().timestamp * 1000000);

        QJsonArray dataSources;
        dataSources.append(createDataSource("com.google.cycling.pedaling.cadence", "rpm"));
        dataSources.append(createDataSource("com.google.cycling.pedaling.cumulative", "watts"));
        dataSources.append(createDataSource("com.google.speed", "m/s"));
        dataSources.append(createDataSource("com.google.distance.delta", "m"));
        dataSources.append(createDataSource("com.google.heart_rate.bpm", "bpm"));
        requestBody["dataSources"] = dataSources;

        QJsonArray dataset;
        for (const auto& point : dataPoints) {
            dataset.append(createDataPoint("com.google.cycling.pedaling.cadence", point.timestamp, point.cadence));
            dataset.append(createDataPoint("com.google.cycling.pedaling.cumulative", point.timestamp, point.watts));
            dataset.append(createDataPoint("com.google.speed", point.timestamp, point.speed));
            dataset.append(createDataPoint("com.google.distance.delta", point.timestamp, point.distance));
            dataset.append(createDataPoint("com.google.heart_rate.bpm", point.timestamp, point.heartRate));
        }
        requestBody["dataset"] = dataset;

        QJsonDocument jsonDoc(requestBody);
        manager->post(request, jsonDoc.toJson());
    }
  private:

    GoogleFitAuth* auth = nullptr;

    QJsonObject createDataSource(const QString& dataType, const QString& unit)
    {
        QJsonObject dataSource;
        QJsonObject dataTypeObj;
        dataTypeObj["name"] = dataType;

        QJsonArray fieldArray;
        QJsonObject fieldObj;
        fieldObj["name"] = "value";
        fieldObj["format"] = "floatPoint";
        if (!unit.isEmpty()) {
            fieldObj["units"] = unit;
        }
        fieldArray.append(fieldObj);

        dataTypeObj["field"] = fieldArray;
        dataSource["dataType"] = dataTypeObj;

        return dataSource;
    }

    QJsonObject createDataPoint(const QString& dataType, qint64 timestampSeconds, double value)
    {
        QJsonObject dataPoint;
        dataPoint["dataTypeName"] = dataType;
        dataPoint["startTimeNanos"] = QString::number(timestampSeconds * 1000000000);
        dataPoint["endTimeNanos"] = QString::number(timestampSeconds * 1000000000);
        dataPoint["value"] = QJsonArray{QJsonObject{{"fpVal", value}}};
        return dataPoint;
    }


  private slots:
    void onReplyFinished(QNetworkReply *reply)
    {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "Workout uploaded successfully!";
        } else {
            qDebug() << "Error uploading workout:" << reply->errorString();
        }
        reply->deleteLater();
    }
};

#endif // GOOGLEFIT_H
