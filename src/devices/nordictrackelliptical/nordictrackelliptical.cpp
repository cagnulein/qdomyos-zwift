#include "nordictrackelliptical.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>
#include <qmath.h>

using namespace std::chrono_literals;

nordictrackelliptical::nordictrackelliptical(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                             double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    QSettings settings;
    nordictrack_elliptical_c7_5 = settings.value(QZSettings::nordictrack_elliptical_c7_5, QZSettings::default_nordictrack_elliptical_c7_5).toBool();
    nordictrack_se7i = settings.value(QZSettings::nordictrack_se7i, QZSettings::default_nordictrack_se7i).toBool();

    initDone = false;
    connect(refresh, &QTimer::timeout, this, &nordictrackelliptical::update);
    refresh->start(200ms);
}

void nordictrackelliptical::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                                bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void nordictrackelliptical::forceIncline(double requestIncline) {

    const uint8_t res[] = {0xfe, 0x02, 0x0d, 0x02};
    writeCharacteristic((uint8_t *)res, sizeof(res), QStringLiteral("incline"), false, false);

    if (nordictrack_elliptical_c7_5) {
        const uint8_t inc00[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc05[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x32, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x64, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x96, 0x00, 0x00, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0xc8, 0x00, 0x00, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc25[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0xfa, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc30[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x2c, 0x01, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc35[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x5e, 0x01, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc40[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x90, 0x01, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc45[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0xc2, 0x01, 0x00, 0xd7, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc50[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0xf4, 0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc55[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x26, 0x02, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc60[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x58, 0x02, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc65[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x8a, 0x02, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc70[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0xbc, 0x02, 0x00, 0xd2, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc75[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0xee, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc80[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x20, 0x03, 0x00, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc85[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x52, 0x03, 0x00, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc90[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0x84, 0x03, 0x00, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc95[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x02, 0xb6, 0x03, 0x00, 0xcd, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc100[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0xe8, 0x03, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc105[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x1a, 0x04, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc110[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x4c, 0x04, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc115[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x7e, 0x04, 0x00, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc120[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0xb0, 0x04, 0x00, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc125[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0xe2, 0x04, 0x00, 0xfa, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc130[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x14, 0x05, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc135[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x46, 0x05, 0x00, 0x5f, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc140[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x78, 0x05, 0x00, 0x91, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc145[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0xaa, 0x05, 0x00, 0xc3, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc150[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0xdc, 0x05, 0x00, 0xf5, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc155[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x0e, 0x06, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc160[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x40, 0x06, 0x00, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc165[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x72, 0x06, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc170[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0xa4, 0x06, 0x00, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc175[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0xd6, 0x06, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc180[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x08, 0x07, 0x00, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc185[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x3a, 0x07, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc190[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x6c, 0x07, 0x00, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc195[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0x9e, 0x07, 0x00, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t inc200[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                  0x02, 0xd0, 0x07, 0x00, 0xeb, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t inc = (uint8_t)(requestIncline * 10.0);
        switch (inc) {
        case 00:
            writeCharacteristic((uint8_t *)inc00, sizeof(inc00), QStringLiteral("inc00"), false, true);
            break;
        case 05:
            writeCharacteristic((uint8_t *)inc05, sizeof(inc05), QStringLiteral("inc05"), false, true);
            break;
        case 10:
            writeCharacteristic((uint8_t *)inc10, sizeof(inc10), QStringLiteral("inc10"), false, true);
            break;
        case 15:
            writeCharacteristic((uint8_t *)inc15, sizeof(inc15), QStringLiteral("inc15"), false, true);
            break;
        case 20:
            writeCharacteristic((uint8_t *)inc20, sizeof(inc20), QStringLiteral("inc20"), false, true);
            break;
        case 25:
            writeCharacteristic((uint8_t *)inc25, sizeof(inc25), QStringLiteral("inc25"), false, true);
            break;
        case 30:
            writeCharacteristic((uint8_t *)inc30, sizeof(inc30), QStringLiteral("inc30"), false, true);
            break;
        case 35:
            writeCharacteristic((uint8_t *)inc35, sizeof(inc35), QStringLiteral("inc35"), false, true);
            break;
        case 40:
            writeCharacteristic((uint8_t *)inc40, sizeof(inc40), QStringLiteral("inc40"), false, true);
            break;
        case 45:
            writeCharacteristic((uint8_t *)inc45, sizeof(inc45), QStringLiteral("inc45"), false, true);
            break;
        case 50:
            writeCharacteristic((uint8_t *)inc50, sizeof(inc50), QStringLiteral("inc50"), false, true);
            break;
        case 55:
            writeCharacteristic((uint8_t *)inc55, sizeof(inc55), QStringLiteral("inc55"), false, true);
            break;
        case 60:
            writeCharacteristic((uint8_t *)inc60, sizeof(inc60), QStringLiteral("inc60"), false, true);
            break;
        case 65:
            writeCharacteristic((uint8_t *)inc65, sizeof(inc65), QStringLiteral("inc65"), false, true);
            break;
        case 70:
            writeCharacteristic((uint8_t *)inc70, sizeof(inc70), QStringLiteral("inc70"), false, true);
            break;
        case 75:
            writeCharacteristic((uint8_t *)inc75, sizeof(inc75), QStringLiteral("inc75"), false, true);
            break;
        case 80:
            writeCharacteristic((uint8_t *)inc80, sizeof(inc80), QStringLiteral("inc80"), false, true);
            break;
        case 85:
            writeCharacteristic((uint8_t *)inc85, sizeof(inc85), QStringLiteral("inc85"), false, true);
            break;
        case 90:
            writeCharacteristic((uint8_t *)inc90, sizeof(inc90), QStringLiteral("inc90"), false, true);
            break;
        case 95:
            writeCharacteristic((uint8_t *)inc95, sizeof(inc95), QStringLiteral("inc95"), false, true);
            break;
        case 100:
            writeCharacteristic((uint8_t *)inc100, sizeof(inc100), QStringLiteral("inc100"), false, true);
            break;
        case 105:
            writeCharacteristic((uint8_t *)inc105, sizeof(inc105), QStringLiteral("inc105"), false, true);
            break;
        case 110:
            writeCharacteristic((uint8_t *)inc110, sizeof(inc110), QStringLiteral("inc110"), false, true);
            break;
        case 115:
            writeCharacteristic((uint8_t *)inc115, sizeof(inc115), QStringLiteral("inc115"), false, true);
            break;
        case 120:
            writeCharacteristic((uint8_t *)inc120, sizeof(inc120), QStringLiteral("inc120"), false, true);
            break;
        case 125:
            writeCharacteristic((uint8_t *)inc125, sizeof(inc125), QStringLiteral("inc125"), false, true);
            break;
        case 130:
            writeCharacteristic((uint8_t *)inc130, sizeof(inc130), QStringLiteral("inc130"), false, true);
            break;
        case 135:
            writeCharacteristic((uint8_t *)inc135, sizeof(inc135), QStringLiteral("inc135"), false, true);
            break;
        case 140:
            writeCharacteristic((uint8_t *)inc140, sizeof(inc140), QStringLiteral("inc140"), false, true);
            break;
        case 145:
            writeCharacteristic((uint8_t *)inc145, sizeof(inc145), QStringLiteral("inc145"), false, true);
            break;
        case 150:
            writeCharacteristic((uint8_t *)inc150, sizeof(inc150), QStringLiteral("inc150"), false, true);
            break;
        case 155:
            writeCharacteristic((uint8_t *)inc155, sizeof(inc155), QStringLiteral("inc155"), false, true);
            break;
        case 160:
            writeCharacteristic((uint8_t *)inc160, sizeof(inc160), QStringLiteral("inc160"), false, true);
            break;
        case 165:
            writeCharacteristic((uint8_t *)inc165, sizeof(inc165), QStringLiteral("inc165"), false, true);
            break;
        case 170:
            writeCharacteristic((uint8_t *)inc170, sizeof(inc170), QStringLiteral("inc170"), false, true);
            break;
        case 175:
            writeCharacteristic((uint8_t *)inc175, sizeof(inc175), QStringLiteral("inc175"), false, true);
            break;
        case 180:
            writeCharacteristic((uint8_t *)inc180, sizeof(inc180), QStringLiteral("inc180"), false, true);
            break;
        case 185:
            writeCharacteristic((uint8_t *)inc185, sizeof(inc185), QStringLiteral("inc185"), false, true);
            break;
        case 190:
            writeCharacteristic((uint8_t *)inc190, sizeof(inc190), QStringLiteral("inc190"), false, true);
            break;
        case 195:
            writeCharacteristic((uint8_t *)inc195, sizeof(inc195), QStringLiteral("inc195"), false, true);
            break;
        case 200:
            writeCharacteristic((uint8_t *)inc200, sizeof(inc200), QStringLiteral("inc200"), false, true);
            break;
        }
    } else if (nordictrack_se7i) {
        // SE7i uses ff 0d packet with byte[10]=0x02 for incline
        // Incline encoding: value = incline% * 100
        uint16_t incValue = (uint16_t)(requestIncline * 100);
        uint8_t incCmd[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                            0x02, (uint8_t)(incValue & 0xFF), (uint8_t)((incValue >> 8) & 0xFF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(incCmd, sizeof(incCmd), QStringLiteral("incline_se7i"), false, true);
    }
}

void nordictrackelliptical::forceResistance(resistance_t requestResistance) {

    QSettings settings;
    bool proform_hybrid_trainer_xt =
        settings.value(QZSettings::proform_hybrid_trainer_xt, QZSettings::default_proform_hybrid_trainer_xt).toBool();
    const uint8_t res[] = {0xfe, 0x02, 0x0d, 0x02};
    writeCharacteristic((uint8_t *)res, sizeof(res), QStringLiteral("resistance"), false, false);

    if (nordictrack_elliptical_c7_5) {
        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x98, 0x01, 0x00, 0xaf, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x5f, 0x03, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x25, 0x05, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0xec, 0x06, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0xb2, 0x08, 0x00, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x79, 0x0a, 0x00, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x3f, 0x0c, 0x00, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x06, 0x0e, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0xcc, 0x0f, 0x00, 0xf1, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x93, 0x11, 0x00, 0xba, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x5a, 0x13, 0x00, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x20, 0x15, 0x00, 0x4b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xe7, 0x16, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xad, 0x18, 0x00, 0xdb, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x74, 0x1a, 0x00, 0xa4, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x3a, 0x1c, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res17[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x01, 0x1e, 0x00, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res18[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xc7, 0x1f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res19[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x8e, 0x21, 0x00, 0xc5, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x54, 0x23, 0x00, 0x8d, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res21[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x1b, 0x25, 0x00, 0x56, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res22[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xe2, 0x26, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00};

        switch (requestResistance) {
        case 1:
            writeCharacteristic((uint8_t *)res1, sizeof(res1), QStringLiteral("resistance1"), false, true);
            break;
        case 2:
            writeCharacteristic((uint8_t *)res2, sizeof(res2), QStringLiteral("resistance2"), false, true);
            break;
        case 3:
            writeCharacteristic((uint8_t *)res3, sizeof(res3), QStringLiteral("resistance3"), false, true);
            break;
        case 4:
            writeCharacteristic((uint8_t *)res4, sizeof(res4), QStringLiteral("resistance4"), false, true);
            break;
        case 5:
            writeCharacteristic((uint8_t *)res5, sizeof(res5), QStringLiteral("resistance5"), false, true);
            break;
        case 6:
            writeCharacteristic((uint8_t *)res6, sizeof(res6), QStringLiteral("resistance6"), false, true);
            break;
        case 7:
            writeCharacteristic((uint8_t *)res7, sizeof(res7), QStringLiteral("resistance7"), false, true);
            break;
        case 8:
            writeCharacteristic((uint8_t *)res8, sizeof(res8), QStringLiteral("resistance8"), false, true);
            break;
        case 9:
            writeCharacteristic((uint8_t *)res9, sizeof(res9), QStringLiteral("resistance9"), false, true);
            break;
        case 10:
            writeCharacteristic((uint8_t *)res10, sizeof(res10), QStringLiteral("resistance10"), false, true);
            break;
        case 11:
            writeCharacteristic((uint8_t *)res11, sizeof(res11), QStringLiteral("resistance11"), false, true);
            break;
        case 12:
            writeCharacteristic((uint8_t *)res12, sizeof(res12), QStringLiteral("resistance12"), false, true);
            break;
        case 13:
            writeCharacteristic((uint8_t *)res13, sizeof(res13), QStringLiteral("resistance13"), false, true);
            break;
        case 14:
            writeCharacteristic((uint8_t *)res14, sizeof(res14), QStringLiteral("resistance14"), false, true);
            break;
        case 15:
            writeCharacteristic((uint8_t *)res15, sizeof(res15), QStringLiteral("resistance15"), false, true);
            break;
        case 16:
            writeCharacteristic((uint8_t *)res16, sizeof(res16), QStringLiteral("resistance16"), false, true);
            break;
        case 17:
            writeCharacteristic((uint8_t *)res17, sizeof(res17), QStringLiteral("resistance17"), false, true);
            break;
        case 18:
            writeCharacteristic((uint8_t *)res18, sizeof(res18), QStringLiteral("resistance18"), false, true);
            break;
        case 19:
            writeCharacteristic((uint8_t *)res19, sizeof(res19), QStringLiteral("resistance19"), false, true);
            break;
        case 20:
            writeCharacteristic((uint8_t *)res20, sizeof(res20), QStringLiteral("resistance20"), false, true);
            break;
        case 21:
            writeCharacteristic((uint8_t *)res21, sizeof(res21), QStringLiteral("resistance21"), false, true);
            break;
        case 22:
            writeCharacteristic((uint8_t *)res22, sizeof(res22), QStringLiteral("resistance22"), false, true);
            break;
        }
    } else if (nordictrack_se7i) {
        // SE7i uses ff 0d packet with byte[10]=0x04 for resistance
        // Resistance encoding: value = resistance * 454 - 1
        uint16_t resValue = (requestResistance * 454) - 1;
        uint8_t resCmd[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                            0x04, (uint8_t)(resValue & 0xFF), (uint8_t)((resValue >> 8) & 0xFF), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(resCmd, sizeof(resCmd), QStringLiteral("resistance_se7i"), false, true);
    } else if (proform_hybrid_trainer_xt) {
        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x32, 0x02, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0xa3, 0x04, 0x00, 0xbd, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x14, 0x07, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x85, 0x09, 0x00, 0xa4, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0xf6, 0x0b, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x67, 0x0e, 0x00, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0xd8, 0x10, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x2b, 0x12, 0x00, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x9c, 0x13, 0x00, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x0d, 0x18, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x7e, 0x1a, 0x00, 0xb3, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xef, 0x1b, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x60, 0x1f, 0x00, 0x9a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xd1, 0x21, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xd1, 0x24, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xd1, 0x25, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00};

        switch (requestResistance) {
        case 1:
            writeCharacteristic((uint8_t *)res1, sizeof(res1), QStringLiteral("resistance1"), false, true);
            break;
        case 2:
            writeCharacteristic((uint8_t *)res2, sizeof(res2), QStringLiteral("resistance2"), false, true);
            break;
        case 3:
            writeCharacteristic((uint8_t *)res3, sizeof(res3), QStringLiteral("resistance3"), false, true);
            break;
        case 4:
            writeCharacteristic((uint8_t *)res4, sizeof(res4), QStringLiteral("resistance4"), false, true);
            break;
        case 5:
            writeCharacteristic((uint8_t *)res5, sizeof(res5), QStringLiteral("resistance5"), false, true);
            break;
        case 6:
            writeCharacteristic((uint8_t *)res6, sizeof(res6), QStringLiteral("resistance6"), false, true);
            break;
        case 7:
            writeCharacteristic((uint8_t *)res7, sizeof(res7), QStringLiteral("resistance7"), false, true);
            break;
        case 8:
            writeCharacteristic((uint8_t *)res8, sizeof(res8), QStringLiteral("resistance8"), false, true);
            break;
        case 9:
            writeCharacteristic((uint8_t *)res9, sizeof(res9), QStringLiteral("resistance9"), false, true);
            break;
        case 10:
            writeCharacteristic((uint8_t *)res10, sizeof(res10), QStringLiteral("resistance10"), false, true);
            break;
        case 11:
            writeCharacteristic((uint8_t *)res11, sizeof(res11), QStringLiteral("resistance11"), false, true);
            break;
        case 12:
            writeCharacteristic((uint8_t *)res12, sizeof(res12), QStringLiteral("resistance12"), false, true);
            break;
        case 13:
            writeCharacteristic((uint8_t *)res13, sizeof(res13), QStringLiteral("resistance13"), false, true);
            break;
        case 14:
            writeCharacteristic((uint8_t *)res14, sizeof(res14), QStringLiteral("resistance14"), false, true);
            break;
        case 15:
            writeCharacteristic((uint8_t *)res15, sizeof(res15), QStringLiteral("resistance15"), false, true);
            break;
        case 16:
            writeCharacteristic((uint8_t *)res16, sizeof(res16), QStringLiteral("resistance16"), false, true);
            break;
        }
    } else {
        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0xc2, 0x01, 0x00, 0xd9, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0xb6, 0x03, 0x00, 0xcf, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0xaa, 0x05, 0x00, 0xc5, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x9e, 0x07, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x92, 0x09, 0x00, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x86, 0x0b, 0x00, 0xa7, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x7a, 0x0d, 0x00, 0x9d, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x6e, 0x0f, 0x00, 0x93, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                0x04, 0x62, 0x11, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x56, 0x13, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x4a, 0x15, 0x00, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x3e, 0x17, 0x00, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x32, 0x19, 0x00, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x26, 0x1b, 0x00, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x1a, 0x1d, 0x00, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x0e, 0x1f, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res17[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0x02, 0x21, 0x00, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res18[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xf6, 0x22, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res19[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xea, 0x24, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                                 0x04, 0xde, 0x26, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00};

        switch (requestResistance) {
        case 1:
            writeCharacteristic((uint8_t *)res1, sizeof(res1), QStringLiteral("resistance1"), false, true);
            break;
        case 2:
            writeCharacteristic((uint8_t *)res2, sizeof(res2), QStringLiteral("resistance2"), false, true);
            break;
        case 3:
            writeCharacteristic((uint8_t *)res3, sizeof(res3), QStringLiteral("resistance3"), false, true);
            break;
        case 4:
            writeCharacteristic((uint8_t *)res4, sizeof(res4), QStringLiteral("resistance4"), false, true);
            break;
        case 5:
            writeCharacteristic((uint8_t *)res5, sizeof(res5), QStringLiteral("resistance5"), false, true);
            break;
        case 6:
            writeCharacteristic((uint8_t *)res6, sizeof(res6), QStringLiteral("resistance6"), false, true);
            break;
        case 7:
            writeCharacteristic((uint8_t *)res7, sizeof(res7), QStringLiteral("resistance7"), false, true);
            break;
        case 8:
            writeCharacteristic((uint8_t *)res8, sizeof(res8), QStringLiteral("resistance8"), false, true);
            break;
        case 9:
            writeCharacteristic((uint8_t *)res9, sizeof(res9), QStringLiteral("resistance9"), false, true);
            break;
        case 10:
            writeCharacteristic((uint8_t *)res10, sizeof(res10), QStringLiteral("resistance10"), false, true);
            break;
        case 11:
            writeCharacteristic((uint8_t *)res11, sizeof(res11), QStringLiteral("resistance11"), false, true);
            break;
        case 12:
            writeCharacteristic((uint8_t *)res12, sizeof(res12), QStringLiteral("resistance12"), false, true);
            break;
        case 13:
            writeCharacteristic((uint8_t *)res13, sizeof(res13), QStringLiteral("resistance13"), false, true);
            break;
        case 14:
            writeCharacteristic((uint8_t *)res14, sizeof(res14), QStringLiteral("resistance14"), false, true);
            break;
        case 15:
            writeCharacteristic((uint8_t *)res15, sizeof(res15), QStringLiteral("resistance15"), false, true);
            break;
        case 16:
            writeCharacteristic((uint8_t *)res16, sizeof(res16), QStringLiteral("resistance16"), false, true);
            break;
        case 17:
            writeCharacteristic((uint8_t *)res17, sizeof(res17), QStringLiteral("resistance17"), false, true);
            break;
        case 18:
            writeCharacteristic((uint8_t *)res18, sizeof(res18), QStringLiteral("resistance18"), false, true);
            break;
        case 19:
            writeCharacteristic((uint8_t *)res19, sizeof(res19), QStringLiteral("resistance19"), false, true);
            break;
        case 20:
            writeCharacteristic((uint8_t *)res20, sizeof(res20), QStringLiteral("resistance20"), false, true);
            break;
        }
    }
}

void nordictrackelliptical::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        QSettings settings;
        bool proform_hybrid_trainer_xt =
            settings.value(QZSettings::proform_hybrid_trainer_xt, QZSettings::default_proform_hybrid_trainer_xt)
                .toBool();

        update_metrics(false, watts());

        if (nordictrack_elliptical_c7_5) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00,
                                   0x0d, 0x3e, 0x96, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x85, 0xc2, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x0d, 0x02};
            uint8_t noOpData5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x00,
                                   0x03, 0xb6, 0x98, 0x70, 0xd2, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"));
                if (requestResistance != -1) {
                    if (requestResistance < 0)
                        requestResistance = 0;
                    if (requestResistance != currentResistance().value() && requestResistance >= 0 &&
                        requestResistance <= max_resistance) {
                        emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                        forceResistance(requestResistance);
                    }
                    requestResistance = -1;
                }
                if (requestInclination != -1) {
                    if (requestInclination < 0)
                        requestInclination = 0;
                    if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                        requestInclination <= max_inclination) {
                        emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -1;
                }
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"), false, true);
                break;
            }
            counterPoll++;
            if (counterPoll > 4) {
                counterPoll = 0;
            }
        } else if (nordictrack_se7i) {
            // NordicTrack Elliptical SE7i - 6 packet sendPoll cycle
            uint8_t se7i_noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t se7i_noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00,
                                        0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
            uint8_t se7i_noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t se7i_noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t se7i_noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00,
                                        0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t se7i_noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(se7i_noOpData1, sizeof(se7i_noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(se7i_noOpData2, sizeof(se7i_noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(se7i_noOpData3, sizeof(se7i_noOpData3), QStringLiteral("noOp"));
                if (requestResistance != -1) {
                    if (requestResistance < 0)
                        requestResistance = 0;
                    if (requestResistance != currentResistance().value() && requestResistance >= 0 &&
                        requestResistance <= max_resistance) {
                        emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                        forceResistance(requestResistance);
                    }
                    requestResistance = -1;
                }
                if (requestInclination != -1) {
                    if (requestInclination < 0)
                        requestInclination = 0;
                    if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                        requestInclination <= max_inclination) {
                        emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -1;
                }
                break;
            case 3:
                writeCharacteristic(se7i_noOpData4, sizeof(se7i_noOpData4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(se7i_noOpData5, sizeof(se7i_noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(se7i_noOpData6, sizeof(se7i_noOpData6), QStringLiteral("noOp"));
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00,
                                   0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x81, 0xb4, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x0d, 0x02};
            uint8_t noOpData5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x00,
                                   0x03, 0x80, 0x00, 0x40, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"));
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"), false,
                                    proform_hybrid_trainer_xt);
                if (requestResistance != -1) {
                    if (requestResistance < 0)
                        requestResistance = 0;
                    if (requestResistance != currentResistance().value() && requestResistance >= 0 &&
                        requestResistance <= max_resistance) {
                        emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                        forceResistance(requestResistance);
                    }
                    requestResistance = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 4) {
                counterPoll = 0;
            }
        }

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }
        /*
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

        // btinit();

        requestStart = -1;
        emit tapeStarted();
    }*/
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void nordictrackelliptical::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void nordictrackelliptical::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

double nordictrackelliptical::GetInclinationFromPacket(QByteArray packet) {
    uint16_t r = ((uint16_t)((uint8_t)packet.at(10))) + ((uint16_t)((uint8_t)packet.at(11)) << 8);
    return ((double)r) / 100.0;
}

double nordictrackelliptical::GetResistanceFromPacket(QByteArray packet) {
    QSettings settings;
    bool proform_hybrid_trainer_xt =
        settings.value(QZSettings::proform_hybrid_trainer_xt, QZSettings::default_proform_hybrid_trainer_xt).toBool();

    if (nordictrack_elliptical_c7_5) {
        uint16_t r = ((uint16_t)((uint8_t)packet.at(12)) << 8) + ((uint16_t)((uint8_t)packet.at(13)));
        switch (r) {
        case 0x9801:
        case 0xc601:
            return 1;
        case 0x5f03:
        case 0x8c03:
            return 2;
        case 0x2505:
        case 0x5205:
            return 3;
        case 0xec06:
        case 0x1807:
            return 4;
        case 0xb208:
        case 0xde08:
            return 5;
        case 0x790a:
        case 0xa40a:
            return 6;
        case 0x3f0c:
        case 0x6a0c:
            return 7;
        case 0x060e:
        case 0x300e:
            return 8;
        case 0xcc0f:
        case 0xf60f:
            return 9;
        case 0x9311:
        case 0xbc11:
            return 10;
        case 0x5a13:
        case 0x8213:
            return 11;
        case 0x2015:
        case 0x4815:
            return 12;
        case 0xe716:
        case 0x0e17:
            return 13;
        case 0xad18:
        case 0xd418:
            return 14;
        case 0x741a:
        case 0x9a1a:
            return 15;
        case 0x3a1c:
        case 0x601c:
            return 16;
        case 0x011e:
        case 0x261e:
            return 17;
        case 0xc71f:
        case 0xec1f:
            return 18;
        case 0x8e21:
        case 0xb221:
            return 19;
        case 0x5423:
        case 0x7823:
            return 20;
        case 0x1b25:
        case 0x3e25:
            return 21;
        case 0xe226:
        case 0x0427:
            return 22;
        default:
        case 0:
            return 1;
        }
    } else if (proform_hybrid_trainer_xt) {
        uint8_t r = (uint8_t)(packet.at(11));
        switch (r) {
        case 0:
            return 0;
        case 2:
            return 1;
        case 4:
            return 2;
        case 7:
        case 8:
            return 3;
        case 9:
            return 4;
        case 0xb:
        case 0xc:
            return 5;
        case 0xe:
            return 6;
        case 0x10:
        case 0x11:
            return 7;
        case 0x12:
            return 8;
        case 0x15:
        case 0x13:
            return 9;
        case 0x18:
            return 10;
        case 0x1a:
            return 11;
        case 0x1b:
            return 12;
        case 0x1f:
            return 13;
        case 0x1d:
            return 14;
        case 0x21:
            return 14;
        case 0x22:
            return 14;
        case 0x23:
            return 15;
        case 0x24:
            return 15;
        case 0x27:
            return 16;
        }
    } else {
        uint8_t r = (uint8_t)(packet.at(11));
        switch (r) {
        case 1:
            return 1;
        case 3:
            return 2;
        case 5:
            return 3;
        case 7:
        case 8:
            return 4;
        case 9:
            return 5;
        case 0xb:
            return 6;
        case 0xc:
        case 0xd:
            return 7;
        case 0xf:
            return 8;
        case 0x11:
            return 9;
        case 0x13:
            return 10;
        case 0x14:
        case 0x15:
            return 11;
        case 0x17:
            return 12;
        case 0x19:
        case 0x1a:
            return 13;
        case 0x1b:
            return 14;
        case 0x1d:
            return 15;
        case 0x1f:
            return 16;
        case 0x21:
            return 17;
        case 0x22:
        case 0x23:
            return 18;
        case 0x24:
        case 0x25:
            return 19;
        case 0x26:
        case 0x27:
            return 20;
        }
    }
    return 1;
}

void nordictrackelliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                  const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    double cadence_gain = settings.value(QZSettings::cadence_gain, QZSettings::default_cadence_gain).toDouble();
    double cadence_offset = settings.value(QZSettings::cadence_offset, QZSettings::default_cadence_offset).toDouble();
    // const double miles = 1.60934; //not used
    bool proform_hybrid_trainer_xt =
        settings.value(QZSettings::proform_hybrid_trainer_xt, QZSettings::default_proform_hybrid_trainer_xt).toBool();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    uint8_t heart = 0;

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    // SE7i Speed and Cadence parsing (Type 0x01 packets with byte[4]=0x46)
    if (nordictrack_se7i && newValue.length() == 20 && newValue.at(0) == 0x01 && newValue.at(1) == 0x12 &&
        newValue.at(4) == 0x46 && initDone == true) {
        // Parse speed from bytes 12-13 (little endian, divided by 100)
        Speed = (double)(((uint16_t)((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t)newValue.at(12))) / 100.0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        lastSpeedChanged = QDateTime::currentDateTime();

        // Parse cadence from byte 2
        uint8_t c = newValue.at(2);
        if (c > 0)
            Cadence = (c * cadence_gain) + cadence_offset;
        else
            Cadence = 0;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }
        return;
    }

    if (!proform_hybrid_trainer_xt && !nordictrack_elliptical_c7_5 && newValue.length() == 20 &&
        newValue.at(0) == 0x01 && newValue.at(1) == 0x12 && newValue.at(19) == 0x2C) {
        uint8_t c = newValue.at(2);
        if (c > 0)
            Cadence = (c * cadence_gain) + cadence_offset;
        else
            Cadence = 0;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }
        return;
    }

    if (!nordictrack_elliptical_c7_5 && !nordictrack_se7i && newValue.length() == 20 && newValue.at(0) == 0x01 && newValue.at(1) == 0x12 &&
        initDone == true) {
        Speed = (double)(((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t)newValue.at(14))) / 100.0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        lastSpeedChanged = QDateTime::currentDateTime();

        if (proform_hybrid_trainer_xt && !disable_hr_frommachinery) {
            heart = newValue.at(3);
            Heart = heart;
            emit debug(QStringLiteral("Current Heart from machinery: ") + QString::number(heart));
        }

    } else if (!nordictrack_se7i && QDateTime::currentDateTime().secsTo(lastSpeedChanged) > 3) {
        Speed = 0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    }

    if (nordictrack_elliptical_c7_5 && newValue.length() == 20 && newValue.at(0) == 0x01 && newValue.at(1) == 0x12 &&
        newValue.at(4) == 0x46 && initDone == true &&
        !(((uint8_t)newValue.at(4)) == 0xFF && ((uint8_t)newValue.at(5)) == 0xFF && ((uint8_t)newValue.at(6)) == 0xFF &&
          ((uint8_t)newValue.at(7)) == 0xFF && ((uint8_t)newValue.at(8)) == 0xFF && ((uint8_t)newValue.at(9)) == 0xFF &&
          ((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(11)) == 0xFF)) {
        Speed = (double)(((uint16_t)((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t)newValue.at(12))) / 100.0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        lastSpeedChanged = QDateTime::currentDateTime();

        if (!disable_hr_frommachinery) {
            heart = newValue.at(5);
            Heart = heart;
            emit debug(QStringLiteral("Current Heart from machinery: ") + QString::number(heart));
        }

        uint8_t c = newValue.at(2);
        if (c > 0)
            Cadence = (c * cadence_gain) + cadence_offset;
        else
            Cadence = 0;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

    } else if (QDateTime::currentDateTime().secsTo(lastSpeedChanged) > 3) {
        Speed = 0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    }

    // Skip the strict packet validation for SE7i (it has different packet structures)
    if (!nordictrack_se7i &&
        (newValue.length() != 20 || (newValue.at(0) != 0x00 && newValue.at(0) != 0x01) || newValue.at(1) != 0x12 ||
        newValue.at(2) != 0x01 || newValue.at(3) != 0x04 || newValue.at(4) != 0x02 ||
        (newValue.at(5) != 0x2e && newValue.at(5) != 0x30 && newValue.at(5) != 0x31) ||
        (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF &&
         ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF &&
         ((uint8_t)newValue.at(16)) == 0xFF && ((uint8_t)newValue.at(17)) == 0xFF &&
         ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF))) {
        return;
    }

    // wattage = newValue.at(12)

    if (proform_hybrid_trainer_xt) {
        uint8_t c = newValue.at(18);
        if (c > 0)
            Cadence = (c * cadence_gain) + cadence_offset;
        else
            Cadence = 0;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }
    }

    if (nordictrack_se7i && newValue.length() == 20 && newValue.at(0) == 0x00 &&
        newValue.at(1) == 0x12 && newValue.at(2) == 0x01 && newValue.at(3) == 0x04 && newValue.at(4) == 0x02 &&
        (newValue.at(5) == 0x30 || newValue.at(5) == 0x31)) {
        // SE7i Resistance and Inclination parsing (Type 0x00 packets)
        // Inclination from bytes 10-11 (little endian, divided by 100)
        uint16_t incValue = ((uint16_t)((uint8_t)newValue.at(10))) + ((uint16_t)((uint8_t)newValue.at(11)) << 8);
        Inclination = ((double)incValue) / 100.0;
        emit debug(QStringLiteral("Current Inclination from packet: ") + QString::number(Inclination.value()));

        // Resistance from bytes 12-13 (little endian): resistance = (value + 1) / 454
        uint16_t resValue = ((uint16_t)((uint8_t)newValue.at(12))) + ((uint16_t)((uint8_t)newValue.at(13)) << 8);
        Resistance = ((double)(resValue + 1)) / 454.0;
        emit debug(QStringLiteral("Current Resistance from packet: ") + QString::number(Resistance.value()));
    } else if (!nordictrack_elliptical_c7_5 && !nordictrack_se7i) {
        Resistance = GetResistanceFromPacket(newValue);
    } else if (nordictrack_elliptical_c7_5 && newValue.length() == 20 && newValue.at(0) == 0x00 &&
               newValue.at(1) == 0x12 && newValue.at(2) == 0x01 && newValue.at(3) == 0x04 && newValue.at(4) == 0x02 &&
               (newValue.at(5) == 0x30 || newValue.at(5) == 0x31)) {
        Inclination = GetInclinationFromPacket(newValue);
        Resistance = GetResistanceFromPacket(newValue);
    }

    uint16_t p = qCeil((Resistance.value() * 3.33) + 10.0);
    if (p > 100)
        p = 100;
    m_pelotonResistance = p;

    if (watts())
        KCal += ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
    // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    if (disable_hr_frommachinery) {
#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                update_hr_from_external();
            }
        }
    }

    emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    // debug("Current Distance: " + QString::number(distance));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
    emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void nordictrackelliptical::btinit() {
    QSettings settings;
    bool proform_hybrid_trainer_xt =
        settings.value(QZSettings::proform_hybrid_trainer_xt, QZSettings::default_proform_hybrid_trainer_xt).toBool();

    if (nordictrack_se7i) {
        // NordicTrack Elliptical SE7i FULL initialization (418 packets: pkt944 to pkt2740)
        max_resistance = 22;
        max_inclination = 20;
        
        uint8_t se7i_init_001[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t se7i_init_002[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_003[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t se7i_init_004[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x80, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_005[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t se7i_init_006[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x88, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_007[] = {0xfe, 0x02, 0x0b, 0x02};
        uint8_t se7i_init_008[] = {0xff, 0x0b, 0x02, 0x04, 0x02, 0x07, 0x02, 0x07, 0x82, 0x00, 0x00, 0x00, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_009[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t se7i_init_010[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_011[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t se7i_init_012[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_013[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t se7i_init_014[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x06, 0x28, 0x90, 0x04, 0x00, 0x0d, 0x68, 0xc9, 0x28, 0x95, 0xf0, 0x69, 0xc0, 0x3d};
        uint8_t se7i_init_015[] = {0x01, 0x12, 0xa8, 0x19, 0x88, 0xf5, 0x60, 0xf9, 0x70, 0xcd, 0x48, 0xc9, 0x48, 0xf5, 0x70, 0xe9, 0x60, 0x1d, 0x88, 0x39};
        uint8_t se7i_init_016[] = {0xff, 0x08, 0xa8, 0x55, 0xc0, 0x80, 0x02, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_017[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t se7i_init_018[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x06, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_019[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_020[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_021[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_022[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_023[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_024[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
        uint8_t se7i_init_025[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x68, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_026[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_027[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_028[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_029[] = {0xfe, 0x02, 0x11, 0x02};
        uint8_t se7i_init_030[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x06, 0x0d, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x08, 0x58, 0x02, 0x00, 0x7c, 0x00};
        uint8_t se7i_init_031[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t se7i_init_032[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x06, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_033[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_034[] = {0xfe, 0x02, 0x11, 0x02};
        uint8_t se7i_init_035[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x06, 0x0d, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x08, 0x58, 0x02, 0x00, 0x7c, 0x00};
        uint8_t se7i_init_036[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_037[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_038[] = {0xfe, 0x02, 0x10, 0x02};
        uint8_t se7i_init_039[] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x06, 0x0c, 0x02, 0x04, 0x00, 0x00, 0x00, 0x02, 0x18, 0x15, 0x00, 0x47, 0x00, 0x00};
        uint8_t se7i_init_040[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_041[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_042[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_043[] = {0xfe, 0x02, 0x10, 0x02};
        uint8_t se7i_init_044[] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x06, 0x0c, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x2a, 0x00, 0x00};
        uint8_t se7i_init_045[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_046[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_047[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_048[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_049[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_050[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_051[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_052[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        /*uint8_t se7i_init_053[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_054[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_055[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_056[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_057[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_058[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_059[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_060[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        uint8_t se7i_init_061[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_062[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_063[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_064[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_065[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_066[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_067[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_068[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t se7i_init_069[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_070[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_071[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_072[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_073[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_074[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_075[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_076[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        uint8_t se7i_init_077[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_078[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_079[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_080[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_081[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_082[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_083[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_084[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        uint8_t se7i_init_085[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_086[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_087[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_088[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_089[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_090[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_091[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_092[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        uint8_t se7i_init_093[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_094[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_095[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_096[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_097[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_098[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_099[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_100[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_101[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_102[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_103[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_104[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_105[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_106[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_107[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_108[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_109[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_110[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_111[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_112[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_113[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_114[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_115[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_116[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_117[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_118[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_119[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_120[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_121[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_122[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_123[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_124[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_125[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_126[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_127[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_128[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_129[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_130[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_131[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_132[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_133[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_134[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_135[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_136[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_137[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_138[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_139[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_140[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_141[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_142[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_143[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_144[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_145[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_146[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_147[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_148[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_149[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_150[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_151[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_152[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_153[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_154[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_155[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_156[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_157[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_158[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_159[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_160[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_161[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_162[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_163[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_164[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_165[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_166[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_167[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_168[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_169[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_170[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_171[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_172[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_173[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_174[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_175[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_176[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_177[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_178[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_179[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_180[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_181[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_182[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_183[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_184[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_185[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_186[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_187[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_188[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_189[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_190[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_191[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_192[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_193[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_194[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_195[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_196[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_197[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_198[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_199[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_200[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_201[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_202[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_203[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_204[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_205[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_206[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_207[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_208[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_209[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_210[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_211[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_212[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_213[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_214[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_215[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_216[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_217[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_218[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_219[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_220[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_221[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_222[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_223[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_224[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_225[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_226[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_227[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_228[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_229[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_230[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_231[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_232[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_233[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_234[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_235[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_236[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_237[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_238[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_239[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_240[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_241[] = {0xfe, 0x02, 0x0d, 0x02};
        uint8_t se7i_init_242[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x02, 0x00, 0x10, 0x0d, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_243[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_244[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_245[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_246[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_247[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_248[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_249[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_250[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_251[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_252[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_253[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_254[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_255[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_256[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_257[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_258[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_259[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_260[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_261[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_262[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_263[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_264[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_265[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_266[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_267[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_268[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_269[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_270[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_271[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_272[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_273[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_274[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_275[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_276[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_277[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_278[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_279[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_280[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_281[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_282[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_283[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_284[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_285[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_286[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_287[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_288[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_289[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_290[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_291[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_292[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_293[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_294[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_295[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_296[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_297[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_298[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_299[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_300[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_301[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_302[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_303[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_304[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_305[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_306[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_307[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_308[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_309[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_310[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_311[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_312[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_313[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_314[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_315[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_316[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_317[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_318[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_319[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_320[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_321[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_322[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_323[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_324[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_325[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_326[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_327[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_328[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_329[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_330[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_331[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_332[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_333[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_334[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_335[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_336[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_337[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_338[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_339[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_340[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_341[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_342[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_343[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_344[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_345[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_346[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_347[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_348[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_349[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_350[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_351[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_352[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_353[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_354[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_355[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_356[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_357[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_358[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_359[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_360[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_361[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_362[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_363[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_364[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_365[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_366[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_367[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_368[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_369[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_370[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_371[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_372[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_373[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_374[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_375[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_376[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_377[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_378[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};*/
        uint8_t se7i_init_379[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_380[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_381[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_382[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_383[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_384[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_385[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_386[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_387[] = {0xfe, 0x02, 0x11, 0x02};
        uint8_t se7i_init_388[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x06, 0x0d, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x08, 0x58, 0x02, 0x00, 0x7c, 0x00};
        uint8_t se7i_init_389[] = {0xfe, 0x02, 0x10, 0x02};
        uint8_t se7i_init_390[] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x06, 0x0c, 0x02, 0x04, 0x00, 0x00, 0x00, 0x02, 0x18, 0x15, 0x00, 0x47, 0x00, 0x00};
        uint8_t se7i_init_391[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_392[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10, 0x40, 0x50, 0x00, 0x80};
        uint8_t se7i_init_393[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_394[] = {0xfe, 0x02, 0x10, 0x02};
        uint8_t se7i_init_395[] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x06, 0x0c, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x2a, 0x00, 0x00};
        uint8_t se7i_init_396[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_397[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_398[] = {0xfe, 0x02, 0x15, 0x03};
        uint8_t se7i_init_399[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x11, 0x06, 0x11, 0x02, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x58};
        uint8_t se7i_init_400[] = {0xff, 0x03, 0x02, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_401[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_402[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_403[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_404[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_405[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_406[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_407[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_408[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t se7i_init_409[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_410[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x04, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_411[] = {0xfe, 0x02, 0x0f, 0x02};
        uint8_t se7i_init_412[] = {0xff, 0x0f, 0x02, 0x04, 0x02, 0x0b, 0x06, 0x0b, 0x02, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x08, 0x20, 0x00, 0x00, 0x00};
        uint8_t se7i_init_413[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_414[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x02, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_415[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_416[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t se7i_init_417[] = {0xfe, 0x02, 0x0c, 0x02};
        uint8_t se7i_init_418[] = {0xff, 0x0c, 0x02, 0x04, 0x02, 0x08, 0x06, 0x08, 0x02, 0x00, 0x02, 0x00, 0x10, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        writeCharacteristic(se7i_init_001, sizeof(se7i_init_001), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_002, sizeof(se7i_init_002), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_003, sizeof(se7i_init_003), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_004, sizeof(se7i_init_004), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_005, sizeof(se7i_init_005), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_006, sizeof(se7i_init_006), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_007, sizeof(se7i_init_007), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_008, sizeof(se7i_init_008), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_009, sizeof(se7i_init_009), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_010, sizeof(se7i_init_010), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_011, sizeof(se7i_init_011), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_012, sizeof(se7i_init_012), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_013, sizeof(se7i_init_013), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_014, sizeof(se7i_init_014), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_015, sizeof(se7i_init_015), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_016, sizeof(se7i_init_016), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_017, sizeof(se7i_init_017), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_018, sizeof(se7i_init_018), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_019, sizeof(se7i_init_019), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_020, sizeof(se7i_init_020), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_021, sizeof(se7i_init_021), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_022, sizeof(se7i_init_022), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_023, sizeof(se7i_init_023), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_024, sizeof(se7i_init_024), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_025, sizeof(se7i_init_025), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_026, sizeof(se7i_init_026), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_027, sizeof(se7i_init_027), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_028, sizeof(se7i_init_028), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_029, sizeof(se7i_init_029), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_030, sizeof(se7i_init_030), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_031, sizeof(se7i_init_031), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_032, sizeof(se7i_init_032), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_033, sizeof(se7i_init_033), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_034, sizeof(se7i_init_034), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_035, sizeof(se7i_init_035), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_036, sizeof(se7i_init_036), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_037, sizeof(se7i_init_037), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_038, sizeof(se7i_init_038), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_039, sizeof(se7i_init_039), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_040, sizeof(se7i_init_040), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_041, sizeof(se7i_init_041), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_042, sizeof(se7i_init_042), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_043, sizeof(se7i_init_043), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_044, sizeof(se7i_init_044), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_045, sizeof(se7i_init_045), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_046, sizeof(se7i_init_046), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_047, sizeof(se7i_init_047), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_048, sizeof(se7i_init_048), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_049, sizeof(se7i_init_049), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_050, sizeof(se7i_init_050), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_051, sizeof(se7i_init_051), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_052, sizeof(se7i_init_052), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        /*writeCharacteristic(se7i_init_053, sizeof(se7i_init_053), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_054, sizeof(se7i_init_054), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_055, sizeof(se7i_init_055), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_056, sizeof(se7i_init_056), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_057, sizeof(se7i_init_057), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_058, sizeof(se7i_init_058), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_059, sizeof(se7i_init_059), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_060, sizeof(se7i_init_060), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_061, sizeof(se7i_init_061), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_062, sizeof(se7i_init_062), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_063, sizeof(se7i_init_063), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_064, sizeof(se7i_init_064), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_065, sizeof(se7i_init_065), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_066, sizeof(se7i_init_066), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_067, sizeof(se7i_init_067), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_068, sizeof(se7i_init_068), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_069, sizeof(se7i_init_069), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_070, sizeof(se7i_init_070), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_071, sizeof(se7i_init_071), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_072, sizeof(se7i_init_072), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_073, sizeof(se7i_init_073), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_074, sizeof(se7i_init_074), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_075, sizeof(se7i_init_075), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_076, sizeof(se7i_init_076), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_077, sizeof(se7i_init_077), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_078, sizeof(se7i_init_078), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_079, sizeof(se7i_init_079), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_080, sizeof(se7i_init_080), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_081, sizeof(se7i_init_081), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_082, sizeof(se7i_init_082), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_083, sizeof(se7i_init_083), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_084, sizeof(se7i_init_084), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_085, sizeof(se7i_init_085), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_086, sizeof(se7i_init_086), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_087, sizeof(se7i_init_087), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_088, sizeof(se7i_init_088), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_089, sizeof(se7i_init_089), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_090, sizeof(se7i_init_090), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_091, sizeof(se7i_init_091), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_092, sizeof(se7i_init_092), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_093, sizeof(se7i_init_093), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_094, sizeof(se7i_init_094), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_095, sizeof(se7i_init_095), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_096, sizeof(se7i_init_096), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_097, sizeof(se7i_init_097), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_098, sizeof(se7i_init_098), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_099, sizeof(se7i_init_099), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_100, sizeof(se7i_init_100), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_101, sizeof(se7i_init_101), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_102, sizeof(se7i_init_102), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_103, sizeof(se7i_init_103), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_104, sizeof(se7i_init_104), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_105, sizeof(se7i_init_105), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_106, sizeof(se7i_init_106), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_107, sizeof(se7i_init_107), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_108, sizeof(se7i_init_108), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_109, sizeof(se7i_init_109), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_110, sizeof(se7i_init_110), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_111, sizeof(se7i_init_111), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_112, sizeof(se7i_init_112), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_113, sizeof(se7i_init_113), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_114, sizeof(se7i_init_114), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_115, sizeof(se7i_init_115), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_116, sizeof(se7i_init_116), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_117, sizeof(se7i_init_117), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_118, sizeof(se7i_init_118), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_119, sizeof(se7i_init_119), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_120, sizeof(se7i_init_120), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_121, sizeof(se7i_init_121), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_122, sizeof(se7i_init_122), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_123, sizeof(se7i_init_123), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_124, sizeof(se7i_init_124), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_125, sizeof(se7i_init_125), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_126, sizeof(se7i_init_126), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_127, sizeof(se7i_init_127), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_128, sizeof(se7i_init_128), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_129, sizeof(se7i_init_129), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_130, sizeof(se7i_init_130), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_131, sizeof(se7i_init_131), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_132, sizeof(se7i_init_132), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_133, sizeof(se7i_init_133), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_134, sizeof(se7i_init_134), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_135, sizeof(se7i_init_135), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_136, sizeof(se7i_init_136), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_137, sizeof(se7i_init_137), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_138, sizeof(se7i_init_138), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_139, sizeof(se7i_init_139), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_140, sizeof(se7i_init_140), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_141, sizeof(se7i_init_141), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_142, sizeof(se7i_init_142), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_143, sizeof(se7i_init_143), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_144, sizeof(se7i_init_144), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_145, sizeof(se7i_init_145), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_146, sizeof(se7i_init_146), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_147, sizeof(se7i_init_147), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_148, sizeof(se7i_init_148), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_149, sizeof(se7i_init_149), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_150, sizeof(se7i_init_150), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_151, sizeof(se7i_init_151), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_152, sizeof(se7i_init_152), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_153, sizeof(se7i_init_153), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_154, sizeof(se7i_init_154), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_155, sizeof(se7i_init_155), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_156, sizeof(se7i_init_156), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_157, sizeof(se7i_init_157), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_158, sizeof(se7i_init_158), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_159, sizeof(se7i_init_159), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_160, sizeof(se7i_init_160), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_161, sizeof(se7i_init_161), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_162, sizeof(se7i_init_162), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_163, sizeof(se7i_init_163), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_164, sizeof(se7i_init_164), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_165, sizeof(se7i_init_165), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_166, sizeof(se7i_init_166), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_167, sizeof(se7i_init_167), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_168, sizeof(se7i_init_168), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_169, sizeof(se7i_init_169), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_170, sizeof(se7i_init_170), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_171, sizeof(se7i_init_171), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_172, sizeof(se7i_init_172), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_173, sizeof(se7i_init_173), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_174, sizeof(se7i_init_174), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_175, sizeof(se7i_init_175), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_176, sizeof(se7i_init_176), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_177, sizeof(se7i_init_177), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_178, sizeof(se7i_init_178), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_179, sizeof(se7i_init_179), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_180, sizeof(se7i_init_180), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_181, sizeof(se7i_init_181), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_182, sizeof(se7i_init_182), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_183, sizeof(se7i_init_183), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_184, sizeof(se7i_init_184), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_185, sizeof(se7i_init_185), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_186, sizeof(se7i_init_186), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_187, sizeof(se7i_init_187), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_188, sizeof(se7i_init_188), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_189, sizeof(se7i_init_189), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_190, sizeof(se7i_init_190), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_191, sizeof(se7i_init_191), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_192, sizeof(se7i_init_192), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_193, sizeof(se7i_init_193), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_194, sizeof(se7i_init_194), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_195, sizeof(se7i_init_195), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_196, sizeof(se7i_init_196), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_197, sizeof(se7i_init_197), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_198, sizeof(se7i_init_198), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_199, sizeof(se7i_init_199), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_200, sizeof(se7i_init_200), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_201, sizeof(se7i_init_201), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_202, sizeof(se7i_init_202), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_203, sizeof(se7i_init_203), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_204, sizeof(se7i_init_204), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_205, sizeof(se7i_init_205), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_206, sizeof(se7i_init_206), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_207, sizeof(se7i_init_207), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_208, sizeof(se7i_init_208), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_209, sizeof(se7i_init_209), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_210, sizeof(se7i_init_210), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_211, sizeof(se7i_init_211), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_212, sizeof(se7i_init_212), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_213, sizeof(se7i_init_213), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_214, sizeof(se7i_init_214), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_215, sizeof(se7i_init_215), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_216, sizeof(se7i_init_216), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_217, sizeof(se7i_init_217), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_218, sizeof(se7i_init_218), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_219, sizeof(se7i_init_219), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_220, sizeof(se7i_init_220), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_221, sizeof(se7i_init_221), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_222, sizeof(se7i_init_222), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_223, sizeof(se7i_init_223), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_224, sizeof(se7i_init_224), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_225, sizeof(se7i_init_225), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_226, sizeof(se7i_init_226), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_227, sizeof(se7i_init_227), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_228, sizeof(se7i_init_228), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_229, sizeof(se7i_init_229), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_230, sizeof(se7i_init_230), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_231, sizeof(se7i_init_231), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_232, sizeof(se7i_init_232), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_233, sizeof(se7i_init_233), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_234, sizeof(se7i_init_234), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_235, sizeof(se7i_init_235), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_236, sizeof(se7i_init_236), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_237, sizeof(se7i_init_237), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_238, sizeof(se7i_init_238), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_239, sizeof(se7i_init_239), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_240, sizeof(se7i_init_240), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_241, sizeof(se7i_init_241), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_242, sizeof(se7i_init_242), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_243, sizeof(se7i_init_243), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_244, sizeof(se7i_init_244), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_245, sizeof(se7i_init_245), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_246, sizeof(se7i_init_246), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_247, sizeof(se7i_init_247), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_248, sizeof(se7i_init_248), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_249, sizeof(se7i_init_249), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_250, sizeof(se7i_init_250), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_251, sizeof(se7i_init_251), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_252, sizeof(se7i_init_252), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_253, sizeof(se7i_init_253), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_254, sizeof(se7i_init_254), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_255, sizeof(se7i_init_255), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_256, sizeof(se7i_init_256), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_257, sizeof(se7i_init_257), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_258, sizeof(se7i_init_258), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_259, sizeof(se7i_init_259), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_260, sizeof(se7i_init_260), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_261, sizeof(se7i_init_261), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_262, sizeof(se7i_init_262), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_263, sizeof(se7i_init_263), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_264, sizeof(se7i_init_264), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_265, sizeof(se7i_init_265), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_266, sizeof(se7i_init_266), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_267, sizeof(se7i_init_267), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_268, sizeof(se7i_init_268), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_269, sizeof(se7i_init_269), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_270, sizeof(se7i_init_270), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_271, sizeof(se7i_init_271), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_272, sizeof(se7i_init_272), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_273, sizeof(se7i_init_273), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_274, sizeof(se7i_init_274), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_275, sizeof(se7i_init_275), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_276, sizeof(se7i_init_276), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_277, sizeof(se7i_init_277), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_278, sizeof(se7i_init_278), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_279, sizeof(se7i_init_279), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_280, sizeof(se7i_init_280), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_281, sizeof(se7i_init_281), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_282, sizeof(se7i_init_282), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_283, sizeof(se7i_init_283), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_284, sizeof(se7i_init_284), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_285, sizeof(se7i_init_285), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_286, sizeof(se7i_init_286), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_287, sizeof(se7i_init_287), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_288, sizeof(se7i_init_288), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_289, sizeof(se7i_init_289), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_290, sizeof(se7i_init_290), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_291, sizeof(se7i_init_291), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_292, sizeof(se7i_init_292), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_293, sizeof(se7i_init_293), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_294, sizeof(se7i_init_294), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_295, sizeof(se7i_init_295), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_296, sizeof(se7i_init_296), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_297, sizeof(se7i_init_297), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_298, sizeof(se7i_init_298), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_299, sizeof(se7i_init_299), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_300, sizeof(se7i_init_300), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_301, sizeof(se7i_init_301), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_302, sizeof(se7i_init_302), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_303, sizeof(se7i_init_303), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_304, sizeof(se7i_init_304), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_305, sizeof(se7i_init_305), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_306, sizeof(se7i_init_306), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_307, sizeof(se7i_init_307), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_308, sizeof(se7i_init_308), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_309, sizeof(se7i_init_309), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_310, sizeof(se7i_init_310), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_311, sizeof(se7i_init_311), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_312, sizeof(se7i_init_312), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_313, sizeof(se7i_init_313), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_314, sizeof(se7i_init_314), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_315, sizeof(se7i_init_315), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_316, sizeof(se7i_init_316), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_317, sizeof(se7i_init_317), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_318, sizeof(se7i_init_318), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_319, sizeof(se7i_init_319), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_320, sizeof(se7i_init_320), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_321, sizeof(se7i_init_321), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_322, sizeof(se7i_init_322), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_323, sizeof(se7i_init_323), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_324, sizeof(se7i_init_324), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_325, sizeof(se7i_init_325), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_326, sizeof(se7i_init_326), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_327, sizeof(se7i_init_327), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_328, sizeof(se7i_init_328), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_329, sizeof(se7i_init_329), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_330, sizeof(se7i_init_330), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_331, sizeof(se7i_init_331), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_332, sizeof(se7i_init_332), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_333, sizeof(se7i_init_333), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_334, sizeof(se7i_init_334), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_335, sizeof(se7i_init_335), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_336, sizeof(se7i_init_336), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_337, sizeof(se7i_init_337), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_338, sizeof(se7i_init_338), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_339, sizeof(se7i_init_339), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_340, sizeof(se7i_init_340), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_341, sizeof(se7i_init_341), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_342, sizeof(se7i_init_342), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_343, sizeof(se7i_init_343), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_344, sizeof(se7i_init_344), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_345, sizeof(se7i_init_345), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_346, sizeof(se7i_init_346), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_347, sizeof(se7i_init_347), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_348, sizeof(se7i_init_348), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_349, sizeof(se7i_init_349), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_350, sizeof(se7i_init_350), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_351, sizeof(se7i_init_351), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_352, sizeof(se7i_init_352), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_353, sizeof(se7i_init_353), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_354, sizeof(se7i_init_354), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_355, sizeof(se7i_init_355), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_356, sizeof(se7i_init_356), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_357, sizeof(se7i_init_357), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_358, sizeof(se7i_init_358), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_359, sizeof(se7i_init_359), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_360, sizeof(se7i_init_360), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_361, sizeof(se7i_init_361), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_362, sizeof(se7i_init_362), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_363, sizeof(se7i_init_363), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_364, sizeof(se7i_init_364), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_365, sizeof(se7i_init_365), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_366, sizeof(se7i_init_366), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_367, sizeof(se7i_init_367), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_368, sizeof(se7i_init_368), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_369, sizeof(se7i_init_369), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_370, sizeof(se7i_init_370), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_371, sizeof(se7i_init_371), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_372, sizeof(se7i_init_372), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_373, sizeof(se7i_init_373), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_374, sizeof(se7i_init_374), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_375, sizeof(se7i_init_375), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_376, sizeof(se7i_init_376), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_377, sizeof(se7i_init_377), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_378, sizeof(se7i_init_378), QStringLiteral("init"), false, true);*/
        QThread::msleep(400);
        writeCharacteristic(se7i_init_379, sizeof(se7i_init_379), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_380, sizeof(se7i_init_380), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_381, sizeof(se7i_init_381), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_382, sizeof(se7i_init_382), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_383, sizeof(se7i_init_383), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_384, sizeof(se7i_init_384), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_385, sizeof(se7i_init_385), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_386, sizeof(se7i_init_386), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_387, sizeof(se7i_init_387), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_388, sizeof(se7i_init_388), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_389, sizeof(se7i_init_389), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_390, sizeof(se7i_init_390), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_391, sizeof(se7i_init_391), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_392, sizeof(se7i_init_392), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_393, sizeof(se7i_init_393), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_394, sizeof(se7i_init_394), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_395, sizeof(se7i_init_395), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_396, sizeof(se7i_init_396), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_397, sizeof(se7i_init_397), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_398, sizeof(se7i_init_398), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_399, sizeof(se7i_init_399), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_400, sizeof(se7i_init_400), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_401, sizeof(se7i_init_401), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_402, sizeof(se7i_init_402), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_403, sizeof(se7i_init_403), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_404, sizeof(se7i_init_404), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_405, sizeof(se7i_init_405), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_406, sizeof(se7i_init_406), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_407, sizeof(se7i_init_407), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_408, sizeof(se7i_init_408), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_409, sizeof(se7i_init_409), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_410, sizeof(se7i_init_410), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_411, sizeof(se7i_init_411), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_412, sizeof(se7i_init_412), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_413, sizeof(se7i_init_413), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_414, sizeof(se7i_init_414), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_415, sizeof(se7i_init_415), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_416, sizeof(se7i_init_416), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_417, sizeof(se7i_init_417), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic(se7i_init_418, sizeof(se7i_init_418), QStringLiteral("init"), false, true);
        QThread::msleep(400);

        initDone = true;
        return;
    }

    {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x80, 0x8a,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x88, 0x92,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};

        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x06, 0x28, 0x90, 0x04,
                                0x00, 0x2e, 0x44, 0x50, 0x6a, 0x82, 0xa8, 0xb4, 0xd6, 0xf6};
        uint8_t initData10b[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x06, 0x28, 0x90, 0x04,
                                 0x00, 0x96, 0x24, 0xa8, 0x3a, 0xca, 0x68, 0xfc, 0x86, 0x2e};
        uint8_t initData10c[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x06, 0x28, 0x90, 0x07,
                                 0x01, 0x97, 0x70, 0x53, 0x3c, 0x27, 0x00, 0xfb, 0xd4, 0xc7};

        uint8_t initData11[] = {0x01, 0x12, 0x2c, 0x48, 0x62, 0x9a, 0xd0, 0xfc, 0x0e, 0x5e,
                                0x94, 0xa0, 0xfa, 0x32, 0x78, 0x84, 0xc6, 0x06, 0x7c, 0xb8};
        uint8_t initData11b[] = {0x01, 0x12, 0xcc, 0x50, 0xf2, 0x92, 0x50, 0xf4, 0x9e, 0x26,
                                 0xf4, 0x98, 0x2a, 0xfa, 0xb8, 0x4c, 0x16, 0xde, 0x9c, 0x20};
        uint8_t initData11c[] = {0x01, 0x12, 0xb0, 0xa3, 0xac, 0x97, 0x80, 0x9b, 0x94, 0x97,
                                 0x90, 0x93, 0x9c, 0x87, 0x80, 0xbb, 0xb4, 0xa7, 0xd0, 0xc3};

        uint8_t initData12[] = {0xff, 0x08, 0xf2, 0x2a, 0xa0, 0x80, 0x02, 0x00, 0x00, 0xda,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData12b[] = {0xff, 0x08, 0xe2, 0xa2, 0xa0, 0x80, 0x02, 0x00, 0x00, 0xf2,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData12c[] = {0xff, 0x08, 0xec, 0x17, 0x00, 0x88, 0x02, 0x00, 0x00, 0xf1,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x06, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3c, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa8, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6b[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa7, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00,
                               0x0d, 0x00, 0x10, 0x00, 0xc0, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
        uint8_t noOpData7b[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00,
                                0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
        uint8_t noOpData8[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x50, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData8b[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x68, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData9[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData10[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa8, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t noOpData9b[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00,
                                0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData10b[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x85, 0xb8, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData9c[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00,
                                0x0d, 0x3e, 0x96, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t noOpData10c[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x85, 0xc2, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        
        if (nordictrack_elliptical_c7_5) {
            max_resistance = 22;
            max_inclination = 20;

            writeCharacteristic(initData10c, sizeof(initData10c), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11c, sizeof(initData11c), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12c, sizeof(initData12c), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData6b, sizeof(noOpData6b), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData7b, sizeof(noOpData7b), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData8b, sizeof(noOpData8b), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData9c, sizeof(noOpData9c), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData10c, sizeof(noOpData10c), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else if (proform_hybrid_trainer_xt) {
            writeCharacteristic(initData10b, sizeof(initData10b), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11b, sizeof(initData11b), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12b, sizeof(initData12b), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData9b, sizeof(noOpData9b), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData10b, sizeof(noOpData10b), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else {
            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        }
    }

    initDone = true;
}

void nordictrackelliptical::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("00001534-1412-efde-1523-785feabcd123"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("00001535-1412-efde-1523-785feabcd123"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &nordictrackelliptical::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &nordictrackelliptical::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &nordictrackelliptical::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &nordictrackelliptical::descriptorWritten);

        // ******************************************* virtual treadmill init *************************************
        QSettings settings;
        if (!firstStateChanged && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &nordictrackelliptical::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &nordictrackelliptical::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset,
                                                       bikeResistanceGain);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &nordictrackelliptical::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstStateChanged = 1;
            }
        }
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void nordictrackelliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void nordictrackelliptical::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                  const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void nordictrackelliptical::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("00001533-1412-efde-1523-785feabcd123"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &nordictrackelliptical::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void nordictrackelliptical::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("nordictrackelliptical::errorService") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void nordictrackelliptical::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("nordictrackelliptical::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void nordictrackelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("I_TL")))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &nordictrackelliptical::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &nordictrackelliptical::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &nordictrackelliptical::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &nordictrackelliptical::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
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
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool nordictrackelliptical::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void nordictrackelliptical::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

int nordictrackelliptical::pelotonToEllipticalResistance(int pelotonResistance) {
    return ceil((pelotonResistance - 10.0) / 3.33);
}
