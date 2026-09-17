/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef DAUMBIKE_H
#define DAUMBIKE_H

#include <QDateTime>
#include <QSettings>
#include <QTimer>

#include "DaumUSB.h"
#include "devices/bike.h"
#include "devices/ftmsbike/ftmsbike.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

// Native Daum classic ergometer implementation. Keep the Daum binary protocol
// and its serial lifecycle independent from the Kettler USB bike driver.
class daumbike : public bike {
    Q_OBJECT
  public:
    daumbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
             double bikeResistanceGain);
    ~daumbike() override;

    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    resistance_t maxResistance() override { return max_resistance; }
    bool connected() override;

    void changeInclination(double grade, double percentage) override;
    void forceInclination(double inclination);

  protected:
    bool supportsNativeInclination() const override { return false; }
    bool applyGearModifier() const override { return false; }

  private:
    resistance_t max_resistance = 800;
    resistance_t min_resistance = 25;

    void btinit();
    uint16_t watts() override;
    void forceResistance(double requestResistance);
    void innerWriteResistance();

    QTimer *refresh = nullptr;
    virtualbike *virtualBike = nullptr;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    uint8_t sec1Update = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    bool initDone = false;
    bool initRequest = false;
    bool noWriteResistance = false;
    bool noHeartService = false;
    DaumUSB *myDaum = nullptr;
    FtmsControlPointCommand m_lastFtmsCommand = FTMS_REQUEST_CONTROL;

#ifdef Q_OS_IOS
    lockscreen *h = nullptr;
#endif

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void update();
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // DAUMBIKE_H
