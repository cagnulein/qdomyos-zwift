#ifndef COMPUTRAINERBIKE_H
#define COMPUTRAINERBIKE_H

#include <QAbstractOAuth2>
#include <QObject>

#include <QSettings>
#include <QTimer>
#include <QtCore/qbytearray.h>

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

#include "Computrainer.h"
#include "bike.h"
#include "virtualbike.h"

class computrainerbike : public bike {
    Q_OBJECT
  public:
    computrainerbike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
                     double bikeResistanceGain);
    resistance_t pelotonToBikeResistance(int pelotonResistance);
    resistance_t resistanceFromPowerRequest(uint16_t power);
    resistance_t maxResistance() { return max_resistance; }
    bool inclinationAvailableByHardware();
    bool connected();

    void *VirtualBike();
    void *VirtualDevice();

  private:
    resistance_t max_resistance = 100;
    resistance_t min_resistance = -20;
    uint16_t wattsFromResistance(resistance_t resistance);
    double GetDistanceFromPacket(QByteArray packet);
    QTime GetElapsedFromPacket(QByteArray packet);
    void btinit();
    void startDiscover();
    void sendPoll();
    uint16_t watts();
    void forceResistance(double requestResistance);
    void innerWriteResistance();

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

    Computrainer *myComputrainer = nullptr;

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:

    void update();
};

#endif // COMPUTRAINERBIKE_H
