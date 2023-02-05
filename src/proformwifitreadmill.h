#ifndef PROFORMWIFITREADMILL_H
#define PROFORMWIFITREADMILL_H

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

#include "treadmill.h"
#include "virtualbike.h"
#include "virtualtreadmill.h"

class proformwifitreadmill : public treadmill {
    Q_OBJECT
  public:
    proformwifitreadmill(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
                         double bikeResistanceGain);
    bool connected();

    void *VirtualTreadMill();
    void *VirtualBike();
    void *VirtualDevice();
    virtual bool canStartStop() { return false; }

  private:
    QWebSocket websocket;
    void connectToDevice();
    double GetDistanceFromPacket(QByteArray packet);
    QTime GetElapsedFromPacket(QByteArray packet);
    void btinit();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    void startDiscover();
    void sendPoll();
    uint16_t watts();

    QTimer *refresh;
    virtualtreadmill *virtualTreadMill = nullptr;
    virtualbike *virtualBike = nullptr;
    uint8_t counterPoll = 0;
    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    uint8_t sec1Update = 0;
    QString lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    
    uint16_t m_watts = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:

    void characteristicChanged(const QString &message);
    void binaryMessageReceived(const QByteArray &message);

    void changeInclinationRequested(double grade, double percentage);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void update();
};

#endif // PROFORMWIFITREADMILL_H
