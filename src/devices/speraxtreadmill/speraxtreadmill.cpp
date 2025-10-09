#include "speraxtreadmill.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

speraxtreadmill::speraxtreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                   double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0) {
        lastSpeed = forceInitSpeed;
    }

    if (forceInitInclination > 0) {
        lastInclination = forceInitInclination;
    }

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &speraxtreadmill::update);
    refresh->start(pollDeviceTime);
}

void speraxtreadmill::writeCharacteristic(const QLowEnergyCharacteristic characteristic, uint8_t *data,
                                           uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &speraxtreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        emit debug(QStringLiteral("writeCharacteristic error because the connection is closed"));

        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(characteristic, *writeBuffer, QLowEnergyService::WriteWithoutResponse);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void speraxtreadmill::forceSpeed(double requestSpeed) {
    // Convert km/h to internal speed value (multiply by 10 and convert to integer)
    int speedValue = static_cast<int>(requestSpeed * 10);

    uint8_t* writeSpeed = nullptr;
    size_t frameSize = 0;

    switch (speedValue) {
    case 1: {
        // Frame from pkt6905 (after discarding first 12 bytes)
        static uint8_t frame0[] = {0xf5, 0x0b, 0x00, 0x15, 0x02, 0x01, 0x00, 0x56, 0xf0, 0x00, 0xfa};
        writeSpeed = frame0;
        frameSize = sizeof(frame0);
        break;
    }
    case 2: {
        // Frame from pkt6981 (after discarding first 12 bytes)
        static uint8_t frame2[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x02, 0x00, 0xbe, 0x98, 0xfa};
        writeSpeed = frame2;
        frameSize = sizeof(frame2);
        break;
    }
    case 3: {
        // Frame from pkt6986 (after discarding first 12 bytes)
        static uint8_t frame3[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x03, 0x00, 0x2b, 0x71, 0xfa};
        writeSpeed = frame3;
        frameSize = sizeof(frame3);
        break;
    }
    case 4: {
        // Frame from pkt7310 (after discarding first 12 bytes)
        static uint8_t frame4[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x04, 0x00, 0x5e, 0x61, 0xfa};
        writeSpeed = frame4;
        frameSize = sizeof(frame4);
        break;
    }
    case 5: {
        // Frame from pkt7442 (after discarding first 12 bytes)
        static uint8_t frame5[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x05, 0x00, 0xcb, 0x88, 0xfa};
        writeSpeed = frame5;
        frameSize = sizeof(frame5);
        break;
    }
    case 6: {
        // Frame from pkt7585 (after discarding first 12 bytes)
        static uint8_t frame6[] = {0xf5, 0x0b, 0x00, 0x15, 0x01, 0x06, 0x00, 0x3b, 0xf0, 0x04, 0xfa};
        writeSpeed = frame6;
        frameSize = sizeof(frame6);
        break;
    }
    case 7: {
        // Frame from pkt7695 (after discarding first 12 bytes)
        static uint8_t frame7[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x07, 0x00, 0xae, 0x1d, 0xfa};
        writeSpeed = frame7;
        frameSize = sizeof(frame7);
        break;
    }
    case 8: {
        // Frame from pkt7807 (after discarding first 12 bytes)
        static uint8_t frame8[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x08, 0x00, 0xd1, 0xd4, 0xfa};
        writeSpeed = frame8;
        frameSize = sizeof(frame8);
        break;
    }
    case 9: {
        // Frame from pkt7964 (after discarding first 12 bytes)
        static uint8_t frame9[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x09, 0x00, 0x44, 0x3d, 0xfa};
        writeSpeed = frame9;
        frameSize = sizeof(frame9);
        break;
    }
    case 10: {
        // Frame from pkt8084 (after discarding first 12 bytes)
        static uint8_t frame10[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x0a, 0x00, 0xb4, 0x41, 0xfa};
        writeSpeed = frame10;
        frameSize = sizeof(frame10);
        break;
    }
    case 11: {
        // Frame from pkt8205 (after discarding first 12 bytes)
        static uint8_t frame11[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x0b, 0x00, 0x21, 0xa8, 0xfa};
        writeSpeed = frame11;
        frameSize = sizeof(frame11);
        break;
    }
    case 12: {
        // Frame from pkt8341 (after discarding first 12 bytes)
        static uint8_t frame12[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x0c, 0x00, 0x54, 0xb8, 0xfa};
        writeSpeed = frame12;
        frameSize = sizeof(frame12);
        break;
    }
    case 13: {
        // Frame from pkt8460 (after discarding first 12 bytes)
        static uint8_t frame13[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x0d, 0x00, 0xc1, 0x51, 0xfa};
        writeSpeed = frame13;
        frameSize = sizeof(frame13);
        break;
    }
    case 14: {
        // Frame from pkt8589 (after discarding first 12 bytes)
        static uint8_t frame14[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x0e, 0x00, 0x31, 0x2d, 0xfa};
        writeSpeed = frame14;
        frameSize = sizeof(frame14);
        break;
    }
    case 15: {
        // Frame from pkt8715 (after discarding first 12 bytes)
        static uint8_t frame15[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x0f, 0x00, 0xa4, 0xc4, 0xfa};
        writeSpeed = frame15;
        frameSize = sizeof(frame15);
        break;
    }
    case 16: {
        // Frame from pkt8850 (after discarding first 12 bytes)
        static uint8_t frame16[] = {0xf5, 0x0b, 0x00, 0x15, 0x01, 0x10, 0x00, 0x80, 0xf0, 0x09, 0xfa};
        writeSpeed = frame16;
        frameSize = sizeof(frame16);
        break;
    }
    case 17: {
        // Frame from pkt8977 (after discarding first 12 bytes)
        static uint8_t frame17[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x11, 0x00, 0x15, 0x10, 0xfa};
        writeSpeed = frame17;
        frameSize = sizeof(frame17);
        break;
    }
    case 18: {
        // Frame from pkt9098 (after discarding first 12 bytes)
        static uint8_t frame18[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x12, 0x00, 0xe5, 0x6c, 0xfa};
        writeSpeed = frame18;
        frameSize = sizeof(frame18);
        break;
    }
    case 19: {
        // Frame from pkt9202 (after discarding first 12 bytes)
        static uint8_t frame19[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x13, 0x00, 0x70, 0x85, 0xfa};
        writeSpeed = frame19;
        frameSize = sizeof(frame19);
        break;
    }
    case 20: {
        // Frame from pkt9387 (after discarding first 12 bytes)
        static uint8_t frame20[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x14, 0x00, 0x05, 0x95, 0xfa};
        writeSpeed = frame20;
        frameSize = sizeof(frame20);
        break;
    }
    case 21: {
        // Frame from pkt9622 (after discarding first 12 bytes)
        static uint8_t frame21[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x15, 0x00, 0x90, 0x7c, 0xfa};
        writeSpeed = frame21;
        frameSize = sizeof(frame21);
        break;
    }
    case 22: {
        // Frame from pkt9711 (after discarding first 12 bytes)
        static uint8_t frame22[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x16, 0x00, 0x60, 0x00, 0xfa};
        writeSpeed = frame22;
        frameSize = sizeof(frame22);
        break;
    }
    case 23: {
        // Frame from pkt9804 (after discarding first 12 bytes)
        static uint8_t frame23[] = {0xf5, 0x0b, 0x00, 0x15, 0x01, 0x17, 0x00, 0xf0, 0x05, 0xe9, 0xfa};
        writeSpeed = frame23;
        frameSize = sizeof(frame23);
        break;
    }
    case 24: {
        // Frame from pkt9876 (after discarding first 12 bytes)
        static uint8_t frame24[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x18, 0x00, 0x8a, 0x20, 0xfa};
        writeSpeed = frame24;
        frameSize = sizeof(frame24);
        break;
    }
    case 25: {
        // Frame from pkt9953 (after discarding first 12 bytes)
        static uint8_t frame25[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x19, 0x00, 0x1f, 0xc9, 0xfa};
        writeSpeed = frame25;
        frameSize = sizeof(frame25);
        break;
    }
    case 26: {
        // Frame from pkt10062 (after discarding first 12 bytes)
        static uint8_t frame26[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x1a, 0x00, 0xef, 0xb5, 0xfa};
        writeSpeed = frame26;
        frameSize = sizeof(frame26);
        break;
    }
    case 27: {
        // Frame from pkt10164 (after discarding first 12 bytes)
        static uint8_t frame27[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x1b, 0x00, 0x7a, 0x5c, 0xfa};
        writeSpeed = frame27;
        frameSize = sizeof(frame27);
        break;
    }
    case 28: {
        // Frame from pkt10292 (after discarding first 12 bytes)
        static uint8_t frame28[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x1c, 0x00, 0x0f, 0x4c, 0xfa};
        writeSpeed = frame28;
        frameSize = sizeof(frame28);
        break;
    }
    case 29: {
        // Frame from pkt10401 (after discarding first 12 bytes)
        static uint8_t frame29[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x1d, 0x00, 0x9a, 0xa5, 0xfa};
        writeSpeed = frame29;
        frameSize = sizeof(frame29);
        break;
    }
    case 30: {
        // Frame from pkt10515 (after discarding first 12 bytes)
        static uint8_t frame30[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x1e, 0x00, 0x6a, 0xd9, 0xfa};
        writeSpeed = frame30;
        frameSize = sizeof(frame30);
        break;
    }
    case 31: {
        // Frame from pkt10668 (after discarding first 12 bytes)
        static uint8_t frame31[] = {0xf5, 0x0b, 0x00, 0x15, 0x01, 0x1f, 0x00, 0xf0, 0x0f, 0x30, 0xfa};
        writeSpeed = frame31;
        frameSize = sizeof(frame31);
        break;
    }
    case 32: {
        // Frame from pkt10756 (after discarding first 12 bytes)
        static uint8_t frame32[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x20, 0x00, 0x22, 0xa3, 0xfa};
        writeSpeed = frame32;
        frameSize = sizeof(frame32);
        break;
    }
    case 33: {
        // Frame from pkt10815 (after discarding first 12 bytes)
        static uint8_t frame33[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x21, 0x00, 0xb7, 0x4a, 0xfa};
        writeSpeed = frame33;
        frameSize = sizeof(frame33);
        break;
    }
    case 34: {
        // Frame from pkt10876 (after discarding first 12 bytes)
        static uint8_t frame34[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x22, 0x00, 0x47, 0x36, 0xfa};
        writeSpeed = frame34;
        frameSize = sizeof(frame34);
        break;
    }
    case 35: {
        // Frame from pkt10950 (after discarding first 12 bytes)
        static uint8_t frame35[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x23, 0x00, 0xd2, 0xdf, 0xfa};
        writeSpeed = frame35;
        frameSize = sizeof(frame35);
        break;
    }
    case 36: {
        // Frame from pkt10983 (after discarding first 12 bytes)
        static uint8_t frame36[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x24, 0x00, 0xa7, 0xcf, 0xfa};
        writeSpeed = frame36;
        frameSize = sizeof(frame36);
        break;
    }
    case 37: {
        // Frame from pkt11032 (after discarding first 12 bytes)
        static uint8_t frame37[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x25, 0x00, 0x32, 0x26, 0xfa};
        writeSpeed = frame37;
        frameSize = sizeof(frame37);
        break;
    }
    case 38: {
        // Frame from pkt11076 (after discarding first 12 bytes)
        static uint8_t frame38[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x26, 0x00, 0xc2, 0x5a, 0xfa};
        writeSpeed = frame38;
        frameSize = sizeof(frame38);
        break;
    }
    case 39: {
        // Frame from pkt11122 (after discarding first 12 bytes)
        static uint8_t frame39[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x27, 0x00, 0x57, 0xb3, 0xfa};
        writeSpeed = frame39;
        frameSize = sizeof(frame39);
        break;
    }
    case 40: {
        // Frame from pkt11163 (after discarding first 12 bytes)
        static uint8_t frame40[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x28, 0x00, 0x28, 0x7a, 0xfa};
        writeSpeed = frame40;
        frameSize = sizeof(frame40);
        break;
    }
    case 41: {
        // Frame from pkt11225 (after discarding first 12 bytes)
        static uint8_t frame41[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x29, 0x00, 0xbd, 0x93, 0xfa};
        writeSpeed = frame41;
        frameSize = sizeof(frame41);
        break;
    }
    case 42: {
        // Frame from pkt11273 (after discarding first 12 bytes)
        static uint8_t frame42[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x2a, 0x00, 0x4d, 0xef, 0xfa};
        writeSpeed = frame42;
        frameSize = sizeof(frame42);
        break;
    }
    case 43: {
        // Frame from pkt11330 (after discarding first 12 bytes)
        static uint8_t frame43[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x2b, 0x00, 0xd8, 0x06, 0xfa};
        writeSpeed = frame43;
        frameSize = sizeof(frame43);
        break;
    }
    case 44: {
        // Frame from pkt11385 (after discarding first 12 bytes)
        static uint8_t frame44[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x2c, 0x00, 0xad, 0x16, 0xfa};
        writeSpeed = frame44;
        frameSize = sizeof(frame44);
        break;
    }
    case 45: {
        // Frame from pkt11435 (after discarding first 12 bytes)
        static uint8_t frame45[] = {0xf5, 0x0b, 0x00, 0x15, 0x01, 0x2d, 0x00, 0x38, 0xf0, 0x0f, 0xfa};
        writeSpeed = frame45;
        frameSize = sizeof(frame45);
        break;
    }
    case 46: {
        // Frame from pkt11483 (after discarding first 12 bytes)
        static uint8_t frame46[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x2e, 0x00, 0xc8, 0x83, 0xfa};
        writeSpeed = frame46;
        frameSize = sizeof(frame46);
        break;
    }
    case 47: {
        // Frame from pkt11541 (after discarding first 12 bytes)
        static uint8_t frame47[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x2f, 0x00, 0x5d, 0x6a, 0xfa};
        writeSpeed = frame47;
        frameSize = sizeof(frame47);
        break;
    }
    case 48: {
        // Frame from pkt11591 (after discarding first 12 bytes)
        static uint8_t frame48[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x30, 0x00, 0x79, 0x57, 0xfa};
        writeSpeed = frame48;
        frameSize = sizeof(frame48);
        break;
    }
    case 49: {
        // Frame from pkt11631 (after discarding first 12 bytes)
        static uint8_t frame49[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x31, 0x00, 0xec, 0xbe, 0xfa};
        writeSpeed = frame49;
        frameSize = sizeof(frame49);
        break;
    }
    case 50: {
        // Frame from pkt11698 (after discarding first 12 bytes)
        static uint8_t frame50[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x32, 0x00, 0x1c, 0xc2, 0xfa};
        writeSpeed = frame50;
        frameSize = sizeof(frame50);
        break;
    }
    case 51: {
        // Frame from pkt11777 (after discarding first 12 bytes)
        static uint8_t frame51[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x33, 0x00, 0x89, 0x2b, 0xfa};
        writeSpeed = frame51;
        frameSize = sizeof(frame51);
        break;
    }
    case 52: {
        // Frame from pkt11856 (after discarding first 12 bytes)
        static uint8_t frame52[] = {0xf5, 0x0b, 0x00, 0x15, 0x01, 0x34, 0x00, 0xf0, 0x0c, 0x3b, 0xfa};
        writeSpeed = frame52;
        frameSize = sizeof(frame52);
        break;
    }
    case 53: {
        // Frame from pkt11922 (after discarding first 12 bytes)
        static uint8_t frame53[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x35, 0x00, 0x69, 0xd2, 0xfa};
        writeSpeed = frame53;
        frameSize = sizeof(frame53);
        break;
    }
    case 54: {
        // Frame from pkt11979 (after discarding first 12 bytes)
        static uint8_t frame54[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x36, 0x00, 0x99, 0xae, 0xfa};
        writeSpeed = frame54;
        frameSize = sizeof(frame54);
        break;
    }
    case 55: {
        // Frame from pkt12042 (after discarding first 12 bytes)
        static uint8_t frame55[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x37, 0x00, 0x0c, 0x47, 0xfa};
        writeSpeed = frame55;
        frameSize = sizeof(frame55);
        break;
    }
    case 56: {
        // Frame from pkt12093 (after discarding first 12 bytes)
        static uint8_t frame56[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x38, 0x00, 0x73, 0x8e, 0xfa};
        writeSpeed = frame56;
        frameSize = sizeof(frame56);
        break;
    }
    case 57: {
        // Frame from pkt12163 (after discarding first 12 bytes)
        static uint8_t frame57[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x39, 0x00, 0xe6, 0x67, 0xfa};
        writeSpeed = frame57;
        frameSize = sizeof(frame57);
        break;
    }
    case 58: {
        // Frame from pkt12216 (after discarding first 12 bytes)
        static uint8_t frame58[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x3a, 0x00, 0x16, 0x1b, 0xfa};
        writeSpeed = frame58;
        frameSize = sizeof(frame58);
        break;
    }
    case 59: {
        // Frame from pkt12268 (after discarding first 12 bytes)
        static uint8_t frame59[] = {0xf5, 0x0b, 0x00, 0x15, 0x01, 0x3b, 0x00, 0x83, 0xf0, 0x02, 0xfa};
        writeSpeed = frame59;
        frameSize = sizeof(frame59);
        break;
    }
    case 60: {
        // Frame from pkt12316 (after discarding first 12 bytes)
        static uint8_t frame60[] = {0xf5, 0x0b, 0x00, 0x15, 0x01, 0x3c, 0x00, 0xf0, 0x06, 0xe2, 0xfa};
        writeSpeed = frame60;
        frameSize = sizeof(frame60);
        break;
    }
    default:
        qDebug() << "no frame to send";
        return;
    }

    // Send the appropriate frame to the treadmill
    if (writeSpeed != nullptr) {
        writeCharacteristic(gattWriteCharacteristic, writeSpeed, frameSize,
                            QStringLiteral("forceSpeed speed=") + QString::number(requestSpeed), false, false);
    }
}

void speraxtreadmill::forceIncline(double requestIncline) {
    /*uint8_t writeIncline[] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x29, 0x06};

    writeIncline[2] = requestIncline;
    writeIncline[5] += requestIncline;
    writeIncline[6] += requestIncline;

    writeCharacteristic(gattWriteCharacteristic, writeIncline, sizeof(writeIncline),
                        QStringLiteral("forceIncline incline=") + QString::number(requestIncline), false, false);*/
}

void speraxtreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void speraxtreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    } else if (/*bluetoothDevice.isValid() &&*/
               m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattWriteCharacteristic.isValid() && gattNotifyCharacteristic.isValid() && initDone) {

        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadMill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &speraxtreadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &speraxtreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadMill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &speraxtreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        // debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        {
            if (requestSpeed != -1) {
                if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                    emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                    forceSpeed(requestSpeed);
                }
                requestSpeed = -1;
            }
            else if (requestInclination != -100) {
                if (requestInclination < 0)
                    requestInclination = 0;
                if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                    requestInclination <= 15) {
                    emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                    forceIncline(requestInclination);
                }
                requestInclination = -100;
            }
            else if (requestStart != -1) {
                emit debug(QStringLiteral("starting..."));
                if (lastSpeed == 0.0) {

                    lastSpeed = 0.5;
                }

                requestStart = -1;
                emit tapeStarted();

                uint8_t start[] = {0xf5, 0x0a, 0x00, 0x15, 0x01, 0x02, 0x00, 0xbe, 0x98, 0xfa};
                writeCharacteristic(gattWriteCharacteristic, start, sizeof(start), QStringLiteral("start"), false, false);
            }
            else if (requestStop != -1) {
                emit debug(QStringLiteral("stopping... ") + (paused ? QStringLiteral("true") : QStringLiteral("false")));

                requestStop = -1;
                uint8_t stop[] = {0xf5, 0x0a, 0x00, 0x15, 0x00, 0x00, 0x00, 0xd3, 0x01, 0xfa};
                writeCharacteristic(gattWriteCharacteristic, stop, sizeof(stop), QStringLiteral("noop"), false, false);
            } else {
                uint8_t noop[] = {0xf5, 0x08, 0x00, 0x19, 0xf0, 0x0a, 0x59, 0xfa};
                writeCharacteristic(gattWriteCharacteristic, noop, sizeof(noop), QStringLiteral("noop"), false, false);

            }
            /*if (requestFanSpeed != -1) {
                emit debug(QStringLiteral("changing fan speed..."));

                sendChangeFanSpeed(requestFanSpeed);
                requestFanSpeed = -1;
            }
            if (requestIncreaseFan != -1) {
                emit debug(QStringLiteral("increasing fan speed..."));

                sendChangeFanSpeed(FanSpeed + 1);
                requestIncreaseFan = -1;
            } else if (requestDecreaseFan != -1) {
                emit debug(QStringLiteral("decreasing fan speed..."));

                sendChangeFanSpeed(FanSpeed - 1);
                requestDecreaseFan = -1;
            }*/
        }
    }
}

void speraxtreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void speraxtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    Q_UNUSED(characteristic);
    QByteArray value = newValue;
    QDateTime now = QDateTime::currentDateTime();

    emit debug(QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' '));
    emit packetReceived();

    if (newValue.length() < 24)
        return;

    lastPacket = value;
    // lastState = value.at(0);

    double speed = GetSpeedFromPacket(value);
    double incline = 0;
    StepCount = (double)((uint8_t)newValue.at(16)) + (((double)(uint8_t)newValue.at(15)) * 256.0);
    qDebug() << "Current Step: " << StepCount.value();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {

            uint8_t heart = 0;
            if (heart == 0) {
                update_hr_from_external();
            } else

                Heart = heart;
        }
    }

    if (!firstCharacteristicChanged) {
        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        Distance += ((speed / (double)3600.0) /
                     ((double)1000.0 / (double)(lastTimeCharacteristicChanged.msecsTo(now))));
        lastTimeCharacteristicChanged = now;
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current incline: ") + QString::number(incline));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    // emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    // emit debug(QStringLiteral("Current Distance: ") + QString::number(distance));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    if (Speed.value() != speed) {

        emit speedChanged(speed);
    }
    Speed = speed;
    if (Inclination.value() != incline) {

        emit inclinationChanged(0, incline);
    }
    Inclination = incline;

    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

    if (speed > 0) {

        lastSpeed = speed;
        lastInclination = incline;
    }

    firstCharacteristicChanged = false;
}

double speraxtreadmill::GetSpeedFromPacket(const QByteArray &packet) {

    uint8_t convertedData = (uint8_t)packet.at(17);
    double data = ((double)(convertedData) / 10.0);
    return data;
}

double speraxtreadmill::GetInclinationFromPacket(const QByteArray &packet) {

    return 0;
}

void speraxtreadmill::btinit(bool startTape) {
    uint8_t initData1[] = {0xf5, 0x07, 0x00, 0x01, 0x26, 0xd8, 0xfa};
    uint8_t initData2[] = {0xf5, 0x09, 0x00, 0x13, 0x01, 0x00, 0x89, 0xb8, 0xfa};

    writeCharacteristic(gattWriteCharacteristic, initData1, sizeof(initData1), QStringLiteral("init"), false, false);
    writeCharacteristic(gattWriteCharacteristic, initData1, sizeof(initData1), QStringLiteral("init"), false, false);
    writeCharacteristic(gattWriteCharacteristic, initData2, sizeof(initData2), QStringLiteral("init"), false, false);


    if (startTape) {
    }

    initDone = true;
}

double speraxtreadmill::minStepInclination() { return 1.0; }

void speraxtreadmill::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xFFF2);
    QBluetoothUuid _gattNotifyCharacteristicId((quint16)0xFFF1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));
    if (state == QLowEnergyService::RemoteServiceDiscovered) {

        // qDebug() << gattCommunicationChannelService->characteristics();
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : std::as_const(characteristics_list)) {
            qDebug() << QStringLiteral("char uuid") << c.uuid() 
                     << c.properties();
        }

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &speraxtreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &speraxtreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &speraxtreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &speraxtreadmill::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void speraxtreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void speraxtreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void speraxtreadmill::serviceScanDone(void) {
    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);
    emit debug(QStringLiteral("serviceScanDone"));

    auto services_list = m_control->services();
    emit debug("Services found:");
    for (const QBluetoothUuid &s : std::as_const(services_list)) {
        emit debug(s.toString());
    }

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if (gattCommunicationChannelService) {
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
                &speraxtreadmill::stateChanged);
        gattCommunicationChannelService->discoverDetails();
    } else {
        emit debug(QStringLiteral("error on find Service"));
    }
}

void speraxtreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("speraxtreadmill::errorService ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void speraxtreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("speraxtreadmill::error ") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void speraxtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    {

        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &speraxtreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &speraxtreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &speraxtreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &speraxtreadmill::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
                    searchStopped = false;
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("Controller connected. Search services..."));
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("LowEnergy controller disconnected"));
            searchStopped = false;
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

void speraxtreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

bool speraxtreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void speraxtreadmill::searchingStop() { searchStopped = true; }