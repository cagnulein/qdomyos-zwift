#ifndef PROFORMWIFIBIKE_H
#define PROFORMWIFIBIKE_H

#include <QAbstractOAuth2>
#include <QObject>

#include <QNetworkAccessManager>

#include <QDesktopServices>
#include <QHttpMultiPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QSettings>
#include <QTimer>
#include <QUrlQuery>
#include <QtCore/qbytearray.h>
#include <QtWebSockets/QWebSocket>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QDateTime>
#include <QString>

#include "bike.h"
#include "virtualbike.h"

class proformwifibike : public bike {
    Q_OBJECT
  public:
    proformwifibike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
                    double bikeResistanceGain);
    resistance_t pelotonToBikeResistance(int pelotonResistance);
    resistance_t resistanceFromPowerRequest(uint16_t power);
    resistance_t maxResistance() { return max_resistance; }
    bool inclinationAvailableByHardware();
    bool connected();

    void *VirtualBike();
    void *VirtualDevice();

  private:
    QWebSocket websocket;
    resistance_t max_resistance = 100;
    resistance_t min_resistance = -20;
    void connectToDevice();
    uint16_t wattsFromResistance(resistance_t resistance);
    double GetDistanceFromPacket(QByteArray packet);
    QTime GetElapsedFromPacket(QByteArray packet);
    void btinit();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    void sendPoll();
    uint16_t watts();
    void forceResistance(double requestResistance);
    void innerWriteResistance();
    void setTargetWatts(double watts);
    void setWorkoutType(QString type);

    QTimer *refresh;
    virtualbike *virtualBike = nullptr;
    uint8_t counterPoll = 0;
    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    uint8_t sec1Update = 0;
    QString lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    
    metric target_watts;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    bool tdf2 = false;

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:

    void characteristicChanged(const QString &message);
    void binaryMessageReceived(const QByteArray &message);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void update();
};

#endif // PROFORMWIFIBIKE_H
