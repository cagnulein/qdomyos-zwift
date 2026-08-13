#ifndef FITSHOWROWER_H
#define FITSHOWROWER_H

#include "devices/rower.h"

#include <QBluetoothDeviceInfo>
#include <QDateTime>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QTimer>

class fitshowrower : public rower {
    Q_OBJECT

  public:
    struct Packet {
        bool valid = false;
        quint8 command = 0;
        quint8 subcommand = 0;
        quint8 status = 0;
        quint16 elapsedSeconds = 0;
        quint16 distanceMeters = 0;
        double calories = 0;
        quint16 strokeCount = 0;
        quint16 cadence = 0;
        double power = 0;
        quint8 heartRate = 0;
        quint8 maxResistance = 0;
        quint8 maxIncline = 0;
    };

    fitshowrower(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    resistance_t maxResistance() override { return 0; }
    static Packet parsePacket(const QByteArray &frame);
    static bool validFrame(const QByteArray &frame);
    static bool isTopiomDeviceName(const QString &name);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  signals:
    void disconnected();
    void debug(QString string);

  private slots:
    void update();
    void serviceDiscovered(const QBluetoothUuid &service);
    void serviceScanDone();
    void serviceStateChanged(QLowEnergyService::ServiceState state);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void controllerError(QLowEnergyController::Error error);
    void serviceError(QLowEnergyService::ServiceError error);

  private:
    void writeFrame(const QByteArray &frame, const QString &description);
    void applyPacket(const Packet &packet);
    void createVirtualDevice();

    QTimer refresh;
    QLowEnergyService *communicationService = nullptr;
    QLowEnergyCharacteristic writeCharacteristic;
    QLowEnergyCharacteristic notifyCharacteristic;
    QList<QByteArray> initializationFrames;
    int initializationIndex = 0;
    bool notificationsEnabled = false;
    bool initialized = false;
    bool pollStatus = true;
    bool noWriteResistance = false;
    bool noHeartService = false;
    QDateTime lastStroke = QDateTime::currentDateTime();
};

#endif
