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

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class proformwifitreadmill : public treadmill {
    Q_OBJECT
  public:
    proformwifitreadmill(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                         double bikeResistanceGain);
    bool connected() override;
    virtual bool canStartStop() override { return false; }
    double minStepSpeed() override { return 0.1; }

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
    uint8_t counterPoll = 0;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    double max_incline_supported = 15;
    double min_incline_supported = 0;
    double maximum_kph = 999;

    uint8_t sec1Update = 0;
    QString lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    bool waitStatePkg = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

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
