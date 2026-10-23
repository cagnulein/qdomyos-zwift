#ifndef YPOOTREADMILL_H
#define YPOOTREADMILL_H

#include "treadmill.h"
#include "ypoom5control.h"
#include "ypoom5distance.h"
#include <QElapsedTimer>
#include <QTimer>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyService>

// YPTM-U0 and YP-M1552 share the unlock/control protocol but use different incline units.
class ypootreadmill : public treadmill {
    Q_OBJECT
  public:
    ypootreadmill(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    double minStepInclination() override { return 1.0; }
    double minStepSpeed() override { return 0.1; }
    bool autoPauseWhenSpeedIsZero() override;
    bool autoStartWhenSpeedIsGreaterThenZero() override;
    bool isYpM1552() const { return YP_M1552; }

  signals:
    void disconnected();
    void debug(QString message);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private:
    void btinit();
    void updateYpooM5Control();
    void forceSpeed(double speed);
    void forceIncline(double incline);
    void writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                             uint8_t *data, uint8_t size, QString info);
    void processTelemetry(const QByteArray &data, qint64 now);
    friend class YpooTreadmillTestPeer;

    QTimer refresh;
    QLowEnergyService *gattFTMSService = nullptr;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyCharacteristic gattWriteCharUnlock;
    bool noHeartService;
    bool YP_M1552 = false;
    bool initDone = false;
    bool initRequest = false;
    unsigned notificationSubscribed = 0;
    bool ypooM5WritePending = false;
    bool ypooM5ControlGranted = false;
    QElapsedTimer ypooM5Clock;
    YpooM5Control ypooM5Control;
    YpooM5Distance ypooM5Distance;
    qint64 lastMetricsUpdate = -1;

  private slots:
    void update();
    void serviceScanDone();
    void stateChanged(QLowEnergyService::ServiceState state);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void changeInclinationRequested(double grade, double percentage);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
};

#endif // YPOOTREADMILL_H
