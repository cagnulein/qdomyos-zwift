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

#ifndef IC15DBIKE_H
#define IC15DBIKE_H

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

#include "IC15DUART.h"
#include "devices/bike.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

// Bike class for the Inspire IC15D Android console's internal UART (issue #4888). Receive-only:
// reads RPM/level telemetry from IC15DUART, and never writes resistance or query commands to
// the hardware, since the CSX protocol's write side has not been documented/tested yet.
class ic15dbike : public bike {
    Q_OBJECT
  public:
    // Console model string exposed by Build.MODEL on the Inspire IC15D (Smile-smart
    // BX_RK3326_A11_INSPIRE_IC15D). Used to narrowly gate auto-instantiation of this device to
    // this specific console, and nothing else.
    static const QString CONSOLE_MODEL;

    // Pure comparison, unit-testable without Android/JNI: true only for an exact match of
    // CONSOLE_MODEL, so no other console/device is affected by this driver.
    static bool matchesConsoleModel(const QString &buildModel);

    // Reads android.os.Build.MODEL via JNI. Returns an empty string on non-Android platforms or
    // if the JNI call fails.
    static QString androidBuildModel();

    // Convenience wrapper: androidBuildModel() run through matchesConsoleModel().
    static bool isIC15DConsole();

    ic15dbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset, double bikeResistanceGain);
    resistance_t maxResistance() override { return max_resistance; }
    bool connected() override;

  private:
    resistance_t max_resistance = 100;
    void btinit();
    uint16_t watts() override;

    QTimer *refresh;
    virtualbike *virtualBike = nullptr;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    IC15DUART *myIC15D = nullptr;

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

#endif // IC15DBIKE_H
