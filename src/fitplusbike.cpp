#include "fitplusbike.h"
#include "keepawakehelper.h"
#include "virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

fitplusbike::fitplusbike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
                         double bikeResistanceGain) {
#ifdef Q_OS_IOS
    QSettings settings;
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();
    if (sportstech_sx600) {
        QZ_EnableDiscoveryCharsAndDescripttors = false;
    } else {
        QZ_EnableDiscoveryCharsAndDescripttors = true;
    }
#endif
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &fitplusbike::update);
    refresh->start(200ms);
}

void fitplusbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                      bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    // if there are some crash here, maybe it's better to use 2 separate event for the characteristicChanged.
    // one for the resistance changed event (spontaneous), and one for the other ones.
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");
        return;
    }

    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("gattWriteCharacteristic is invalid");
        return;
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic,
                                                         QByteArray((const char *)data, data_len));

    if (!disable_log)
        qDebug() << QStringLiteral(" >> ") + QByteArray((const char *)data, data_len).toHex(' ') +
                        QStringLiteral(" // ") + info;

    loop.exec();
}

void fitplusbike::forceResistance(resistance_t requestResistance) {
    QSettings settings;
    bool virtufit_etappe = settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();
    if (virtufit_etappe || merach_MRK) {
        if (requestResistance == 1) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x01, 0xf9, 0xb9, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 2) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x02, 0xf9, 0xba, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 3) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x03, 0xfa, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 4) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x04, 0xfb, 0xbe, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 5) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x05, 0xfc, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 6) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x06, 0xfd, 0xba, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 7) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x07, 0xfe, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 8) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x08, 0xff, 0xb6, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 9) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x09, 0x00, 0x48, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 10) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0a, 0x00, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 11) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0b, 0x01, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 12) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0c, 0x03, 0x4e, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 13) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0d, 0x03, 0x4f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 14) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0e, 0x05, 0x4a, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 15) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0f, 0x05, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 16) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x10, 0x06, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 17) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x11, 0x07, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 18) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x12, 0x08, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 19) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x13, 0x09, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 20) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x14, 0x0a, 0x5f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 21) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x15, 0x0b, 0x5f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 22) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x16, 0x0c, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 23) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x17, 0x0d, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 24) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x18, 0x0e, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        }
    } else if (sportstech_sx600) {
        if (requestResistance == 1) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x00, 0x00, 0x41, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 2) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x02, 0x00, 0x43, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 3) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x03, 0xfa, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 4) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x04, 0xfb, 0xbe, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 5) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x05, 0x00, 0x44, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 6) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x06, 0xfd, 0xba, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 7) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x07, 0xfe, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 8) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x08, 0x00, 0x49, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 9) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x09, 0x00, 0x48, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 10) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0a, 0x00, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 11) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0b, 0x00, 0x4a, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 12) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0c, 0x03, 0x4e, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 13) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0d, 0x03, 0x4f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 14) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0e, 0x00, 0x4f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 15) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0f, 0x05, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 16) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x10, 0x06, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 17) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x11, 0x00, 0x50, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 18) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x12, 0x08, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 19) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x13, 0x00, 0x52, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 20) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x14, 0x00, 0x55, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 21) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x15, 0x00, 0x54, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 22) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x16, 0x00, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 23) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x17, 0x00, 0x56, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 24) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x18, 0x00, 0x59, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 25) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x19, 0x00, 0x58, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 27) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1a, 0x00, 0x59, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 27) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1b, 0x00, 0x5a, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 28) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1c, 0x00, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 29) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1d, 0x00, 0x5c, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 30) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1e, 0x00, 0x5f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 31) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1f, 0x00, 0x5e, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 32) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x20, 0x00, 0x61, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        }
    }
}

void fitplusbike::update() {
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
        update_metrics(true, watts());
        bool virtufit_etappe =
            settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();
        bool sportstech_sx600 =
            settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();

        if (virtufit_etappe || merach_MRK || sportstech_sx600) {

        } else {

            if (Heart.value() > 0) {
                int avgP = ((settings.value(QZSettings::power_hr_pwr1, QZSettings::default_power_hr_pwr1).toDouble() *
                             settings.value(QZSettings::power_hr_hr2, QZSettings::default_power_hr_hr2).toDouble()) -
                            (settings.value(QZSettings::power_hr_pwr2, QZSettings::default_power_hr_pwr2).toDouble() *
                             settings.value(QZSettings::power_hr_hr1, QZSettings::default_power_hr_hr1).toDouble())) /
                               (settings.value(QZSettings::power_hr_hr2, QZSettings::default_power_hr_hr2).toDouble() -
                                settings.value(QZSettings::power_hr_hr1, QZSettings::default_power_hr_hr1).toDouble()) +
                           (Heart.value() *
                            ((settings.value(QZSettings::power_hr_pwr1, QZSettings::default_power_hr_pwr1).toDouble() -
                              settings.value(QZSettings::power_hr_pwr2, QZSettings::default_power_hr_pwr2).toDouble()) /
                             (settings.value(QZSettings::power_hr_hr1, QZSettings::default_power_hr_hr1).toDouble() -
                              settings.value(QZSettings::power_hr_hr2, QZSettings::default_power_hr_hr2).toDouble())));
                if (avgP < 50) {
                    avgP = 50;
                }
                m_watt = avgP;
                qDebug() << QStringLiteral("Current Watt: ") + QString::number(m_watt.value());
            }
        }

        // sending poll every 2 seconds
        if (sec1Update++ >= (2000 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (sportstech_sx600) {
            uint8_t noOpData[] = {0x02, 0x42, 0x42, 0x03};
            uint8_t noOpData1[] = {0x02, 0x02, 0x43, 0x01, 0x42, 0x03, 0x01, 0x03};
            uint8_t noOpData2[] = {0x02, 0x53, 0x9f, 0x02, 0x00, 0x00, 0xce, 0x03};
            uint8_t noOpData3[] = {0x02, 0x43, 0x01, 0x42, 0x03};
            uint8_t noOpData4[] = {0x02, 0x53, 0x9f, 0x02, 0x00, 0x00, 0xce, 0x03};
            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
                break;
            case 1:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
                break;
            case 2:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"), false, true);
                break;
            case 3:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), false, true);
                break;
            default:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), false, true);
                break;
            }

            counterPoll++;
            if (counterPoll > 4) {
                counterPoll = 0;
            }
        } else {
            uint8_t noOpData[] = {0x02, 0x42, 0x42, 0x03};
            uint8_t noOpData1[] = {0x02, 0x43, 0x01, 0x42, 0x03};
            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
                break;
            default:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
                break;
            }

            counterPoll++;
            if (counterPoll > 1) {
                counterPoll = 0;
            }
        }

        if (requestResistance != -1) {
            if (requestResistance > max_resistance) {
                requestResistance = max_resistance;
            } else if (requestResistance <= 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                qDebug() << QStringLiteral("writing resistance ") + QString::number(requestResistance);
                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestStart != -1) {
            qDebug() << QStringLiteral("starting...");

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            qDebug() << QStringLiteral("stopping...");
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void fitplusbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void fitplusbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << QStringLiteral(" << ") + newValue.toHex(' ');

    lastPacket = newValue;

    bool virtufit_etappe = settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();

    if (sportstech_sx600 && characteristic.uuid() == QBluetoothUuid((quint16)0x2AD2)) {
        bool disable_hr_frommachinery =
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

        union flags {
            struct {
                uint16_t moreData : 1;
                uint16_t avgSpeed : 1;
                uint16_t instantCadence : 1;
                uint16_t avgCadence : 1;
                uint16_t totDistance : 1;
                uint16_t resistanceLvl : 1;
                uint16_t instantPower : 1;
                uint16_t avgPower : 1;
                uint16_t expEnergy : 1;
                uint16_t heartRate : 1;
                uint16_t metabolic : 1;
                uint16_t elapsedTime : 1;
                uint16_t remainingTime : 1;
                uint16_t spare : 3;
            };

            uint16_t word_flags;
        };

        flags Flags;

        int index = 0;
        Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
        index += 2;

        if (!Flags.moreData) {
            if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                  (uint16_t)((uint8_t)newValue.at(index)))) /
                        100.0;
            } else {
                Speed = metric::calculateSpeedFromPower(
                    watts(), Inclination.value(), Speed.value(),
                    fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
            index += 2;
            qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            qDebug() << QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed);
        }

        if (Flags.instantCadence) {
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {

                // this bike sent a cadence 1/10 of the real one
                Cadence = (((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                     (uint16_t)((uint8_t)newValue.at(index)))) /
                           2.0) *
                          settings
                              .value(QZSettings::horizon_gr7_cadence_multiplier,
                                     QZSettings::default_horizon_gr7_cadence_multiplier)
                              .toDouble();
            }
            index += 2;
            qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
        }

        if (Flags.avgCadence) {
            double avgCadence;
            avgCadence = (((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          2.0) *
                         settings
                             .value(QZSettings::horizon_gr7_cadence_multiplier,
                                    QZSettings::default_horizon_gr7_cadence_multiplier)
                             .toDouble();
            index += 2;
            qDebug() << QStringLiteral("Current Average Cadence: ") + QString::number(avgCadence);
        }

        if (Flags.totDistance) {
            index += 3;
        } else {
        }

        qDebug() << QStringLiteral("Current Distance: ") + QString::number(Distance.value());

        if (Flags.resistanceLvl) {
            Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            emit resistanceRead(Resistance.value());
            m_pelotonResistance = Resistance.value();
            index += 2;
            qDebug() << QStringLiteral("Current Resistance: ") + QString::number(Resistance.value());
        }

        if (Flags.instantPower) {
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            qDebug() << (QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
        }

        if (Flags.avgPower) {
            double avgPower;
            avgPower = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            qDebug() << (QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
        }

        if (Flags.expEnergy && newValue.length() > index + 1) {
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
            if (watts())
                KCal += ((((0.048 * ((double)watts()) + 1.19) *
                           settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                          200.0) /
                         (60000.0 /
                          ((double)lastRefreshCharacteristicChanged.msecsTo(
                              QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                // kg * 3.5) / 200 ) / 60
        }

        qDebug() << (QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate && !disable_hr_frommachinery && newValue.length() > index) {
                Heart = ((double)((newValue.at(index))));
                // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
                qDebug() << (QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
            } else {
                Flags.heartRate = false;
            }
        }

        if (Flags.metabolic) {
            // todo
        }

        if (Flags.elapsedTime) {
            // todo
        }

        if (Flags.remainingTime) {
            // todo
        }
    } else if (virtufit_etappe || merach_MRK || (sportstech_sx600 && !gattCommunicationChannelServiceFTMS)) {
        if (newValue.length() != 15 && newValue.length() != 13)
            return;

        if (newValue.length() == 15) {
            Resistance = newValue.at(5);
            if (merach_MRK || sportstech_sx600) {
                // if we change this, also change the wattsFromResistance function. We can create a standard function in
                // order to have all the costants in one place (I WANT MORE TIME!!!)
                double ac = 0.01243107769;
                double bc = 1.145964912;
                double cc = -23.50977444;

                double ar = 0.1469553975;
                double br = -5.841344538;
                double cr = 97.62165482;

                m_pelotonResistance =
                    (((sqrt(pow(br, 2.0) - 4.0 * ar *
                                               (cr - (m_watt.value() * 132.0 /
                                                      (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
                       br) /
                      (2.0 * ar)) *
                     settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                    settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
            } else {
                m_pelotonResistance = (100 * Resistance.value()) / max_resistance;
            }

            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                Cadence = ((uint8_t)newValue.at(6));
            m_watt = (double)((((uint8_t)newValue.at(4)) << 8) | ((uint8_t)newValue.at(3))) / 10.0;

            /*if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool())
                Speed = (double)((((uint8_t)newValue.at(4)) << 10) | ((uint8_t)newValue.at(9))) / 100.0;
            else*/
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());

        } else if (newValue.length() == 13) {

            return;
        }

    } else {

        if (newValue.length() != 14) {
            return;
        }

        /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
            return;*/

        Resistance = 1;
        m_pelotonResistance = 1;
        emit resistanceRead(Resistance.value());
        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")))
            Cadence = ((uint8_t)newValue.at(8));
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool())
            Speed = (double)((((uint8_t)newValue.at(7)) << 8) | ((uint8_t)newValue.at(6))) / 10.0;
        else
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            this->updateLockscreenEnergyDistanceHeartRate();
        }
    }

    this->doPelotonWorkaround();

    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs);
    qDebug() << QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime);
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void fitplusbike::btinit() {

    QSettings settings;
    bool virtufit_etappe = settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();

    if (merach_MRK) {
        uint8_t initData1[] = {0xaa, 0x01, 0x00, 0x01, 0x55};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    } else if (virtufit_etappe) {
        uint8_t initData1[] = {0x02, 0x42, 0x42, 0x03};
        uint8_t initData2[] = {0x02, 0x41, 0x02, 0x43, 0x03};
        uint8_t initData3[] = {0x02, 0x41, 0x03, 0x42, 0x03};
        uint8_t initData4[] = {0x02, 0x44, 0x01, 0x45, 0x03};
        uint8_t initData5[] = {0x02, 0x44, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xaa, 0x18, 0x00, 0xc0, 0x03};
        uint8_t initData6[] = {0x02, 0x44, 0x0b, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x03};
        uint8_t initData7[] = {0x02, 0x44, 0x02, 0x46, 0x03};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);

        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
    } else if (sportstech_sx600) {
        uint8_t initData1[] = {0x02, 0x42, 0x42, 0x03};
        uint8_t initData2[] = {0x02, 0x53, 0x9f, 0x02, 0x00, 0x00, 0xce, 0x03};
        uint8_t initData3[] = {0x02, 0x44, 0x01, 0x45, 0x03};
        uint8_t initData4[] = {0x02, 0x44, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xaa, 0x18, 0x00, 0xc0, 0x03};
        uint8_t initData5[] = {0x02, 0x44, 0x0b, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x03};
        uint8_t initData6[] = {0x02, 0x44, 0x02, 0x46, 0x03};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);

        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData2), QStringLiteral("init"), false, false);

        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData2), QStringLiteral("init"), false, false);

        max_resistance = 32;

    } else {

        uint8_t initData1[] = {0x02, 0x44, 0x01, 0x45, 0x03};
        uint8_t initData2[] = {0x02, 0x44, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x03};
        uint8_t initData3[] = {0x02, 0x44, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x46, 0xaa, 0x19, 0x00, 0xbb, 0x03};
        uint8_t initData4[] = {0x02, 0x44, 0x02, 0x46, 0x03};
        uint8_t initData5[] = {0x02, 0x41, 0x02, 0x43, 0x03};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
    }

    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void fitplusbike::stateChanged(QLowEnergyService::ServiceState state) {
    QSettings settings;
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();

    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (sportstech_sx600 && gattCommunicationChannelServiceFTMS) {
        if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceDiscovered ||
            gattCommunicationChannelServiceFTMS->state() != QLowEnergyService::ServiceDiscovered) {
            qDebug() << "sportstech_sx600 not all services discovered" << gattCommunicationChannelService->state()
                     << gattCommunicationChannelServiceFTMS->state();
            return;
        }
    }

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &fitplusbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &fitplusbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &fitplusbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &fitplusbike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
        if (!this->isVirtualDeviceSetUp() && !virtualBike && !this->isPelotonWorkaroundActive()) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();

            if (virtual_device_enabled) {
                qDebug() << QStringLiteral("creating virtual bike interface...");
                virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&fitplusbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &fitplusbike::changeInclination);
            }
        }
        this->setVirtualDeviceSetUp();
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);

        if (sportstech_sx600 && gattCommunicationChannelServiceFTMS) {
            QBluetoothUuid _gattNotifyFTMSCharacteristicId((quint16)0x2AD2);
            gattNotifyFTMSCharacteristic =
                gattCommunicationChannelServiceFTMS->characteristic(_gattNotifyFTMSCharacteristicId);
            Q_ASSERT(gattNotifyFTMSCharacteristic.isValid());

            connect(gattCommunicationChannelServiceFTMS, &QLowEnergyService::characteristicChanged, this,
                    &fitplusbike::characteristicChanged);
            connect(gattCommunicationChannelServiceFTMS, &QLowEnergyService::characteristicWritten, this,
                    &fitplusbike::characteristicWritten);
            connect(
                gattCommunicationChannelServiceFTMS,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &fitplusbike::errorService);
            connect(gattCommunicationChannelServiceFTMS, &QLowEnergyService::descriptorWritten, this,
                    &fitplusbike::descriptorWritten);

            QByteArray descriptor;
            descriptor.append((char)0x01);
            descriptor.append((char)0x00);
            gattCommunicationChannelServiceFTMS->writeDescriptor(
                gattNotifyFTMSCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        }
    }
}

void fitplusbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void fitplusbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void fitplusbike::serviceScanDone(void) {
    QSettings settings;
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &fitplusbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();

    if (sportstech_sx600) {
        gattCommunicationChannelServiceFTMS = m_control->createServiceObject(QBluetoothUuid((quint16)0x1826));
        if (gattCommunicationChannelServiceFTMS) {
            qDebug() << "FTMS found!";
            connect(gattCommunicationChannelServiceFTMS, &QLowEnergyService::stateChanged, this,
                    &fitplusbike::stateChanged);
            gattCommunicationChannelServiceFTMS->discoverDetails();
        }
    }
}

void fitplusbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("fitplusbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void fitplusbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("fitplusbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void fitplusbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')';
    {
        bluetoothDevice = device;

        if (device.name().startsWith(QStringLiteral("MRK-"))) {
            merach_MRK = true;
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &fitplusbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &fitplusbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &fitplusbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &fitplusbike::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    qDebug() << QStringLiteral("Cannot connect to remote device.");
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("LowEnergy controller disconnected");
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool fitplusbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void *fitplusbike::VirtualBike() { return virtualBike; }

void *fitplusbike::VirtualDevice() { return VirtualBike(); }

uint16_t fitplusbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void fitplusbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
