#ifndef SPORTSTECHBIKE_SERIAL_H
#define SPORTSTECHBIKE_SERIAL_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QtCore/qbytearray.h>
#include <QtCore/qtimer.h>

#include <QObject>
#include <QTime>

#include "devices/bike.h"

class sportstechbike_serial : public bike {
    Q_OBJECT
  public:
    sportstechbike_serial(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                          double bikeResistanceGain);
    bool connected() override;
    resistance_t maxResistance() override;
    resistance_t resistanceFromPowerRequest(uint16_t power) override;

  private:
    double GetSpeedFromPacket(const QByteArray &packet);
    double GetResistanceFromPacket(const QByteArray &packet);
    double GetKcalFromPacket(const QByteArray &packet);
    uint16_t GetElapsedFromPacket(const QByteArray &packet);
    uint16_t wattsFromResistance(double resistance);
    void btinit(bool startTape);
    void writeCharacteristic(const uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                             bool wait_for_response);
    uint16_t watts() override;
    double GetWattFromPacket(const QByteArray &packet);
    double GetCadenceFromPacket(const QByteArray &packet);
    void processIncomingData(const QByteArray &bytes);
    void processFrame(const QByteArray &frame);
    void pollSerialInput();
    bool openSerial();

    QTimer *refresh = nullptr;
    bool noWriteResistance = false;
    bool noHeartService = false;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    bool serialOpen = false;
    bool firstCharChanged = true;
    QTime lastTimeCharChanged;
    uint8_t sec1update = 0;
    QByteArray lastPacket;
    QByteArray serialRxBuffer;
    bool initDone = false;
    bool initRequest = false;
    bool packetReceivedSinceWrite = false;

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void update();
};

#endif // SPORTSTECHBIKE_SERIAL_H
