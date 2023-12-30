#ifndef PLAYERSTATEWRAPPER_H
#define PLAYERSTATEWRAPPER_H

#include <QMap>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDebug>

class ZwiftRequest: public QObject {
    Q_OBJECT

public:
    ZwiftRequest(const QString& getAccessToken) : getAccessToken(getAccessToken) {}

    QString json(const QString& url) {
        QNetworkRequest request(QUrl(BASE_URL + url));
        request.setRawHeader("Accept", "application/json");
        request.setRawHeader("Authorization", "Bearer " + getAccessToken.toUtf8());

        QNetworkReply* reply = manager.get(request);
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error: " << reply->errorString();
            return "";
        }

        return reply->readAll();
    }

    QByteArray protobuf(const QString& url) {
        QNetworkRequest request(QUrl(BASE_URL + url));
        request.setRawHeader("Accept", "application/x-protobuf-lite");
        request.setRawHeader("Authorization", "Bearer " + getAccessToken.toUtf8());

        QNetworkReply* reply = manager.get(request);
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error: " << reply->errorString();
            return QByteArray();
        }

        return reply->readAll();
    }

private:
    QNetworkAccessManager manager;
    const QString BASE_URL = "https://us-or-rly101.zwift.com";
    const QString getAccessToken;
};

class World {
public:
    typedef struct PlayerState {
        char temp[24];
        qint32 id;
        qint64 worldTime;
        qint32 distance;
        qint32 roadTime;
        qint32 laps;
        qint32 speed;
        qint32 roadPosition;
        qint32 cadenceUHz;
        qint32 heartrate;
        qint32 power;
        qint64 heading;
        qint32 lean;
        qint32 climbing;
        qint32 time;
        qint32 f19;
        qint32 f20;
        qint32 progress;
        qint64 customisationId;
        qint32 justWatching;
        qint32 calories;
        float x;
        float altitude;
        float y;
        qint32 watchingRiderId;
        qint32 groupId;
        qint64 sport;
    } PlayerState;
    
    World(int worldId, const QString& getAccessToken) : worldId(worldId), request(getAccessToken) {}

    QString getPlayers() {
        return request.json("/relay/worlds/" + QString::number(worldId));
    }

    QByteArray playerStatus(int playerId) {
        QByteArray buffer = request.protobuf("/relay/worlds/" + QString::number(worldId) + "/players/" + QString::number(playerId));
        return buffer;
    }

private:
    int worldId;
    ZwiftRequest request;
};

class PlayerStateWrapper {
public:
    
    enum TURN_SIGNALS {
        RIGHT = 'right',
        LEFT = 'left',
        STRAIGHT = 'straight'
    };

    //PlayerStateWrapper(const zwift_messages::PlayerState& playerState) : playerState(playerState) {}

    int getRideOns() {
        return (playerState.at(19) >> 24) & 0xfff;
    }

    bool isTurning() {
        return (playerState.at(19) & 8) != 0;
    }

    bool isForward() {
        return (playerState.at(19) & 4) != 0;
    }

    int getCourse() {
        return (playerState.at(19) & 0xff0000) >> 16;
    }

    int getWorld() {
        return COURSE_TO_WORLD[getCourse()];
    }

    int getRoadId() {
        return (playerState.at(20) & 0xff00) >> 8;
    }

    int getRoadDirection() {
        return (playerState.at(20) & 0xffff000000) >> 24;
    }

    TURN_SIGNALS getTurnSignal() {
        int signalCode = playerState.at(20) & 0x70;
        if (signalCode == 0x10) {
            return RIGHT;
        } else if (signalCode == 0x20) {
            return LEFT;
        } else if (signalCode == 0x40) {
            return STRAIGHT;
        } else {
            return STRAIGHT;
        }
    }

    int getPowerUp() {
        return playerState.at(20) & 0xf;
    }

    bool hasFeatherBoost() {
        return getPowerUp() == 0;
    }

    bool hasDraftBoost() {
        return getPowerUp() == 1;
    }

    bool hasAeroBoost() {
        return getPowerUp() == 5;
    }

    int getCadence() {
        //return static_cast<int>((playerState.cadenceuhz() * 60) / 1000000);
    }

    std::string operator()(const std::string& item) {
        try {
            //return playerState.GetReflection()->GetString(playerState, playerState.GetDescriptor()->FindFieldByName(item));
        } catch (const std::exception& e) {
            qDebug() << "Error: " << e.what();
            return "";
        }
    }

private:
    
    QMap<int, int> COURSE_TO_WORLD = {{3, 1}, {4, 2}, {5, 3}, {6, 1}};

    enum COURSES {
        WATOPIA = 3,
        RICHMOND = 4,
        LONDON = 5
    };

    //zwift_messages::PlayerState playerState;
    QByteArray playerState;
};

#endif // PLAYERSTATEWRAPPER_H
