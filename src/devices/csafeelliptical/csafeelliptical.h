/*
 * Copyright (c) 2024 Marcel Verpaalen (marcel@verpaalen.com)
 * based on csaferower
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CSAFEELLIPTICAL_H
#define CSAFEELLIPTICAL_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
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
#include <QObject>
#include <QString>

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

#include "devices/csafe/csafe.h"
#include "devices/csafe/csaferunner.h"
#include "devices/csafe/csafeutility.h"
#include "devices/csafe/kalmanfilter.h"
#include "devices/csafe/serialhandler.h"

#include "devices/elliptical.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QSettings>
#include <QThread>

// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/types.h>

/**
 * @brief This class is a CSAFE implementation for elliptical devices.
 * Developed for Life Fitness 95x but most likely also working for other CSAFE devices.
 */

class csafeelliptical : public elliptical {
    Q_OBJECT
  public:
    csafeelliptical(bool noWriteResistance, bool noHeartService, bool noVirtualDevice, int8_t bikeResistanceOffset,
                    double bikeResistanceGain);
    bool connected() override;

  private:
    QTimer *refresh;
    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    int lastStatus = -1;
    QSettings settings;

    uint16_t watts() override;
    void setupCommands(CsafeRunnerThread *runner);
    void setupWorkout();

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noVirtualDevice = false;

    bool distanceIsChanging = false;
    metric distanceReceived;
    KalmanFilter *kalman; 
    KalmanFilter *kalman1; 
    KalmanFilter *kalman2; 
    KalmanFilter *kalman3; 
    KalmanFilter *kalman4;
    bool _connected = true;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void sendCsafeCommand(const QStringList &commands);

  private slots:
    void update();
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void changeInclinationRequested(double grade, double percentage);
    void changeResistance(resistance_t res) override;
    void onPower(double power);
    void onCadence(double cadence);
    void onHeart(double hr);
    void onCalories(double calories);
    void onDistance(double distance);
    void onStatus(char status);
    void onSpeed(double speed);
    void portAvailable(bool available);
    void onCsafeFrame(const QVariantMap &frame);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
};

#endif // CSAFEELLIPTICAL_H
