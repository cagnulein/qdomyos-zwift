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

    QSettings settings;
    bool nordictrack_elliptical_c7_5 =
        settings.value(QZSettings::nordictrack_elliptical_c7_5, QZSettings::default_nordictrack_elliptical_c7_5)
            .toBool();
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
    bool nordictrack_elliptical_c7_5 =
        settings.value(QZSettings::nordictrack_elliptical_c7_5, QZSettings::default_nordictrack_elliptical_c7_5)
            .toBool();
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
        bool nordictrack_elliptical_c7_5 =
            settings.value(QZSettings::nordictrack_elliptical_c7_5, QZSettings::default_nordictrack_elliptical_c7_5)
                .toBool();

        update_metrics(true, watts());

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
    bool nordictrack_elliptical_c7_5 =
        settings.value(QZSettings::nordictrack_elliptical_c7_5, QZSettings::default_nordictrack_elliptical_c7_5)
            .toBool();
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
    bool nordictrack_elliptical_c7_5 =
        settings.value(QZSettings::nordictrack_elliptical_c7_5, QZSettings::default_nordictrack_elliptical_c7_5)
            .toBool();
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
    bool nordictrack_elliptical_c7_5 =
        settings.value(QZSettings::nordictrack_elliptical_c7_5, QZSettings::default_nordictrack_elliptical_c7_5)
            .toBool();

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
        if (nordictrack_se7i) {
            // NordicTrack Elliptical SE7i initialization (19 packets: pkt944 to pkt1020)
            max_resistance = 22;
            max_inclination = 20;

            uint8_t se7i_initData1[] = {0xfe, 0x02, 0x08, 0x02};
            uint8_t se7i_initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t se7i_initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x80, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t se7i_initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x88, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t se7i_initData5[] = {0xfe, 0x02, 0x0b, 0x02}; // pkt972
            uint8_t se7i_initData6[] = {0xff, 0x0b, 0x02, 0x04, 0x02, 0x07, 0x02, 0x07, 0x82, 0x00, 0x00, 0x00, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // pkt975
            uint8_t se7i_initData7[] = {0xfe, 0x02, 0x0a, 0x02}; // pkt982
            uint8_t se7i_initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // pkt985
            uint8_t se7i_initData9[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // pkt994
            uint8_t se7i_initData10[] = {0xfe, 0x02, 0x2c, 0x04}; // pkt1000
            uint8_t se7i_initData11[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x06, 0x28, 0x90, 0x04, 0x00, 0x0d, 0x68, 0xc9, 0x28, 0x95, 0xf0, 0x69, 0xc0, 0x3d}; // pkt1003
            uint8_t se7i_initData12[] = {0x01, 0x12, 0xa8, 0x19, 0x88, 0xf5, 0x60, 0xf9, 0x70, 0xcd, 0x48, 0xc9, 0x48, 0xf5, 0x70, 0xe9, 0x60, 0x1d, 0x88, 0x39}; // pkt1006
            uint8_t se7i_initData13[] = {0xff, 0x08, 0xa8, 0x55, 0xc0, 0x80, 0x02, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // pkt1009
            uint8_t se7i_initData14[] = {0xfe, 0x02, 0x19, 0x03}; // pkt1014
            uint8_t se7i_initData15[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x06, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // pkt1017
            uint8_t se7i_initData16[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // pkt1020

            int sleepms = 400;
            writeCharacteristic(se7i_initData1, sizeof(se7i_initData1), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData2, sizeof(se7i_initData2), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData1, sizeof(se7i_initData1), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData3, sizeof(se7i_initData3), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData1, sizeof(se7i_initData1), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData4, sizeof(se7i_initData4), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData5, sizeof(se7i_initData5), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData6, sizeof(se7i_initData6), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData7, sizeof(se7i_initData7), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData8, sizeof(se7i_initData8), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData1, sizeof(se7i_initData1), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData9, sizeof(se7i_initData9), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData10, sizeof(se7i_initData10), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData11, sizeof(se7i_initData11), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData12, sizeof(se7i_initData12), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData13, sizeof(se7i_initData13), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData14, sizeof(se7i_initData14), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData15, sizeof(se7i_initData15), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
            writeCharacteristic(se7i_initData16, sizeof(se7i_initData16), QStringLiteral("init"), false, true);
            QThread::msleep(sleepms);
        } else if (nordictrack_elliptical_c7_5) {
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
