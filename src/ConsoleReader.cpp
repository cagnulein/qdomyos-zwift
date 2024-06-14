//#if defined(Q_OS_LINUX)
#if 1
#include "ConsoleReader.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static struct termios oldSettings;
static struct termios newSettings;

/* Initialize new terminal i/o settings */
void initTermios(int echo) {
    tcgetattr(0, &oldSettings);                 /* grab old terminal i/o settings */
    newSettings = oldSettings;                  /* make new settings same as old settings */
    newSettings.c_lflag &= ~ICANON;             /* disable buffered i/o */
    newSettings.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &newSettings);        /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) { tcsetattr(0, TCSANOW, &oldSettings); }

/* Read 1 character without echo */
char getch(void) { return getchar(); }

ConsoleReader::ConsoleReader(bluetooth *bt) {
    bluetoothManager = bt;
    initTermios(0);
}

ConsoleReader::~ConsoleReader() { resetTermios(); }

void ConsoleReader::run() {
    forever {
        char key = getch();
        qDebug() << "key pressed" << key;
        if (key == 'q') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                ((treadmill *)bluetoothManager->device())->changeSpeed(speed + 0.5);
            }
        } else if (key == 'w') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                ((treadmill *)bluetoothManager->device())->changeSpeed(speed - 0.5);
            }
        } else if (key == 'a') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double inclination = ((treadmill *)bluetoothManager->device())->currentInclination().value();
                ((treadmill *)bluetoothManager->device())->changeInclination(inclination + 0.5, inclination + 0.5);
            }
        } else if (key == 's') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double inclination = ((treadmill *)bluetoothManager->device())->currentInclination().value();
                ((treadmill *)bluetoothManager->device())->changeInclination(inclination - 0.5, inclination - 0.5);
            }
        } else if (key == '1') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                ((treadmill *)bluetoothManager->device())->changeSpeed(5);
            }
        } else if (key == '2') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double inclination = ((treadmill *)bluetoothManager->device())->currentInclination().value();
                ((treadmill *)bluetoothManager->device())->changeInclination(inclination + 0.5, inclination + 0.5);
            }
        } else if (key == '3') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                ((treadmill *)bluetoothManager->device())->changeSpeed(10);
            }
        } else if (key == '4') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                ((treadmill *)bluetoothManager->device())->changeSpeed(speed - 0.5);
            }
        } else if (key == '5') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double inclination = ((treadmill *)bluetoothManager->device())->currentInclination().value();
                ((treadmill *)bluetoothManager->device())->changeInclination(inclination - 0.5, inclination - 0.5);
            }
        } else if (key == '6') {
            if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                ((treadmill *)bluetoothManager->device())->changeSpeed(speed + 0.5);
            }
        }
        emit KeyPressed(key);
    }
}
#endif
