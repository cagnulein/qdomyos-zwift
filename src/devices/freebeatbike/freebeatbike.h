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

#ifndef FREEBEATBIKE_H
#define FREEBEATBIKE_H

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

#include "FreebeatUSB.h"
#include "devices/bike.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class freebeatbike : public bike {
    Q_OBJECT
  public:
    freebeatbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                 double bikeResistanceGain);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    resistance_t maxResistance() override { return max_resistance; }
    bool connected() override;

  private:
    resistance_t max_resistance = 100;
    resistance_t min_resistance = 1;
    void btinit();
    uint16_t watts() override;
    void forceResistance(resistance_t requestResistance);
    void innerWriteResistance();

    QTimer *refresh;
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

    FreebeatUSB *myFreebeat = nullptr;

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

#endif // FREEBEATBIKE_H
