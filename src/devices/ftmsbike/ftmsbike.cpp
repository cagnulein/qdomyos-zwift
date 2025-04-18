#include "ftmsbike.h"
#include "homeform.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include <QLowEnergyConnectionParameters>
#endif
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include <chrono>
#include "wheelcircumference.h"

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

using namespace std::chrono_literals;

ftmsbike::ftmsbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                   double bikeResistanceGain) {
    QSettings settings;
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &ftmsbike::update);
    refresh->start(settings.value(QZSettings::poll_device_time, QZSettings::default_poll_device_time).toInt());
    wheelCircumference::GearTable g;
    g.printTable();
}

void ftmsbike::writeCharacteristicZwiftPlay(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                   bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    QSettings settings;
    bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();

    if(!zwiftPlayService || !gears_zwift_ratio) {
        qDebug() << QStringLiteral("zwiftPlayService is null!");
        return;
    }

    if (wait_for_response) {
        connect(zwiftPlayService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(zwiftPlayService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (zwiftPlayWriteChar.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        zwiftPlayService->writeCharacteristic(zwiftPlayWriteChar, *writeBuffer,
                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        zwiftPlayService->writeCharacteristic(zwiftPlayWriteChar, *writeBuffer);
    }

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

bool ftmsbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                   bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    QSettings settings;
    bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();

    if(!gattFTMSService) {
        qDebug() << QStringLiteral("gattFTMSService is null!");
        return false;
    }
    
    if(zwiftPlayService && gears_zwift_ratio) {
        qDebug() << QStringLiteral("zwiftPlayService is present!");
        return false;
    }

    if (wait_for_response) {
        connect(gattFTMSService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattFTMSService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharControlPointId.properties() & QLowEnergyCharacteristic::WriteNoResponse && !DOMYOS) {
        gattFTMSService->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer,
                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattFTMSService->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer);
    }

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();

    return true;
}

void ftmsbike::init() {
    if (initDone)
        return;

    if(ICSE) {
        uint8_t write[] = {FTMS_REQUEST_CONTROL};
        bool ret = writeCharacteristic(write, sizeof(write), "requestControl", false, true);
        write[0] = {FTMS_RESET};
        ret = writeCharacteristic(write, sizeof(write), "reset", false, true);
    }
    
    uint8_t write[] = {FTMS_REQUEST_CONTROL};
    bool ret = writeCharacteristic(write, sizeof(write), "requestControl", false, true);
    write[0] = {FTMS_START_RESUME};
    ret = writeCharacteristic(write, sizeof(write), "start simulation", false, true);

    if(ret) {
        initDone = true;
        initRequest = false;
    }
}

ftmsbike::~ftmsbike() {
}

void ftmsbike::zwiftPlayInit() {
    QSettings settings;
    bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();

    if(zwiftPlayService && gears_zwift_ratio) {
        uint8_t rideOn[] = {0x52, 0x69, 0x64, 0x65, 0x4f, 0x6e, 0x02, 0x01};
        writeCharacteristicZwiftPlay(rideOn, sizeof(rideOn), "rideOn", false, true);

        uint8_t init1[] = {0x41, 0x08, 0x05};
        writeCharacteristicZwiftPlay(init1, sizeof(init1), "init1", false, true);

        uint8_t init2[] = {0x04, 0x2a, 0x04, 0x10, 0xc0, 0xbb, 0x01};
        writeCharacteristicZwiftPlay(init2, sizeof(init2), "init2", false, true);

        uint8_t init3[] = {0x00, 0x08, 0x00};
        writeCharacteristicZwiftPlay(init3, sizeof(init3), "init3", false, true);

        writeCharacteristicZwiftPlay(init1, sizeof(init1), "init1", false, true);

        uint8_t init4[] = {0x00, 0x08, 0x88, 0x04};
        writeCharacteristicZwiftPlay(init4, sizeof(init4), "init4", false, true);

        uint8_t init5[] = {0x04, 0x2a, 0x0a, 0x10, 0xc0, 0xbb, 0x01, 0x20, 0xbf, 0x06, 0x28, 0xb4, 0x42};
        writeCharacteristicZwiftPlay(init5, sizeof(init5), "init5", false, true);

        uint8_t init6[] = {0x04, 0x22, 0x0b, 0x08, 0x00, 0x10, 0xda, 0x02, 0x18, 0xec, 0x27, 0x20, 0x90, 0x03};
        writeCharacteristicZwiftPlay(init6, sizeof(init6), "init6", false, true);

        writeCharacteristicZwiftPlay(init2, sizeof(init2), "init2", false, true);
        writeCharacteristicZwiftPlay(init4, sizeof(init4), "init4", false, true);

        uint8_t init7[] = {0x04, 0x22, 0x03, 0x10, 0xa9, 0x01};
        writeCharacteristicZwiftPlay(init7, sizeof(init7), "init7", false, true);

        writeCharacteristicZwiftPlay(init2, sizeof(init2), "init2", false, true);
        writeCharacteristicZwiftPlay(init4, sizeof(init4), "init4", false, true);

        uint8_t init8[] = {0x04, 0x22, 0x02, 0x10, 0x00};
        writeCharacteristicZwiftPlay(init8, sizeof(init8), "init8", false, true);
    }
}

void ftmsbike::forcePower(int16_t requestPower) {
    if(resistance_lvl_mode) { 
        forceResistance(resistanceFromPowerRequest(requestPower));
    } else {
        uint8_t write[] = {FTMS_SET_TARGET_POWER, 0x00, 0x00};

        write[1] = ((uint16_t)requestPower) & 0xFF;
        write[2] = ((uint16_t)requestPower) >> 8;

        writeCharacteristic(write, sizeof(write), QStringLiteral("forcePower ") + QString::number(requestPower));

        powerForced = true;
    }
}

uint16_t ftmsbike::wattsFromResistance(double resistance) {
    if(DU30_bike) {
        double y = 1.46193548 * Cadence.value() + 0.0000887836638 * Cadence.value() * resistance + 0.000625 * resistance * resistance + 0.0580645161 * Cadence.value() + 0.00292986091 * resistance + 6.48448135542904;
        return y;
    }
    return _ergTable.estimateWattage(Cadence.value(), resistance);
}

resistance_t ftmsbike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();

    if (Cadence.value() == 0)
        return 1;

    for (resistance_t i = 1; i < max_resistance; i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << wattsFromResistance(i)
                     << wattsFromResistance(i + 1) << power;
            return i;
        }
    }
    if (power < wattsFromResistance(1))
        return 1;
    else
        return max_resistance;
}

void ftmsbike::forceResistance(resistance_t requestResistance) {

    QSettings settings;
    if (!settings.value(QZSettings::ss2k_peloton, QZSettings::default_ss2k_peloton).toBool() &&
        resistance_lvl_mode == false && _3G_Cardio_RB == false && JFBK5_0 == false) {
        uint8_t write[] = {FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS, 0x00, 0x00, 0x00, 0x00, 0x28, 0x19};

        double fr = (((double)requestResistance) * bikeResistanceGain) + ((double)bikeResistanceOffset);
        requestResistance = fr;

        write[3] = ((uint16_t)requestResistance * 10) & 0xFF;
        write[4] = ((uint16_t)requestResistance * 10) >> 8;

        writeCharacteristic(write, sizeof(write),
                            QStringLiteral("forceResistance ") + QString::number(requestResistance));
    } else {
        if(SL010)
            Resistance = requestResistance;
        
        if(JFBK5_0 || DIRETO_XR) {
            uint8_t write[] = {FTMS_SET_TARGET_RESISTANCE_LEVEL, 0x00, 0x00};
            write[1] = ((uint16_t)requestResistance * 10) & 0xFF;
            write[2] = ((uint16_t)requestResistance * 10) >> 8;
            writeCharacteristic(write, sizeof(write),
                                QStringLiteral("forceResistance ") + QString::number(requestResistance));
        } else {
            uint8_t write[] = {FTMS_SET_TARGET_RESISTANCE_LEVEL, 0x00};
            if(_3G_Cardio_RB || SL010)
                requestResistance = requestResistance * 10;
            write[1] = ((uint8_t)(requestResistance));
            writeCharacteristic(write, sizeof(write),
                                QStringLiteral("forceResistance ") + QString::number(requestResistance));
        }
    }
}

void ftmsbike::update() {
    if (!m_control)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        zwiftPlayInit();
        if(ICSE)
            requestResistance = 1;  // to force the engine to send every second a target inclination
        initRequest = false;
    } else if (bluetoothDevice.isValid() &&
               m_control->state() == QLowEnergyController::DiscoveredState //&&
                                                                           // gattCommunicationChannelService &&
                                                                           // gattWriteCharacteristic.isValid() &&
                                                                           // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {
        update_metrics(false, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (powerForced && !autoResistance()) {
            qDebug() << QStringLiteral("disabling resistance ") << QString::number(currentResistance().value());
            powerForced = false;
            requestPower = -1;
            init();
            forceResistance(currentResistance().value());
        }

        auto virtualBike = this->VirtualBike();
        QSettings settings;
        bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();

        if (requestResistance != -1 || lastGearValue != gears()) {
            if (requestResistance > 100) {
                requestResistance = 100;
            } // TODO, use the bluetooth value
            else if (requestResistance == 0) {
                requestResistance = 1;
            }

            double gearMultiplier = 5;
            if(REEBOK)
                gearMultiplier = 1;
            resistance_t rR = requestResistance + (gears() * gearMultiplier);

            if (rR != currentResistance().value() || lastGearValue != gears()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                // if the FTMS is connected, the ftmsCharacteristicChanged event will do all the stuff because it's a
                // FTMS bike. This condition handles the peloton requests                
                if (((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike || resistance_lvl_mode) &&
                    (requestPower == 0 || requestPower == -1 || resistance_lvl_mode)) {
                    init();

                    forceResistance(rR);
                }
            }
            if(!ICSE)
                requestResistance = -1;
        }
        
        if((virtualBike && virtualBike->ftmsDeviceConnected()) && lastGearValue != gears() && lastRawRequestedInclinationValue != -100 && lastPacketFromFTMS.length() >= 7) {
            qDebug() << "injecting fake ftms frame in order to send the new gear value ASAP" << lastPacketFromFTMS.toHex(' ');
            ftmsCharacteristicChanged(QLowEnergyCharacteristic(), lastPacketFromFTMS);
        }

        if(zwiftPlayService && gears_zwift_ratio && lastGearValue != gears()) {
            QSettings settings;
            wheelCircumference::GearTable table;
            wheelCircumference::GearTable::GearInfo g = table.getGear((int)gears());
            double original_ratio = ((double)settings.value(QZSettings::gear_crankset_size, QZSettings::default_gear_crankset_size).toDouble()) /
            ((double)settings.value(QZSettings::gear_cog_size, QZSettings::default_gear_cog_size).toDouble());
            
            double current_ratio = ((double)g.crankset / (double)g.rearCog);
            
            uint32_t gear_value = static_cast<uint32_t>(10000.0 * (current_ratio/original_ratio) * (42.0/14.0));
            
            qDebug() << "zwift hub gear current ratio" << current_ratio << g.crankset << g.rearCog << "gear_value" << gear_value << "original_ratio" << original_ratio;
 
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            QByteArray proto = lockscreen::zwift_hub_setGearsCommand(gear_value);
#else
            QByteArray proto;
#endif
#elif defined Q_OS_ANDROID
            QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod(
                "org/cagnulen/qdomyoszwift/ZwiftHubBike",
                "setGearCommand",
                "(I)[B",
                gear_value);

            if (!result.isValid()) {
                qDebug() << "setGearCommand returned invalid value";
                return;
            }

            jbyteArray array = result.object<jbyteArray>();
            QAndroidJniEnvironment env;
            jbyte* bytes = env->GetByteArrayElements(array, nullptr);
            jsize length = env->GetArrayLength(array);

            QByteArray proto((char*)bytes, length);

            env->ReleaseByteArrayElements(array, bytes, JNI_ABORT);
#else
            QByteArray proto;
            qDebug() << "ERROR: gear message not handled!";
            return;
#endif
            writeCharacteristicZwiftPlay((uint8_t*)proto.data(), proto.length(), "gear", false, true);

            uint8_t gearApply[] = {0x00, 0x08, 0x88, 0x04};
            writeCharacteristicZwiftPlay(gearApply, sizeof(gearApply), "gearApply", false, true);
        }

        lastGearValue = gears();

        if (requestPower != -1) {
            qDebug() << QStringLiteral("writing power") << requestPower;
            init();
            forcePower(requestPower);
            requestPower = -1;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;

            QSettings settings;
            if (settings.value(QZSettings::ss2k_peloton, QZSettings::default_ss2k_peloton).toBool()) {
                uint8_t write[] = {FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS, 0x00, 0x00, 0x00, 0x00, 0x28, 0x19};

                writeCharacteristic(write, sizeof(write), QStringLiteral("init SS2K"));
            }
        }
    }
}

void ftmsbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void ftmsbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    bool heart = false;
    bool watt_ignore_builtin =
        settings.value(QZSettings::watt_ignore_builtin, QZSettings::default_watt_ignore_builtin).toBool();

    qDebug() << characteristic.uuid() << newValue.length() << QStringLiteral(" << ") << newValue.toHex(' ');

    lastPacket = newValue;

    if (DU30_bike && characteristic.uuid() == QBluetoothUuid(QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb")) && newValue.length() >= 14) {
        resistance_received = true;
        Resistance = (double)(newValue.at(5));
        emit resistanceRead(Resistance.value());
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        return;
    }

    if (characteristic.uuid() == QBluetoothUuid((quint16)0x2A19) && !D2RIDE) { // Battery Service
        if(newValue.length() > 0) {
            uint8_t b = (uint8_t)newValue.at(0);
            if(b != battery_level)
                if(homeform::singleton())
                    homeform::singleton()->setToastRequested(bluetoothDevice.name() + QStringLiteral(" Battery Level ") + QString::number(b) + " %");
            battery_level = b;
        }
        return;
    }
    
    if(characteristic.uuid() == QBluetoothUuid(QStringLiteral("00000002-19ca-4651-86e5-fa29dcdd09d1")) && newValue.at(0) == 0x03) {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        m_watt =  lockscreen::zwift_hub_getPowerFromBuffer(newValue.mid(1));
        qDebug() << "Current power: " << m_watt.value();
        
        Cadence =  lockscreen::zwift_hub_getCadenceFromBuffer(newValue.mid(1));
        qDebug() << "Current cadence: " << Cadence.value();
#endif
#endif
        return;
    }

    if(T2 && characteristic.uuid() == QBluetoothUuid(QStringLiteral("6e400003-b5a3-f393-e0a9-e50e24dcca9e")) && newValue.length() == 62) {
        int16_t gears = ((int16_t)(((int16_t)((uint8_t)newValue.at(55)) << 8) |
                          (int16_t)((uint8_t)newValue.at(54))));

        qDebug() << QStringLiteral("T2 gears event, actual gear") << gears << QStringLiteral("previous value") << T2_lastGear;

        if (gears < T2_lastGear) {
            for (int i = 0; i < T2_lastGear - gears; ++i) {
                gearDown();
            }
        } else if (gears > T2_lastGear) {
            for (int i = 0; i < gears - T2_lastGear; ++i) {
                gearUp();
            }
        }

        T2_lastGear = gears;
        return;
    }

    // Wattbike Atom First Generation - Display Gears
    if(WATTBIKE && characteristic.uuid() == QBluetoothUuid(QStringLiteral("b4cc1224-bc02-4cae-adb9-1217ad2860d1")) &&
        newValue.length() > 3 && newValue.at(1) == 0x03 && (uint8_t)newValue.at(2) == 0xb6) {
        uint8_t gear = newValue.at(3);
        qDebug() << "watt bike gears" << gear;
        setGears(gear);
    }

    if (characteristic.uuid() == QBluetoothUuid((quint16)0x2AD2)) {

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

        // clean time in case for a long period we don't receive values
        if(lastRefreshCharacteristicChanged2AD2.secsTo(now) > 5) {
            qDebug() << "clearing lastRefreshCharacteristicChanged2AD2" << lastRefreshCharacteristicChanged2AD2 << now;
            lastRefreshCharacteristicChanged2AD2 = now;
        }

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
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
            index += 2;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.instantCadence) {
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          2.0;
            }
            index += 2;
            emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        }

        if (Flags.avgCadence) {
            double avgCadence;
            avgCadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index)))) /
                         2.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Cadence: ") + QString::number(avgCadence));
        }

        if (Flags.totDistance) {

            /*
             * the distance sent from the most trainers is a total distance, so it's useless for QZ
             *
            Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                                  (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint32_t)((uint8_t)newValue.at(index)))) /
                       1000.0;*/
            index += 3;
        }

        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged2AD2.msecsTo(now)));

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.resistanceLvl) {
            double d = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));            
            index += 2;            
            if(Resistance.value() > 0) {
                if(BIKE_)
                    d = d / 10.0;
                // for this bike, i will use the resistance that I set directly because the bike sends a different ratio.
                if(!SL010)
                    Resistance = d;
                emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
                emit resistanceRead(Resistance.value());
                resistance_received = true;
            }
        }
            double ac = 0.01243107769;
            double bc = 1.145964912;
            double cc = -23.50977444;

            double ar = 0.1469553975;
            double br = -5.841344538;
            double cr = 97.62165482;

            if (Cadence.value() && m_watt.value()) {
                m_pelotonResistance =
                    (((sqrt(pow(br, 2.0) - 4.0 * ar *
                                               (cr - (m_watt.value() * 132.0 /
                                                      (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
                       br) /
                      (2.0 * ar)) *
                     settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
                if (!resistance_received && !DU30_bike && !SL010) {
                    Resistance = m_pelotonResistance;
                    emit resistanceRead(Resistance.value());
                    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
                }
            }
   

        if (Flags.instantPower) {
            // power table from an user
            if(DU30_bike) {
                m_watt = wattsFromResistance(Resistance.value());
                emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
            } else if (LYDSTO && watt_ignore_builtin) {
                m_watt = wattFromHR(true);
                emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
            } else if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
        } else if(DOMYOS) {
            // doesn't send power at all and the resistance either
            m_watt = wattFromHR(true);
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
        }

        if (Flags.avgPower && newValue.length() > index + 1) {
            double avgPower;
            avgPower = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
        }

        if (Flags.expEnergy && newValue.length() > index + 1) {
            /*KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));*/
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        }

        if (watts())
            KCal += ((((0.048 * ((double)watts()) + 1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged2AD2.msecsTo(
                          now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate && !disable_hr_frommachinery && newValue.length() > index) {
                Heart = ((double)(((uint8_t)newValue.at(index))));
                // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
                emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
            } else {
                Flags.heartRate = false;
            }
            heart = Flags.heartRate;
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

        lastRefreshCharacteristicChanged2AD2 = now;
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACE)) {
        union flags {
            struct {
                uint32_t moreData : 1;
                uint32_t avgSpeed : 1;
                uint32_t totDistance : 1;
                uint32_t stepCount : 1;
                uint32_t strideCount : 1;
                uint32_t elevationGain : 1;
                uint32_t rampAngle : 1;
                uint32_t resistanceLvl : 1;
                uint32_t instantPower : 1;
                uint32_t avgPower : 1;
                uint32_t expEnergy : 1;
                uint32_t heartRate : 1;
                uint32_t metabolicEq : 1;
                uint32_t elapsedTime : 1;
                uint32_t remainingTime : 1;
                uint32_t movementDirection : 1;
                uint32_t spare : 8;
            };

            uint32_t word_flags;
        };

        flags Flags;
        int index = 0;
        Flags.word_flags = (newValue.at(2) << 16) | (newValue.at(1) << 8) | newValue.at(0);
        index += 3;

        if (!Flags.moreData) {
            if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                  (uint16_t)((uint8_t)newValue.at(index)))) /
                        100.0;
            } else {
                Speed = metric::calculateSpeedFromPower(
                    watts(), Inclination.value(), Speed.value(),
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
            index += 2;
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.totDistance) {
            Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                                  (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint32_t)((uint8_t)newValue.at(index)))) /
                       1000.0;
            index += 3;
        } else {
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged2ACE.msecsTo(now)));
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.stepCount) {
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index))));
            }
            emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

            index += 2;
            index += 2;
        }

        if (Flags.strideCount) {
            index += 2;
        }

        if (Flags.elevationGain) {
            index += 2;
            index += 2;
        }

        if (Flags.rampAngle) {
            index += 2;
            index += 2;
        }

        if (Flags.resistanceLvl) {
            Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            emit resistanceRead(Resistance.value());
            index += 2;
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        } else if(!DU30_bike) {
            double ac = 0.01243107769;
            double bc = 1.145964912;
            double cc = -23.50977444;

            double ar = 0.1469553975;
            double br = -5.841344538;
            double cr = 97.62165482;

            if (Cadence.value() && m_watt.value()) {
                m_pelotonResistance =
                    (((sqrt(pow(br, 2.0) - 4.0 * ar *
                                               (cr - (m_watt.value() * 132.0 /
                                                      (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
                       br) /
                      (2.0 * ar)) *
                     settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                    settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
                Resistance = m_pelotonResistance;
                emit resistanceRead(Resistance.value());
            }
        }

        if (Flags.instantPower) {
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
            index += 2;
        }

        if (Flags.avgPower && newValue.length() > index + 1) {
            double avgPower;
            avgPower = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index))));
            emit debug(QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
            index += 2;
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
                          ((double)lastRefreshCharacteristicChanged2ACE.msecsTo(
                              now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                // kg * 3.5) / 200 ) / 60
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate && !disable_hr_frommachinery && newValue.length() > index) {
                Heart = ((double)(((uint8_t)newValue.at(index))));
                // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
                emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
            } else {
                Flags.heartRate = false;
            }
            heart = Flags.heartRate;
        }

        if (Flags.metabolicEq) {
            // todo
        }

        if (Flags.elapsedTime) {
            // todo
        }

        if (Flags.remainingTime) {
            // todo
        }

        lastRefreshCharacteristicChanged2ACE = now;
    } else {
        return;
    }

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) &&
        (!heart || Heart.value() == 0 || disable_hr_frommachinery)) {
        update_hr_from_external();
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void ftmsbike::stateChanged(QLowEnergyService::ServiceState state) {
    QSettings settings;
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::RemoteServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    if (state != QLowEnergyService::ServiceState::ServiceDiscovered) {
        qDebug() << QStringLiteral("ignoring this state");
        return;
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::RemoteServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &ftmsbike::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &ftmsbike::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &ftmsbike::characteristicRead);
            connect(
                s, &QLowEnergyService::errorOccurred,
                this, &ftmsbike::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &ftmsbike::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &ftmsbike::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            if (ICSE) {
                QBluetoothUuid ftmsService((quint16)0x1826);
                QBluetoothUuid CSCService((quint16)0x1816);
                if (s->serviceUuid() != ftmsService && s->serviceUuid() != CSCService) {
                    qDebug() << QStringLiteral("ICSE bike wants to be subscribed only to FTMS and CSC services in order "
                                               "to send metrics")
                             << s->serviceUuid();
                    continue;
                }
            }
            
            if (settings.value(QZSettings::hammer_racer_s, QZSettings::default_hammer_racer_s).toBool() || SCH_190U || DOMYOS || SMB1) {
                QBluetoothUuid ftmsService((quint16)0x1826);
                if (s->serviceUuid() != ftmsService) {
                    qDebug() << QStringLiteral("hammer racer bike wants to be subscribed only to FTMS service in order "
                                               "to send metrics")
                             << s->serviceUuid();
                    continue;
                }
            }

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << c.properties();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                           QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }

                QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << QStringLiteral("FTMS service and Control Point found");
                    gattWriteCharControlPointId = c;
                    gattFTMSService = s;
                }

                QBluetoothUuid _zwiftPlayWriteCharControlPointId(QStringLiteral("00000003-19ca-4651-86e5-fa29dcdd09d1"));
                if (c.uuid() == _zwiftPlayWriteCharControlPointId) {
                    qDebug() << QStringLiteral("Zwift Play service and Control Point found");
                    zwiftPlayWriteChar = c;
                    zwiftPlayService = s;
                }
            }
        }
    }

    if(gattFTMSService == nullptr && DOMYOS) {
        settings.setValue(QZSettings::domyosbike_notfmts, true);
        if(homeform::singleton())
            homeform::singleton()->setToastRequested("Domyos bike presents itself like a FTMS but it's not. Restart QZ to apply the fix, thanks.");
    }

    if (gattFTMSService && gattWriteCharControlPointId.isValid() &&
        (settings.value(QZSettings::hammer_racer_s, QZSettings::default_hammer_racer_s).toBool() || SMB1)) {
        init();
    }

    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {
        QSettings settings;
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence =
            settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence) {
            qDebug() << "ios_peloton_workaround activated!";
            h = new lockscreen();
            h->virtualbike_ios();
        } else
#endif
#endif
            if (virtual_device_enabled) {
            emit debug(QStringLiteral("creating virtual bike interface..."));
            auto virtualBike =
                new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
            // connect(virtualBike,&virtualbike::debug ,this,&ftmsbike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &ftmsbike::changeInclination);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &ftmsbike::ftmsCharacteristicChanged);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void ftmsbike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    if (!autoResistance() || resistance_lvl_mode) {
        qDebug() << "ignoring routing FTMS packet to the bike from virtualbike because of auto resistance OFF or resistance lvl mode is on"
                 << characteristic.uuid() << newValue.toHex(' ');
        return;
    }

    QByteArray b = newValue;
    QSettings settings;
    bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();

    if (gattWriteCharControlPointId.isValid()) {
        qDebug() << "routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');

        // handling gears
        if (b.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS && (zwiftPlayService == nullptr || !gears_zwift_ratio)) {
            double min_inclination = settings.value(QZSettings::min_inclination, QZSettings::default_min_inclination).toDouble();
            double offset =
                settings.value(QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset).toDouble();
            double gain =
                settings.value(QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain).toDouble();

            lastPacketFromFTMS.clear();
            for(int i=0; i<b.length(); i++)
                lastPacketFromFTMS.append(b.at(i));
            qDebug() << "lastPacketFromFTMS" << lastPacketFromFTMS.toHex(' ');
            int16_t slope = (((uint8_t)b.at(3)) + (b.at(4) << 8));

            if (gears() != 0) {
                slope += (gears() * 50);
            }

            if(min_inclination > (((double)slope) / 100.0)) {
                slope = min_inclination * 100;
                qDebug() << "grade override due to min_inclination " << min_inclination;
            }

            slope *= gain;
            slope += (offset * 100);

            b[3] = slope & 0xFF;
            b[4] = slope >> 8;

            qDebug() << "applying gears mod" << gears() << slope;

        } else if(b.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS && zwiftPlayService != nullptr && gears_zwift_ratio) {
            int16_t slope = (((uint8_t)b.at(3)) + (b.at(4) << 8));
            
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            QByteArray message = lockscreen::zwift_hub_inclinationCommand(((double)slope) / 100.0);
#else
            QByteArray message;
#endif
#elif defined(Q_OS_ANDROID)
            QAndroidJniObject result = QAndroidJniObject::callStaticObjectMethod(
                "org/cagnulen/qdomyoszwift/ZwiftHubBike",
                "inclinationCommand",
                "(D)[B",
                ((double)slope) / 100.0);

            if(!result.isValid()) {
                qDebug() << "inclinationCommand returned invalid value";
                return;
            }

            jbyteArray array = result.object<jbyteArray>();
            QAndroidJniEnvironment env;
            jbyte* bytes = env->GetByteArrayElements(array, nullptr);
            jsize length = env->GetArrayLength(array);

            QByteArray message((char*)bytes, length);

            env->ReleaseByteArrayElements(array, bytes, JNI_ABORT);
#else
            QByteArray message;
            qDebug() << "implement zwift hub protobuf!";
            return;
#endif
            writeCharacteristicZwiftPlay((uint8_t*)message.data(), message.length(), "gearInclination", false, false);
            return;
        } else if(b.at(0) == FTMS_SET_TARGET_POWER && zwiftPlayService != nullptr && gears_zwift_ratio) {
            qDebug() << "discarding";
            return;
        }
        // gears on erg mode is quite useless and it's confusing
        /* else if(b.at(0) == FTMS_SET_TARGET_POWER && b.length() > 2) {
            lastPacketFromFTMS.clear();
            for(int i=0; i<b.length(); i++)
                lastPacketFromFTMS.append(b.at(i));
            qDebug() << "lastPacketFromFTMS" << lastPacketFromFTMS.toHex(' ');
            int16_t power = (((uint8_t)b.at(1)) + (b.at(2) << 8));
            if (gears() != 0) {
                power += (gears() * 10);
            }
            b[1] = power & 0xFF;
            b[2] = power >> 8;
            qDebug() << "applying gears mod" << gears() << gearsZwiftRatio() << power;
        }*/

        writeCharacteristic((uint8_t*)b.data(), b.length(), "injectWrite ", false, true);
    }
}

void ftmsbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void ftmsbike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void ftmsbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void ftmsbike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void ftmsbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    initRequest = false;
    auto services_list = m_control->services();
    QBluetoothUuid ftmsService((quint16)0x1826);
    bool JK_fitness_577 = bluetoothDevice.name().toUpper().startsWith("DHZ-");
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        if ((JK_fitness_577 && s == ftmsService) || !JK_fitness_577) {
            gattCommunicationChannelService.append(m_control->createServiceObject(s));
            connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                    &ftmsbike::stateChanged);
            gattCommunicationChannelService.constLast()->discoverDetails();

            // watt bikes has the 6 as default gear value
            if(s == QBluetoothUuid(QStringLiteral("b4cc1223-bc02-4cae-adb9-1217ad2860d1")) && SS2K == false) {
                WATTBIKE = true;
                qDebug() << QStringLiteral("restoring gear 6 to watt bikes");
                setGears(6);
            }
        }
    }
}

void ftmsbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("ftmsbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void ftmsbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("ftmsbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

resistance_t ftmsbike::pelotonToBikeResistance(int pelotonResistance) {
    return (pelotonResistance * max_resistance) / 100;
}

void ftmsbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        if (bluetoothDevice.name().toUpper().startsWith("SUITO")) {
            qDebug() << QStringLiteral("SUITO found");
            max_resistance = 16;
        } else if ((bluetoothDevice.name().toUpper().startsWith("MAGNUS "))) {
            qDebug() << QStringLiteral("MAGNUS found");
            resistance_lvl_mode = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("DU30-"))) {
            qDebug() << QStringLiteral("DU30 found");
            max_resistance = 32;
            DU30_bike = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("ICSE") && bluetoothDevice.name().length() == 4)) {
            qDebug() << QStringLiteral("ICSE found");
            ICSE = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("DOMYOS"))) {
            qDebug() << QStringLiteral("DOMYOS found");
            resistance_lvl_mode = true;
            DOMYOS = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("3G Cardio RB"))) {
            qDebug() << QStringLiteral("_3G_Cardio_RB found");
            _3G_Cardio_RB = true;
        } else if((bluetoothDevice.name().toUpper().startsWith("SCH_190U"))) {
            qDebug() << QStringLiteral("SCH_190U found");
            SCH_190U = true;
        } else if(bluetoothDevice.name().toUpper().startsWith("D2RIDE")) {
            qDebug() << QStringLiteral("D2RIDE found");
            D2RIDE = true;
        } else if(bluetoothDevice.name().toUpper().startsWith("VFSPINBIKE")) {
            qDebug() << QStringLiteral("VFSPINBIKE found");
            VFSPINBIKE = true;
        } else if(bluetoothDevice.name().toUpper().startsWith("SMARTSPIN2K")) {
            qDebug() << QStringLiteral("SS2K found");
            SS2K = true;
        } else if(bluetoothDevice.name().toUpper().startsWith("DIRETO XR")) {
            qDebug() << QStringLiteral("DIRETO XR found");
            DIRETO_XR = true;
        } else if(bluetoothDevice.name().toUpper().startsWith("JFBK5.0") || bluetoothDevice.name().toUpper().startsWith("JFBK7.0")) {
            qDebug() << QStringLiteral("JFBK5.0 found");
            resistance_lvl_mode = true;
            JFBK5_0 = true;
        } else if((bluetoothDevice.name().toUpper().startsWith("BIKE-"))) {
            qDebug() << QStringLiteral("BIKE- found");
            BIKE_ = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("SMB1"))) {
            qDebug() << QStringLiteral("SMB1 found");
            SMB1 = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("SPAX-BK-"))) {
            qDebug() << QStringLiteral("SPAX-BK found");
            resistance_lvl_mode = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("LYDSTO"))) {
            qDebug() << QStringLiteral("LYDSTO found");
            LYDSTO = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("SL010-"))) {
            qDebug() << QStringLiteral("SL010 found");
            SL010 = true;
            max_resistance = 25;
            resistance_lvl_mode = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("REEBOK"))) {
            qDebug() << QStringLiteral("REEBOK found");
            REEBOK = true;
            max_resistance = 32;
            resistance_lvl_mode = true;
        } else if ((bluetoothDevice.name().toUpper().startsWith("T2 "))) {
            qDebug() << QStringLiteral("T2 found");
            T2 = true;
        }
        
        if(settings.value(QZSettings::force_resistance_instead_inclination, QZSettings::default_force_resistance_instead_inclination).toBool()) {
            resistance_lvl_mode = true;
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &ftmsbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &ftmsbike::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &ftmsbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &ftmsbike::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
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

bool ftmsbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void ftmsbike::setWheelDiameter(double diameter) {
    uint8_t write[] = {FTMS_SET_WHEEL_CIRCUMFERENCE, 0x00, 0x00};

    diameter = diameter * 10.0;

    write[1] = ((uint16_t)diameter) & 0xFF;
    write[2] = ((uint16_t)diameter) >> 8;

    writeCharacteristic(write, sizeof(write), QStringLiteral("setWheelCircumference ") + QString::number(diameter));
}

uint16_t ftmsbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void ftmsbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

double ftmsbike::maxGears() {
    QSettings settings;
    bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();

    if((zwiftPlayService != nullptr) && gears_zwift_ratio) {
        wheelCircumference::GearTable g;
        return g.maxGears;
    } else if(WATTBIKE) {
        return 22;
    } else {
        return 9999.0;
    }
}

double ftmsbike::minGears() {
    QSettings settings;
    bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();

    if((zwiftPlayService != nullptr) && gears_zwift_ratio ) {
        return 1;
    } else if(WATTBIKE) {
        return 1;
    } else {
        return -9999.0;
    }
}
