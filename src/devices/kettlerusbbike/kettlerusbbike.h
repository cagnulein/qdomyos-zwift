/*
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

#ifndef KETTLERUSBBIKE_H
#define KETTLERUSBBIKE_H

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

#include "KettlerUSB.h"
#include "devices/bike.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class kettlerusbbike : public bike {
    Q_OBJECT
  public:
    kettlerusbbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                   double bikeResistanceGain);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    resistance_t maxResistance() override { return max_resistance; }
    bool connected() override;

    // Sim mode support: override inclination methods to use power-based simulation
    void changeInclination(double grade, double percentage) override;
    void forceInclination(double inclination);

  protected:
    // KettlerUSB doesn't support native inclination, use sim mode instead
    bool supportsNativeInclination() const override { return false; }

  private:
    resistance_t max_resistance = 250; // Max watts
    resistance_t min_resistance = 0;
    uint16_t wattsFromResistance(resistance_t resistance);
    void btinit();
    uint16_t watts() override;
    void forceResistance(double requestResistance);
    void innerWriteResistance();

    QTimer *refresh;
    virtualbike *virtualBike = nullptr;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    uint8_t sec1Update = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    metric target_watts;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    KettlerUSB *myKettler = nullptr;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void update();
};

#endif // KETTLERUSBBIKE_H
