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

#include "MapCoordinate.h"
#include "ZwiftWorldConstants.h"

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
    World(int worldId, const QString& getAccessToken) : worldId(worldId), request(getAccessToken) {}

    MapCoordinate* ToMapCoordinate(int worldId, double Longitude, double Latitude, double Altitude)
    {
        ZwiftWorldConstants* worldConstants;

        switch (worldId)
        {
            case _Watopia:
                worldConstants = Watopia;
                break;
            case _MakuriIslands:
                worldConstants = MakuriIslands;
                break;
            case _Richmond:
                worldConstants = Richmond;
                break;
            case _London:
                worldConstants = London;
                break;
            case _NewYork:
                worldConstants = NewYork;
                break;
            case _Innsbruck:
                worldConstants = Innsbruck;
                break;
            case _Bologna:
                worldConstants = Bologna;
                break;
            case _Yorkshire:
                worldConstants = Yorkshire;
                break;
            case _CritCity:
                worldConstants = CritCity;
                break;
            case _France:
                worldConstants = France;
                break;
            case _Paris:
                worldConstants = Paris;
                break;
            default:
                return new MapCoordinate(0,0,0,0);
        }

        // NOTE: The coordinates in Zwift itself are flipped which
        //       is why you see longitude used to calculate latitude
        //       and negative latitude to calculate longitude.
        double latitudeAsCentimetersFromOrigin = (Longitude * worldConstants->MetersBetweenLatitudeDegree * 100);
        double latitudeOffsetCentimeters = latitudeAsCentimetersFromOrigin - worldConstants->CenterLatitudeFromOrigin;

        double longitudeAsCentimetersFromOrigin = -Latitude * worldConstants->MetersBetweenLongitudeDegree * 100;
        double longitudeOffsetCentimeters = longitudeAsCentimetersFromOrigin - worldConstants->CenterLongitudeFromOrigin;

        return new MapCoordinate(latitudeOffsetCentimeters, longitudeOffsetCentimeters, Altitude, worldId);
    }

    QString getPlayers() {
        return request.json("/relay/worlds/" + QString::number(worldId));
    }

    QByteArray playerStatus(int playerId) {
        QByteArray buffer = request.protobuf("/relay/worlds/" + QString::number(worldId) + "/players/" + QString::number(playerId));
        return buffer;
    }

    QString player_id() {
        return request.json("/api/profiles/me");
    }

private:
    int worldId;
    ZwiftRequest request;

   enum ZwiftWorldId
    {
        _Unknown = -1,
        _Watopia = 1,
        _Richmond = 2,
        _London = 3,
        _NewYork = 4,
        _Innsbruck = 5,
        _Bologna = 6,
        _Yorkshire = 7,
        _CritCity = 8,
        _MakuriIslands = 9,
        _France = 10,
        _Paris = 11
    };

    // https://github.com/sandermvanvliet/RoadCaptain/blob/d8ec891349212d2a8ef2691925376712680e0bc4/src/RoadCaptain/TrackPoint.cs#L256
    ZwiftWorldConstants* Watopia = new ZwiftWorldConstants(110614.71, 109287.52, -11.644904f, 166.95293);
    ZwiftWorldConstants* Richmond = new ZwiftWorldConstants(110987.82, 88374.68, 37.543f, -77.4374f);
    ZwiftWorldConstants* London = new ZwiftWorldConstants(111258.3, 69400.28, 51.501705f, -0.16794094f);
    ZwiftWorldConstants* NewYork = new ZwiftWorldConstants(110850.0, 84471.0, 40.76723f, -73.97667f);
    ZwiftWorldConstants* Innsbruck = new ZwiftWorldConstants(111230.0, 75027.0, 47.2728f, 11.39574f);
    ZwiftWorldConstants* Bologna = new ZwiftWorldConstants(111230.0, 79341.0, 44.49477f, 11.34324f);
    ZwiftWorldConstants* Yorkshire = new ZwiftWorldConstants(111230.0, 65393.0, 53.991127f, -1.541751f);
    ZwiftWorldConstants* CritCity = new ZwiftWorldConstants(110614.71, 109287.52, -10.3844f, 165.8011f);
    ZwiftWorldConstants* MakuriIslands = new ZwiftWorldConstants(110614.71, 109287.52, -10.749806f, 165.83644f);
    ZwiftWorldConstants* France = new ZwiftWorldConstants(110726.0, 103481.0, -21.695074f, 166.19745f);
    ZwiftWorldConstants* Paris = new ZwiftWorldConstants(111230.0, 73167.0, 48.86763f, 2.31413f);    
};

class PlayerStateWrapper {
public:
    
    enum TURN_SIGNALS {
        RIGHT = 0,
        LEFT = 1,
        STRAIGHT = 2
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
