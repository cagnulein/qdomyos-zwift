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

#ifndef _KettlerJoystick_h
#define _KettlerJoystick_h 1

#include <QDebug>
#include <QMutex>
#include <QString>
#include <QThread>
#include <QObject>
#include <QDateTime>

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

/* Kettler Joystick Constants */

/* Button mapping to modem control lines */
#define KETTLER_JOY_LEFT_ARROW  0  // DCD signal
#define KETTLER_JOY_RIGHT_ARROW 1  // CTS signal
#define KETTLER_JOY_DOWN_ARROW  2  // DSR signal
#define KETTLER_JOY_UP_ARROW    3  // RING signal

/* Poll interval in milliseconds */
#define KETTLER_JOY_POLL_INTERVAL 50

/* Debounce time in milliseconds */
#define KETTLER_JOY_DEBOUNCE_MS 200

class KettlerJoystick : public QThread {
    Q_OBJECT

  public:
    KettlerJoystick(QObject *parent = 0, QString deviceFilename = 0);
    ~KettlerJoystick();

    QObject *parent;

    // HIGH-LEVEL FUNCTIONS
    int start();                           // Calls QThread to start
    int stop();                            // stops monitoring thread
    int quit(int error);                   // called by thread before exiting
    bool discover(QString deviceFilename); // confirm Joystick is attached

    // SET
    void setDevice(QString deviceFilename); // setup the device filename

  signals:
    void buttonPressed(int button); // Emitted when a button is pressed

  private:
    void run() override; // called by start to kick off the monitoring thread

    // Utility functions
    int openPort();
    int closePort();
    void checkButtonStates(); // Check modem control line states

    // Mutex for controlling access to private data
    QMutex pvars;

    // Button state tracking for edge detection
    bool lastButtonStates[4] = {false, false, false, false};
    QDateTime lastButtonPress[4];

    // device port
    QString deviceFilename;
#ifdef WIN32
    HANDLE devicePort;
    DCB deviceSettings;
#else
    int devicePort;
    struct termios deviceSettings;
#endif

    // Thread control
    volatile bool shouldStop = false;
};

#endif // _KettlerJoystick_h
