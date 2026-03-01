#include "virtualdevices/virtualbike.h"
#include "devices/bike.h"
#include <QThread>
#include <QDataStream>
#include <QMetaEnum>
#include <QSettings>
#include <QtMath>
#include <chrono>

using namespace std::chrono_literals;

virtualbike::virtualbike(bluetoothdevice *t, bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                         double bikeResistanceGain) {
    Bike = t;

    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    QSettings settings;
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool bike_wheel_revs = settings.value(QZSettings::bike_wheel_revs, QZSettings::default_bike_wheel_revs).toBool();
    bool power = settings.value(QZSettings::bike_power_sensor, QZSettings::default_bike_power_sensor).toBool();
    bool battery = settings.value(QZSettings::battery_service, QZSettings::default_battery_service).toBool();
    bool service_changed = settings.value(QZSettings::service_changed, QZSettings::default_service_changed).toBool();
    bool heart_only =
        settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();
    bool echelon =
        settings.value(QZSettings::virtual_device_echelon, QZSettings::default_virtual_device_echelon).toBool();
    bool ifit = settings.value(QZSettings::virtual_device_ifit, QZSettings::default_virtual_device_ifit).toBool();
    bool garmin_bluetooth_compatibility = settings.value(QZSettings::garmin_bluetooth_compatibility, QZSettings::default_garmin_bluetooth_compatibility).toBool();
    bool zwift_play_emulator = settings.value(QZSettings::zwift_play_emulator, QZSettings::default_zwift_play_emulator).toBool();
    bool watt_bike_emulator = settings.value(QZSettings::watt_bike_emulator, QZSettings::default_watt_bike_emulator).toBool();

    if (settings.value(QZSettings::dircon_yes, QZSettings::default_dircon_yes).toBool()) {
        dirconManager = new DirconManager(Bike, bikeResistanceOffset, bikeResistanceGain, this);
        connect(dirconManager, SIGNAL(changeInclination(double, double)), this,
                SIGNAL(changeInclination(double, double)));
        connect(dirconManager, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
                SLOT(dirconFtmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
        connect(dirconManager, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
                SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    }
    if (!settings.value(QZSettings::virtual_device_bluetooth, QZSettings::default_virtual_device_bluetooth).toBool())
        return;
    notif2AD2 = new CharacteristicNotifier2AD2(Bike, this);
    notif2AD9 = new CharacteristicNotifier2AD9(Bike, this);
    notif2A63 = new CharacteristicNotifier2A63(Bike, this);
    notif2A37 = new CharacteristicNotifier2A37(Bike, this);
    notif2A5B = new CharacteristicNotifier2A5B(Bike, this);
    notif0002 = new CharacteristicNotifier0002(Bike, this);
    notif0004 = new CharacteristicNotifier0004(Bike, this);
    writeP2AD9 = new CharacteristicWriteProcessor2AD9(bikeResistanceGain, bikeResistanceOffset, Bike, notif2AD9, this);
    writeP0003 = new CharacteristicWriteProcessor0003(bikeResistanceGain, bikeResistanceOffset, Bike, notif0002, notif0004, this);
    connect(writeP2AD9, SIGNAL(changeInclination(double, double)), this, SIGNAL(changeInclination(double, double)));
    Q_UNUSED(noWriteResistance)

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if ((ios_peloton_workaround && !cadence && !echelon && !ifit && !heart_only) || garmin_bluetooth_compatibility) {

        qDebug() << "ios_zwift_workaround activated!";
        h = new lockscreen();
        h->virtualbike_zwift_ios(
                                 settings.value(QZSettings::bike_heartrate_service, QZSettings::default_bike_heartrate_service).toBool(), garmin_bluetooth_compatibility, zwift_play_emulator, watt_bike_emulator);
    } else

#endif
#endif
    {
        //! [Advertising Data]
        advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
        advertisingData.setIncludePowerLevel(true);
        if (!echelon && !ifit) {
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
        advertisingData.setLocalName(QStringLiteral("QZPI"));
#else            
        advertisingData.setLocalName(QStringLiteral("QZ"));
#endif
        } else if (ifit) {
            advertisingData.setLocalName(QStringLiteral("I_EB"));
        } else {
            advertisingData.setLocalName(QStringLiteral("ECHEX-5s-113399"));
        }
        QList<QBluetoothUuid> services;

        if (!echelon && !ifit) {
            if (!heart_only) {
                if (!cadence && !power) {
                    services << ((QBluetoothUuid::ServiceClassUuid)0x1826);
                } // FitnessMachineServiceUuid
                else if (power) {

                    services << (QBluetoothUuid::ServiceClassUuid::CyclingPower);
                } else {
                    services << (QBluetoothUuid::ServiceClassUuid::CyclingSpeedAndCadence);
                }
            }
            if (!this->noHeartService || heart_only) {
                services << QBluetoothUuid::ServiceClassUuid::HeartRate;
            }
        } else if (ifit) {
            services << (QBluetoothUuid(QStringLiteral("00001533-1412-efde-1523-785feabcd123")));

            this->noHeartService = true;
            if (!this->noHeartService) {
                services << QBluetoothUuid::ServiceClassUuid::HeartRate;
            }
        } else {
            services << (QBluetoothUuid(QStringLiteral("0bf669f0-45f2-11e7-9598-0800200c9a66")));

            if (!this->noHeartService) {
                services << QBluetoothUuid::ServiceClassUuid::HeartRate;
            }
        }

        advertisingData.setServices(services);
        //! [Advertising Data]

        if (!echelon && !ifit) {
            if (!heart_only) {
                if (!cadence && !power) {

                    serviceDataFIT.setType(QLowEnergyServiceData::ServiceTypePrimary);
                    QLowEnergyCharacteristicData charDataFIT;
                    charDataFIT.setUuid(
                        (QBluetoothUuid::CharacteristicType)0x2ACC); // FitnessMachineFeatureCharacteristicUuid
                    QByteArray valueFIT;
                    valueFIT.append((char)0x83); // average speed, cadence and resistance level supported
                    valueFIT.append((char)0x14); // heart rate and elapsed time
                    valueFIT.append((char)0x00);
                    valueFIT.append((char)0x00);
                    valueFIT.append((char)0x0C); // resistance and power target supported
                    valueFIT.append((char)0xE0); // indoor simulation, wheel and spin down supported
                    valueFIT.append((char)0x00);
                    valueFIT.append((char)0x00);
                    charDataFIT.setValue(valueFIT);
                    charDataFIT.setProperties(QLowEnergyCharacteristic::Read);

                    QLowEnergyCharacteristicData charDataFIT2;
                    charDataFIT2.setUuid(
                        (QBluetoothUuid::
                             CharacteristicType)0x2AD6); // supported_resistance_level_rangeCharacteristicUuid
                    charDataFIT2.setProperties(QLowEnergyCharacteristic::Read);
                    QByteArray valueFIT2;
                    valueFIT2.append((char)0x0A); // min resistance value
                    valueFIT2.append((char)0x00); // min resistance value
                    valueFIT2.append((char)0x96); // max resistance value
                    valueFIT2.append((char)0x00); // max resistance value
                    valueFIT2.append((char)0x0A); // step resistance
                    valueFIT2.append((char)0x00); // step resistance
                    charDataFIT2.setValue(valueFIT2);

                    QLowEnergyCharacteristicData charDataFIT3;
                    charDataFIT3.setUuid((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
                    charDataFIT3.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate |
                                               QLowEnergyCharacteristic::Notify);
                    QByteArray descriptor9;
                    descriptor9.append((char)0x03);
                    descriptor9.append((char)0x00);
                    const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                                  descriptor9);
                    charDataFIT3.addDescriptor(cpClientConfig);

                    QLowEnergyCharacteristicData charDataFIT4;
                    charDataFIT4.setUuid((QBluetoothUuid::CharacteristicType)0x2AD2); // indoor bike
                    charDataFIT4.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                                 descriptor);
                    charDataFIT4.addDescriptor(clientConfig4);

                    QLowEnergyCharacteristicData charDataFIT5;
                    charDataFIT5.setUuid((QBluetoothUuid::CharacteristicType)0x2ADA); // Fitness Machine status
                    charDataFIT5.setProperties(QLowEnergyCharacteristic::Notify);
                    QByteArray descriptor5;
                    descriptor5.append((char)0x01);
                    descriptor5.append((char)0x00);
                    const QLowEnergyDescriptorData clientConfig5(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                                 descriptor5);
                    charDataFIT5.addDescriptor(clientConfig5);

                    QLowEnergyCharacteristicData charDataFIT6;
                    charDataFIT6.setUuid((QBluetoothUuid::CharacteristicType)0x2AD3);
                    charDataFIT6.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
                    QByteArray valueFIT6;
                    valueFIT6.append((char)0x00);
                    valueFIT6.append((char)0x01);
                    charDataFIT6.setValue(valueFIT6);
                    QByteArray descriptor6;
                    descriptor6.append((char)0x01);
                    descriptor6.append((char)0x00);
                    const QLowEnergyDescriptorData clientConfig6(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                                 descriptor6);
                    charDataFIT6.addDescriptor(clientConfig6);
                    charDataFIT6.setProperties(QLowEnergyCharacteristic::Read);

                    serviceDataFIT.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
                    serviceDataFIT.addCharacteristic(charDataFIT);
                    serviceDataFIT.addCharacteristic(charDataFIT2);
                    serviceDataFIT.addCharacteristic(charDataFIT3);
                    serviceDataFIT.addCharacteristic(charDataFIT4);
                    serviceDataFIT.addCharacteristic(charDataFIT5);
                    serviceDataFIT.addCharacteristic(charDataFIT6);

                    if(zwift_play_emulator) {
                        QLowEnergyCharacteristicData charData;
                        charData.setUuid(QBluetoothUuid(QStringLiteral("00000003-19ca-4651-86e5-fa29dcdd09d1")));
                        charData.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::WriteNoResponse);

                        QLowEnergyCharacteristicData charData2;
                        charData2.setUuid(QBluetoothUuid(QStringLiteral("00000002-19ca-4651-86e5-fa29dcdd09d1")));
                        charData2.setProperties(QLowEnergyCharacteristic::Notify);
                        const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, descriptor);
                        charData2.addDescriptor(clientConfig2);

                        QLowEnergyCharacteristicData charData3;
                        charData3.setUuid(QBluetoothUuid(QStringLiteral("00000004-19ca-4651-86e5-fa29dcdd09d1")));
                        charData3.setProperties(QLowEnergyCharacteristic::Indicate);
                        QByteArray descriptorIndicate;
                        descriptorIndicate.append((char)0x02);
                        descriptorIndicate.append((char)0x00);
                        const QLowEnergyDescriptorData clientConfig3(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, descriptorIndicate);
                        charData3.addDescriptor(clientConfig3);

                        serviceDataZwiftPlayBike.setType(QLowEnergyServiceData::ServiceTypePrimary);
                        serviceDataZwiftPlayBike.setUuid(QBluetoothUuid(QStringLiteral("00000001-19ca-4651-86e5-fa29dcdd09d1")));
                        serviceDataZwiftPlayBike.addCharacteristic(charData);
                        serviceDataZwiftPlayBike.addCharacteristic(charData2);
                        serviceDataZwiftPlayBike.addCharacteristic(charData3);
                    } else if(watt_bike_emulator) {
                        QLowEnergyCharacteristicData charData;
                        charData.setUuid(QBluetoothUuid(QStringLiteral("b4cc1225-bc02-4cae-adb9-1217ad2860d1")));
                        charData.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::WriteNoResponse);

                        QLowEnergyCharacteristicData charData2;
                        charData2.setUuid(QBluetoothUuid(QStringLiteral("b4cc1224-bc02-4cae-adb9-1217ad2860d1")));
                        charData2.setProperties(QLowEnergyCharacteristic::Notify);
                        const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, descriptor);

                        charData2.addDescriptor(clientConfig2);

                        serviceDataWattAtomBike.setType(QLowEnergyServiceData::ServiceTypePrimary);
                        serviceDataWattAtomBike.setUuid(QBluetoothUuid(QStringLiteral("b4cc1223-bc02-4cae-adb9-1217ad2860d1")));
                        serviceDataWattAtomBike.addCharacteristic(charData);
                        serviceDataWattAtomBike.addCharacteristic(charData2);                                           
                    }
                } else if (power) {

                    QLowEnergyCharacteristicData charData;
                    charData.setUuid(QBluetoothUuid::CharacteristicType::CyclingPowerFeature);
                    charData.setProperties(QLowEnergyCharacteristic::Read);
                    QByteArray value;
                    value.append((char)0x08); // crank supported
                    value.append((char)0x00);
                    value.append((char)0x00);
                    value.append((char)0x00);
                    charData.setValue(value);

                    QLowEnergyCharacteristicData charData2;
                    charData2.setUuid(QBluetoothUuid::CharacteristicType::SensorLocation);
                    charData2.setProperties(QLowEnergyCharacteristic::Read);
                    QByteArray valueLocaltion;
                    valueLocaltion.append((char)13); // rear hub
                    charData2.setValue(valueLocaltion);
                    /*const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                        QByteArray(2, 0));
            charData2.addDescriptor(clientConfig2);*/

                    QLowEnergyCharacteristicData charData3;
                    charData3.setUuid(QBluetoothUuid::CharacteristicType::CyclingPowerMeasurement);
                    charData3.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                                 descriptor);
                    charData3.addDescriptor(clientConfig4);

                    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
                    serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::CyclingPower);
                    serviceData.addCharacteristic(charData);
                    serviceData.addCharacteristic(charData3);
                    serviceData.addCharacteristic(charData2);
                } else {

                    QLowEnergyCharacteristicData charData;
                    charData.setUuid(QBluetoothUuid::CharacteristicType::CSCFeature);
                    charData.setProperties(QLowEnergyCharacteristic::Read);
                    QByteArray value;
                    if (!bike_wheel_revs) {
                        value.append((char)0x02); // crank supported
                    } else {
                        value.append((char)0x03); // crank and wheel supported
                    }
                    value.append((char)0x00);
                    charData.setValue(value);

                    QLowEnergyCharacteristicData charData2;
                    charData2.setUuid(QBluetoothUuid::CharacteristicType::SensorLocation);
                    charData2.setProperties(QLowEnergyCharacteristic::Read);
                    QByteArray valueLocaltion;
                    valueLocaltion.append((char)13); // rear hub
                    charData2.setValue(valueLocaltion);
                    /*const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                        QByteArray(2, 0));
            charData2.addDescriptor(clientConfig2);*/

                    QLowEnergyCharacteristicData charData3;
                    charData3.setUuid(QBluetoothUuid::CharacteristicType::CSCMeasurement);
                    charData3.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify);
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                                 descriptor);
                    charData3.addDescriptor(clientConfig4);

                    QLowEnergyCharacteristicData charData4;
                    charData4.setUuid(QBluetoothUuid::CharacteristicType::SCControlPoint);
                    charData4.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
                    const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                                  QByteArray(2, 0));
                    charData4.addDescriptor(cpClientConfig);

                    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
                    serviceData.setUuid(QBluetoothUuid::ServiceClassUuid::CyclingSpeedAndCadence);
                    serviceData.addCharacteristic(charData);
                    serviceData.addCharacteristic(charData3);
                    serviceData.addCharacteristic(charData2);
                    serviceData.addCharacteristic(charData4);
                }
            }
        } else if (ifit) {
            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(QStringLiteral("00001534-1412-efde-1523-785feabcd123")));
            charData.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::WriteNoResponse);

            QLowEnergyCharacteristicData charData2;
            charData2.setUuid(QBluetoothUuid(QStringLiteral("00001535-1412-efde-1523-785feabcd123")));
            charData2.setProperties(QLowEnergyCharacteristic::Notify);
            QByteArray descriptor;
            descriptor.append((char)0x01);
            descriptor.append((char)0x00);
            const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, descriptor);

            charData2.addDescriptor(clientConfig2);

            serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceData.setUuid(QBluetoothUuid(QStringLiteral("00001533-1412-efde-1523-785feabcd123")));
            serviceData.addCharacteristic(charData);
            serviceData.addCharacteristic(charData2);
        } else {

            serviceEchelon.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceEchelon.setUuid((QBluetoothUuid(QStringLiteral("0bf669f0-45f2-11e7-9598-0800200c9a66"))));

            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid(QStringLiteral("0bf669f2-45f2-11e7-9598-0800200c9a66")));
            charData.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::WriteNoResponse);

            QLowEnergyCharacteristicData charData2;
            charData2.setUuid(QBluetoothUuid(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66")));
            charData2.setProperties(QLowEnergyCharacteristic::Notify);
            QByteArray descriptor;
            descriptor.append((char)0x01);
            descriptor.append((char)0x00);
            const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration, descriptor);

            charData2.addDescriptor(clientConfig2);

            QLowEnergyCharacteristicData charData3;
            charData3.setUuid(QBluetoothUuid(QStringLiteral("0bf669f3-45f2-11e7-9598-0800200c9a66")));
            charData3.setProperties(QLowEnergyCharacteristic::Notify);
            QByteArray descriptor3;
            descriptor3.append((char)0x01);
            descriptor3.append((char)0x00);
            const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                         descriptor3);
            charData3.addDescriptor(clientConfig4);

            serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceData.setUuid(QBluetoothUuid(QStringLiteral("0bf669f1-45f2-11e7-9598-0800200c9a66")));
            serviceData.addCharacteristic(charData);
            serviceData.addCharacteristic(charData3);
            serviceData.addCharacteristic(charData2);
        }

        if (battery) {

            QLowEnergyCharacteristicData charDataBattery;
            charDataBattery.setUuid(QBluetoothUuid::CharacteristicType::BatteryLevel);
            charDataBattery.setValue(QByteArray(2, 0));
            charDataBattery.setProperties(QLowEnergyCharacteristic::Notify);
            const QLowEnergyDescriptorData clientConfigBattery(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                               QByteArray(2, 0));
            charDataBattery.addDescriptor(clientConfigBattery);

            serviceDataBattery.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceDataBattery.setUuid(QBluetoothUuid::ServiceClassUuid::BatteryService);
            serviceDataBattery.addCharacteristic(charDataBattery);
        }

        if (!this->noHeartService || heart_only) {

            QLowEnergyCharacteristicData charDataHR;
            charDataHR.setUuid(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
            charDataHR.setValue(QByteArray(2, 0));
            charDataHR.setProperties(QLowEnergyCharacteristic::Notify);
            const QLowEnergyDescriptorData clientConfigHR(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                          QByteArray(2, 0));
            charDataHR.addDescriptor(clientConfigHR);

            serviceDataHR.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceDataHR.setUuid(QBluetoothUuid::ServiceClassUuid::HeartRate);
            serviceDataHR.addCharacteristic(charDataHR);
        }

        if (service_changed) {

            QLowEnergyCharacteristicData charData;
            charData.setUuid(QBluetoothUuid::CharacteristicType::ServiceChanged);
            charData.setProperties(QLowEnergyCharacteristic::Indicate);
            const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration,
                                                          QByteArray(2, 0));
            charData.addDescriptor(cpClientConfig);

            serviceDataChanged.setType(QLowEnergyServiceData::ServiceTypePrimary);
            serviceDataChanged.setUuid(QBluetoothUuid::ServiceClassUuid::GenericAttribute);
            serviceDataChanged.addCharacteristic(charData);
        }

        //! [Start Advertising]
        leController = QLowEnergyController::createPeripheral();
        Q_ASSERT(leController);

        if (service_changed)
            serviceChanged = leController->addService(serviceDataChanged);

        if (!echelon && !ifit) {
            if (!heart_only) {
                if (!cadence && !power) {

                    serviceFIT = leController->addService(serviceDataFIT);

                    if(zwift_play_emulator) {
                        QThread::msleep(100); // give time to Android to add the service async.ly
                        serviceZwiftPlayBike = leController->addService(serviceDataZwiftPlayBike);
                    } else if(watt_bike_emulator) {
                        QThread::msleep(100); // give time to Android to add the service async.ly
                        serviceWattAtomBike = leController->addService(serviceDataWattAtomBike);
                    }
                } else {
                    service = leController->addService(serviceData);
                }
            }
        } else if (ifit) {
            service = leController->addService(serviceData);
        } else {

            service = leController->addService(serviceEchelon);
            QThread::msleep(100); // give time to Android to add the service async.ly
            service = leController->addService(serviceData);
        }
        QThread::msleep(100); // give time to Android to add the service async.ly

        if (battery) {
            serviceBattery = leController->addService(serviceDataBattery);
        }

        if (!this->noHeartService || heart_only) {
            serviceHR = leController->addService(serviceDataHR);
        }

        if (!echelon && !ifit) {
            if (!heart_only) {
                if (!cadence && !power) {
                    QObject::connect(serviceFIT, &QLowEnergyService::characteristicChanged, this,
                                     &virtualbike::characteristicChanged);
                    QObject::connect(serviceWattAtomBike, &QLowEnergyService::characteristicChanged, this,
                                     &virtualbike::characteristicChanged);
                    QObject::connect(serviceZwiftPlayBike, &QLowEnergyService::characteristicChanged, this,
                                     &virtualbike::characteristicChanged);
                } else {
                    QObject::connect(service, &QLowEnergyService::characteristicChanged, this,
                                     &virtualbike::characteristicChanged);
                }
            }
        } else if (ifit) {
            QObject::connect(service, &QLowEnergyService::characteristicChanged, this,
                             &virtualbike::characteristicChanged);
        } else {
            QObject::connect(service, &QLowEnergyService::characteristicChanged, this,
                             &virtualbike::characteristicChanged);
        }

        bool bluetooth_relaxed =
            settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
        QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
        if (!bluetooth_relaxed) {
            pars.setInterval(100, 100);
        }

        leController->startAdvertising(pars, advertisingData, advertisingData);

        //! [Start Advertising]
    }

    //! [Provide Heartbeat]
    QObject::connect(&bikeTimer, &QTimer::timeout, this, &virtualbike::bikeProvider);
    if (settings.value(QZSettings::race_mode, QZSettings::default_race_mode).toBool() || zwift_play_emulator)
        bikeTimer.start(50ms);
    else
        bikeTimer.start(1s);

    //! [Provide Heartbeat]
    QObject::connect(leController, &QLowEnergyController::disconnected, this, &virtualbike::reconnect);
    QObject::connect(
        leController,
        &QLowEnergyController::errorOccurred, this,
        &virtualbike::error);
}

// zwift play emulator protobuf

// Decode a protobuf varint starting from startIndex
// Returns the decoded value and number of bytes read
virtualbike::VarintResult virtualbike::decodeVarint(const QByteArray& bytes, int startIndex) {
   qint64 result = 0;
   int shift = 0;
   int bytesRead = 0;
   
   for (int i = startIndex; i < bytes.size(); i++) {
       quint8 byte = static_cast<quint8>(bytes.at(i));
       result |= static_cast<qint64>(byte & 0x7F) << shift;
       bytesRead++;
       
       // If the most significant bit is 0, we're done
       if ((byte & 0x80) == 0) {
           break;
       }
       shift += 7;
   }
   
   return {result, bytesRead};
}

// Decode a protobuf sint value from a byte array
// Expects a field header (0x22), length byte, inner header (0x10) and varint value
qint32 virtualbike::decodeSInt(const QByteArray& bytes) {
   // Check field header (0x22 = field number 4, wire type 2)
   if (static_cast<quint8>(bytes.at(0)) != 0x22) {
       qFatal("Invalid field header");
   }
   
   // Get content length
   int length = static_cast<quint8>(bytes.at(1));
   
   // Check inner header (0x10 = field number 2, wire type 0)
   if (static_cast<quint8>(bytes.at(2)) != 0x10) {
       qFatal("Invalid inner header");
   }
   
   // Decode the varint value
   VarintResult varint = decodeVarint(bytes, 3);
   
   // Apply ZigZag decoding to get the original signed value
   qint32 decoded = (varint.value >> 1) ^ -(varint.value & 1);
   
   return decoded;
}

void virtualbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QByteArray reply;
    QSettings settings;
    bool zwift_play_emulator = settings.value(QZSettings::zwift_play_emulator, QZSettings::default_zwift_play_emulator).toBool();
    bool echelon =
        settings.value(QZSettings::virtual_device_echelon, QZSettings::default_virtual_device_echelon).toBool();
    bool ifit = settings.value(QZSettings::virtual_device_ifit, QZSettings::default_virtual_device_ifit).toBool();

    double normalizeWattage = Bike->wattsMetricforUI();
    if (normalizeWattage < 0)
        normalizeWattage = 0;

    qDebug() << QStringLiteral("characteristicChanged ") + QString::number(characteristic.uuid().toUInt16()) +
                    QStringLiteral(" ") + newValue.toHex(' ');

    switch (characteristic.uuid().toUInt16()) {

    case 0x2AD9: // Fitness Machine Control Point
        if (!echelon && !ifit) {
            lastFTMSFrameReceived = QDateTime::currentMSecsSinceEpoch();
            emit ftmsCharacteristicChanged(characteristic, newValue);
        }
        if (writeP2AD9->writeProcess(0x2AD9, newValue, reply) == CP_OK) {

            QLowEnergyCharacteristic characteristic =
                serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual bike not connected");

                return;
            }
            writeCharacteristic(serviceFIT, characteristic, reply);
        }
        break;
    }

    //******************** IFIT ******************
    if (characteristic.uuid().toString().contains(QStringLiteral("00001534-1412-efde-1523-785feabcd123"))) {
        QLowEnergyCharacteristic characteristic =
            service->characteristic(QBluetoothUuid(QStringLiteral("00001535-1412-efde-1523-785feabcd123")));

        Q_ASSERT(characteristic.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }

        QByteArray reply1;
        QByteArray reply2;
        QByteArray reply3;
        QByteArray reply4;

        static bool answer_13 = false;

        iFit_TSLastFrame = QDateTime::currentMSecsSinceEpoch();

        iFit_LastFrameReceived.clear();
        foreach (uint8_t a, newValue)
            iFit_LastFrameReceived.append(a);

        if (answer_13) {
            answer_13 = false;

            qDebug() << "ifit ans 13";
            reply1 = QByteArray::fromHex("fe020a02000000000000001bffffffffffffffff");
            reply2 = QByteArray::fromHex("ff0a010402060706900208a7ffffffffffffffff");

            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x81) {
            // equipment information
            qDebug() << "ifit ans 1";
            reply1 = QByteArray::fromHex("fe02210340ff7b81600080dfbf1404fffb4808b7");
            reply2 = QByteArray::fromHex("00120104021d071d810253010300000000000fbc");
            reply3 = QByteArray::fromHex("ff0fbcfdc3fcffca94e707c0c0d118180d000fbc");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x80) {
            qDebug() << "ifit ans 2";
            reply1 = QByteArray::fromHex("fe021303c3fcffca94e707c0c0d118180d000fbc");
            reply2 = QByteArray::fromHex("00120104020f070f8002094c4745434d4e464f41");
            reply3 = QByteArray::fromHex("ff012d04020f070f8002094c4745434d4e464f41");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x88) {
            qDebug() << "ifit ans 3";
            reply1 = QByteArray::fromHex("fe021102020f070f8002094c4745434d4e464f41");
            reply2 = QByteArray::fromHex("ff110104020d070d880209829083718984954f41");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x82) {
            qDebug() << "ifit ans 4";
            reply1 = QByteArray::fromHex("fe022504020d070d880209829083718984954f41");
            reply2 = QByteArray::fromHex("00120104022107218202640001aff900002b5706");
            reply3 = QByteArray::fromHex("01120056002ae8030024f400f401000001020000");
            reply4 = QByteArray::fromHex("ff01bc56002ae8030024f400f401000001020000");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
            writeCharacteristic(service, characteristic, reply4);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x84) {
            qDebug() << "ifit ans 5";
            reply1 = QByteArray::fromHex("fe022003002ae8030024f400f401000001020000");
            reply2 = QByteArray::fromHex("00120104021c071c8402539600302e312e303631");
            reply3 = QByteArray::fromHex("ff0e32323031372e30393038012a030f2e303631");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x95) {
            qDebug() << "ifit ans 6";
            reply1 = QByteArray::fromHex("fe021c033031372e30393038012a030f2e303631");
            reply2 = QByteArray::fromHex("00120104021807189502123431373131302d4e4e");
            reply3 = QByteArray::fromHex("ff0a32335a313130313737af31373131302d4e4e");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x5d) {
            qDebug() << "ifit ans 7";
            reply1 = QByteArray::fromHex("fe02240302060706900208a731373131302d4e4e");
            reply2 = QByteArray::fromHex("0012010402200720020202701750001e00780000");
            reply3 = QByteArray::fromHex("ff12104c2c2c010000000000000000b400000003");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
        } else if (newValue.length() > 9 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x00 &&
                   ((uint8_t)newValue.at(9)) == 0xd1) {
            qDebug() << "ifit ans 8";
            reply1 = QByteArray::fromHex("fe020a025a313130313737af31373131302d4e4e");
            reply2 = QByteArray::fromHex("ff0a010402060706900208a731373131302d4e4e");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 9 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x00 &&
                   ((uint8_t)newValue.at(3)) == 0x80) {
            qDebug() << "ifit ans 9";
            reply1 = QByteArray::fromHex("fe0209020205070502021000000000b400000003");
            reply2 = QByteArray::fromHex("ff0901040205070502021000000000b400000003");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x3d) {
            qDebug() << "ifit ans 10";
            reply1 = QByteArray::fromHex("fe0209020205070502021000000000b400000003");
            reply2 = QByteArray::fromHex("ff0901040205070502021000000000b400000003");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(8)) == 0x00) {
            qDebug() << "ifit ans 11";
            if (iFit_timer == 0)
                iFit_timer = QDateTime::currentSecsSinceEpoch();
            reply1 = QByteArray::fromHex("fe0233040000302a00000075ffffffffffffffff");
            reply2 = QByteArray::fromHex("00120104022f072f020232021f00530000002100");
            reply3 = QByteArray::fromHex("01120000000017000000021700a4031700000069");
            reply4 = QByteArray::fromHex("ff0f9c0200b4002a00580000000000002e000069");

            // with mod bytes
            /*
             if(counter == 0) {
                 reply1 = QByteArray::fromHex("fe0233040000302a00000075ffffffffffffffff");
                 reply2 = QByteArray::fromHex("00120104022f072f020232021f00530000002100");
                 reply3 = QByteArray::fromHex("01120000000017000000021700a4031700000069");
                 reply4 = QByteArray::fromHex("ff0f9c0200b4002a00580200000000002e000069");
             } else if(counter == 1) {
                 reply1 = QByteArray::fromHex("fe0233040000302a00000075ffffffffffffffff");
                 reply2 = QByteArray::fromHex("00120104022f072f020232021f00530000002100");
                 reply3 = QByteArray::fromHex("01120000000018000000021800a4031800000066");
                 reply4 = QByteArray::fromHex("ff0f9c0200b4002a00580200000000002e000066");
             }

             */
            // original
            /*
             fe0233040000302a00000075ffffffffffffffff
             00120104022f072f020232021f00530000002100
             01120000000017000000021700a4031700000069
             ff0f9c0200b4002a00580200000000002e000069

             fe0233040000302a00000075ffffffffffffffff
             00120104022f072f020232021f00530000002100
             01120000000018000000021800a403180000007d
             ff0fb50200b4002a00580200000000005e00007d
             */

            int resistance = (iFit_LastResistanceRequested == 0
                                  ? iFit_pelotonToBikeResistance((uint8_t)((bike *)Bike)->pelotonResistance().value())
                                  : iFit_LastResistanceRequested);

            qDebug() << QStringLiteral("current resistance converted from the bike")
                     << iFit_pelotonToBikeResistance((uint8_t)((bike *)Bike)->pelotonResistance().value())
                     << QStringLiteral("last requested resistance") << iFit_LastResistanceRequested
                     << QStringLiteral("resistance sent to ifit") << resistance;

            double odometer = Bike->odometer() * 1000;
            // ifit applies a constant multiplier to the kcal sent from bluetooth
            double calories = Bike->calories().value() * 1.488;
            if (resistance > 0x26)
                resistance = 0x26;
            qint64 t = (QDateTime::currentSecsSinceEpoch() - iFit_timer);
            reply2[11] = resistance;                                       // resistance (limit to 0x26)
            reply2[12] = ((uint16_t)normalizeWattage) & 0xff;              // watt (l)
            reply2[13] = ((uint16_t)normalizeWattage) >> 8;                // watt (h)
            reply2[14] = ((uint32_t)odometer) & 0xFF;                      // distance (l)
            reply2[15] = ((uint32_t)odometer) >> 8;                        // distance (ll)
            reply2[16] = ((uint32_t)odometer) >> 16;                       // distance (h)
            reply2[18] = ((uint8_t)Bike->currentCadence().value()) & 0xff; // cadence
            reply3[6] = t & 0xff;
            reply3[7] = t >> 8;
            reply3[11] = t & 0xff;
            reply3[12] = t >> 8;
            double speed = Bike->currentSpeed().value();
            reply3[13] = ((uint16_t)(speed * 100.0)) & 0xff; // speed (l)
            reply3[14] = ((uint16_t)(speed * 100.0)) >> 8;   // speed (h)
            reply3[15] = t & 0xff;
            reply3[16] = t >> 8;
            reply4[3] = ((uint16_t)calories);         // KCal
            reply4[4] = (((uint16_t)calories) >> 8);  // KCal
            reply4[10] = ((uint16_t)calories);        // KCal estimated
            reply4[11] = (((uint16_t)calories) >> 8); // KCal
            reply3[19] = 0xEE - (reply3[15] * 3) - (reply4[10] * 2) - (reply4[4] * 2) - (reply2[18]) - (reply2[11]) -
                         (reply2[12]) - (reply2[13]) - (reply3[13]) - (reply3[14]) - (reply2[14]) - (reply3[7]) -
                         (reply3[12]) - (reply3[16]) - (reply2[15]) - (reply2[16]);
            reply4[19] = reply3[19];

            /*static uint64_t time = 0;
            if(time == 0) time = QDateTime::currentMSecsSinceEpoch();

            reply3[13] = ((QDateTime::currentMSecsSinceEpoch() - time) / 10) & 0xff;
            reply3[14] = ((QDateTime::currentMSecsSinceEpoch() - time) / 10) >> 8;*/

            /*
            reply1 = QByteArray::fromHex("fe02330400caaf020000000000330000df130013");
            reply2 = QByteArray::fromHex("00120104022f072f020200003d00650000003700");
            reply3 = QByteArray::fromHex("01120000000114000000021400f5061400000067");
            reply4 = QByteArray::fromHex("ff0fd702002c013300b4000000000000a1000067");

            reply2[11] = Bike->currentResistance().value();
            reply2[12] = ((uint16_t)normalizeWattage) & 0xFF;
            reply2[13] = (((uint16_t)normalizeWattage) >> 8) & 0xFF;
            reply2[18] = Bike->currentCadence().value();*/

            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
            writeCharacteristic(service, characteristic, reply4);
        } else if (newValue.length() > 8 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(1)) == 0x07 &&
                   ((uint8_t)newValue.at(7)) == 0x10) {
            qDebug() << "ifit ans 12";
            if(iFit_Stop == false) {
                reply1 = QByteArray::fromHex("fe021c0300b4002200580200000000007e0000b4");
            } else {
                qDebug() << "ifit ans 12 - with stop request";
                reply1 = QByteArray::fromHex("fe021c0302050705020210000000000036000000");
                iFit_Stop = false;
            }
            reply2 = QByteArray::fromHex("001201040218071802020000ffffffffffffffff");
            reply3 = QByteArray::fromHex("ff0a00000000302a00000075ffffffffffffffff");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);

            /*
            reply1 = QByteArray::fromHex("fe023304002c012700b400000000000005000085");
            reply2 = QByteArray::fromHex("00120104022f072f020200003900450000003500");
            reply3 = QByteArray::fromHex("01120000ffffffffffffffff00000000020d000d");
            reply4 = QByteArray::fromHex("ff0f000000bac00100000000002e0000aa0d000d");

            reply2[11] = Bike->currentResistance().value();
            reply2[12] = ((uint16_t)normalizeWattage) & 0xFF;
            reply2[13] = (((uint16_t)normalizeWattage) >> 8) & 0xFF;
            reply2[18] = Bike->currentCadence().value();

            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
            writeCharacteristic(service, characteristic, reply3);
            writeCharacteristic(service, characteristic, reply4);*/
        } else if (newValue.length() > 8 &&
                   !newValue.compare(QByteArray::fromHex("0112b472461cf0be92403ceacea488764a2804e2"))) {
            answer_13 = true;
        } else if (newValue.length() > 12 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(1)) == 0x0D &&
                   ((uint8_t)newValue.at(2)) == 0x02) {
            qDebug() << "ifit ans 14";
            // force resistance
            reply1 = QByteArray::fromHex("fe0209020205070502021075ffffffffffffffff");
            reply2 = QByteArray::fromHex("ff0901040205070502021075ffffffffffffffff");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);

            QSettings settings;
            bool force_resistance =
                settings.value(QZSettings::virtualbike_forceresistance, QZSettings::default_virtualbike_forceresistance)
                    .toBool();

            iFit_LastResistanceRequested = newValue.at(12);
            qDebug() << QStringLiteral("requested iFit resistance ") + QString::number(iFit_LastResistanceRequested);

            if(((bike*)Bike)->ifitCompatible()) {
                if (force_resistance)
                    Bike->changeResistance(iFit_resistanceToIfit(iFit_LastResistanceRequested));
            } else if(((bike*)Bike)->inclinationAvailableByHardware()) {
                Bike->changeInclination((iFit_LastResistanceRequested * bikeResistanceGain) + bikeResistanceOffset, (iFit_LastResistanceRequested * bikeResistanceGain) + bikeResistanceOffset);
            } else {
                for (int i = 0; i < 100; i++) {
                    if (iFit_pelotonToBikeResistance(i) == iFit_LastResistanceRequested) {
                        if (force_resistance) {                        
                            // same on the training program
                            Bike->changeResistance(
                                (resistance_t)(round((((bike *)Bike)->pelotonToBikeResistance(i)) * bikeResistanceGain)) +
                                bikeResistanceOffset); // resistance start from 1
                        }
                        break;
                    }

                }
            }
        } else if (newValue.length() > 12 && ((uint8_t)newValue.at(0)) == 0xFF && ((uint8_t)newValue.at(1)) == 0x0F &&
                   ((uint8_t)newValue.at(2)) == 0x02 && ((uint8_t)newValue.at(3)) == 0x04 &&
                   ((uint8_t)newValue.at(4)) == 0x02 && ((uint8_t)newValue.at(5)) == 0x0b &&
                   ((uint8_t)newValue.at(6)) == 0x07 && ((uint8_t)newValue.at(7)) == 0x0b &&
                   ((uint8_t)newValue.at(8)) == 0x02) { // ff0f0204020b070b0202041032020a0068000000
            qDebug() << "ifit ans 15 stop request";
            iFit_timer = 0;
            iFit_Stop = true;
            reply1 = QByteArray::fromHex("fe02090200b40000005802000000000038000000");
            reply2 = QByteArray::fromHex("ff09010402050705020210000000000038000000");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);
        } else if (newValue.length() > 12 && ((uint8_t)newValue.at(0)) == 0xFF &&
                   ((uint8_t)newValue.at(1)) == 0x10) { // Value: ff100204020c040c02020004b600000400d20000
            qDebug() << "ifit ans 16";
            reply1 = QByteArray::fromHex("fe0209025802341c0500341c050000a56700b400");
            reply2 = QByteArray::fromHex("ff0901040205040502020d1c050000a56700b400");
            writeCharacteristic(service, characteristic, reply1);
            writeCharacteristic(service, characteristic, reply2);

        } else if (newValue.length() > 8 && (uint8_t)newValue.at(0) == 0xFF) {
            qDebug() << "ifit not managed";
        }
    }

    //********************ZWIFT PLAY**************

    if(characteristic.uuid().toString().contains(QStringLiteral("00000003-19ca-4651-86e5-fa29dcdd09d1")) && zwift_play_emulator) {
        static const QByteArray expectedHexArray = QByteArray::fromHex("52696465 4F6E02");
        static const QByteArray expectedHexArray2 = QByteArray::fromHex("410805");
        static const QByteArray expectedHexArray3 = QByteArray::fromHex("00088804");
        static const QByteArray expectedHexArray4 = QByteArray::fromHex("042A0A10 C0BB0120");
        static const QByteArray expectedHexArray5 = QByteArray::fromHex("0422");
        static const QByteArray expectedHexArray6 = QByteArray::fromHex("042A0410");
        static const QByteArray expectedHexArray7 = QByteArray::fromHex("042A0310");
        static const QByteArray expectedHexArray8 = QByteArray::fromHex("0418");

        QByteArray receivedData = newValue;

        QLowEnergyCharacteristic zwiftPlayRead =
            serviceZwiftPlayBike->characteristic(QBluetoothUuid(QStringLiteral("00000002-19ca-4651-86e5-fa29dcdd09d1")));

        QLowEnergyCharacteristic zwiftPlayIndicate =
            serviceZwiftPlayBike->characteristic(QBluetoothUuid(QStringLiteral("00000004-19ca-4651-86e5-fa29dcdd09d1")));

        if (receivedData.startsWith(expectedHexArray)) {
            qDebug() << "Zwift Play Ask 1";

            QByteArray response = QByteArray::fromHex("2a08031211220f4154582030342c2053545820303400");
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayRead, response);

            response = QByteArray::fromHex("2a0803120d220b524944455f4f4e28322900");
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayRead, response);

            response = QByteArray::fromHex("526964654f6e0200");
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayIndicate, response);
        }
        else if (receivedData.startsWith(expectedHexArray2)) {
            qDebug() << "Zwift Play Ask 2";

            QByteArray response = QByteArray::fromHex("3c080012320a3008800412040500050"
                                                      "11a0b4b49434b5220434f524500320f"
                                                      "3430323431383030393834000000003a01314204080110140");
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayIndicate, response);
        }
        else if (receivedData.startsWith(expectedHexArray3)) {
            qDebug() << "Zwift Play Ask 3";

            QByteArray response = QByteArray::fromHex("3c0888041206 0a0440c0bb01");
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayIndicate, response);
        }
        else if (receivedData.startsWith(expectedHexArray4)) {
            qDebug() << "Zwift Play Ask 4";

            QByteArray response = QByteArray::fromHex("0308001000185920002800309bed01");
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayRead, response);

            response = QByteArray::fromHex("2a08031227222567"
                                           "61705f706172616d735f6368616e6765"
                                           "2832293a2037322c2037322c20302c20"
                                           "36303000");
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayRead, response);
        }
        else if (receivedData.startsWith(expectedHexArray5)) {
            qDebug() << "Zwift Play Ask 5";

            double slopefloat = decodeSInt(receivedData.mid(1));
            QByteArray slope(2, 0);
            slope[0] = quint8(qint16(slopefloat) & 0xFF);
            slope[1] = quint8((qint16(slopefloat) >> 8) & 0x00FF);

            QBluetoothUuid targetUuid = QBluetoothUuid(quint16(0x2ad9));
            QLowEnergyCharacteristic targetCharacteristic;

            for (const QLowEnergyCharacteristic &characteristic : serviceFIT->characteristics()) {
                if (characteristic.uuid() == targetUuid) {
                    targetCharacteristic = characteristic;
                    break; 
                }
            }

            if (targetCharacteristic.isValid()) {
                characteristicChanged(targetCharacteristic, QByteArray::fromHex("116901") + slope + QByteArray::fromHex("3228"));

                QByteArray response = CharacteristicWriteProcessor0003::encodeHubRidingData(Bike->wattsMetricforUI(), Bike->currentCadence().value(), 0,
                                                                      Bike->wattsMetricforUI(),
                                                                      CharacteristicWriteProcessor0003::calculateUnknown1(Bike->wattsMetricforUI()),
                                                                      0);
                writeCharacteristic(serviceZwiftPlayBike, zwiftPlayIndicate, response);
            } else {
                qDebug() << "ERROR! Zwift Play Ask 5 without answer!";
            }
        }
        else if (receivedData.startsWith(expectedHexArray6)) {
            qDebug() << "Zwift Play Ask 6";

            QByteArray response = QByteArray::fromHex("3c0888041206 0a0440c0bb01");
            response[9] = receivedData[4];
            response[10] = receivedData[5];
            response[11] = receivedData[6];
            writeP0003->handleZwiftGear(receivedData.mid(4));
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayIndicate, response);

            response = QByteArray::fromHex("03080010001827e7 20002896143093ed01");
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayRead, response);
        }
        else if (receivedData.startsWith(expectedHexArray7)) {
            qDebug() << "Zwift Play Ask 7";

            QByteArray response = QByteArray::fromHex("03080010001827e7 2000 28 00 3093ed01");
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayRead, response);

            response = QByteArray::fromHex("3c088804120503408c60");
            response[9] = receivedData[4];
            response[10] = receivedData[5];
            writeP0003->handleZwiftGear(receivedData.mid(4));
            writeCharacteristic(serviceZwiftPlayBike, zwiftPlayIndicate, response);
        }
        else if (receivedData.startsWith(expectedHexArray8)) {
            qDebug() << "Zwift Play Ask 8";

            QByteArray power(2, 0);
            VarintResult Power = decodeVarint(receivedData, 2);
            power[0] = quint8(qint16(Power.value) & 0xFF);
            power[1] = quint8((qint16(Power.value) >> 8) & 0x00FF);

            QBluetoothUuid targetUuid = QBluetoothUuid(quint16(0x2ad9));
            QLowEnergyCharacteristic targetCharacteristic;

            for (const QLowEnergyCharacteristic &characteristic : serviceFIT->characteristics()) {
                if (characteristic.uuid() == targetUuid) {
                    targetCharacteristic = characteristic;
                    break; 
                }
            }

            if (targetCharacteristic.isValid()) {
                characteristicChanged(targetCharacteristic, QByteArray::fromHex("05") + power);

                QByteArray response = CharacteristicWriteProcessor0003::encodeHubRidingData(Bike->wattsMetricforUI(), Bike->currentCadence().value(), 0,
                                                                      Bike->wattsMetricforUI(),
                                                                      CharacteristicWriteProcessor0003::calculateUnknown1(Bike->wattsMetricforUI()),
                                                                      0);
                writeCharacteristic(serviceZwiftPlayBike, zwiftPlayIndicate, response);
            } else {
                qDebug() << "ERROR! Zwift Play Ask 8 without answer!";
            }
        }
    }

    //******************** ECHELON ***************
    if (characteristic.uuid().toString().contains(QStringLiteral("0bf669f2-45f2-11e7-9598-0800200c9a66"))) {
        QLowEnergyCharacteristic characteristic =
            service->characteristic(QBluetoothUuid(QStringLiteral("0bf669f3-45f2-11e7-9598-0800200c9a66")));
        QLowEnergyCharacteristic characteristic2 =
            service->characteristic(QBluetoothUuid(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66")));

        Q_ASSERT(characteristic.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }

        QByteArray reply;
        if (((uint8_t)newValue.at(1)) == 0xA1) {

            // f0 a1 06 01 0b 00 33 0c 03 e5
            // f0 a1 06 01 0b 00 33 06 03 df
            reply.append(0xf0);
            reply.append(0xa1);
            reply.append(0x06);
            reply.append((char)0x01);
            reply.append(0x0b);
            reply.append((char)0x00);
            reply.append(0x33);
            reply.append(0x0c);
            reply.append(0x03);
            reply.append(0xe5);
            writeCharacteristic(service, characteristic, reply);
            echelonWriteStatus();
            echelonInitDone = true;
        } else if (((uint8_t)newValue.at(1)) == 0xA3) {

            // f0 a3 02 20 01 b6
            reply.append(0xf0);
            reply.append(0xa3);
            reply.append(0x02);
            reply.append(0x20);
            reply.append(0x01);
            reply.append(0xb6);
            writeCharacteristic(service, characteristic, reply);
        } else if (((uint8_t)newValue.at(1)) == 0xA4) {

            // f0 a4 .. .. ..
            reply.append(0xf0);
            reply.append(0xa4);
            reply.append(0x01);
            reply.append((char)0x00);
            reply.append(0x95);

            writeCharacteristic(service, characteristic, reply);
        }
        // f0 b0 01 00 a1
        else if (((uint8_t)newValue.at(1)) == 0xB0 && ((uint8_t)newValue.at(3)) == 0x00) {

            // f0 d0 01 00 c1
            reply.append(0xf0);
            reply.append(0xd0);
            reply.append(0x01);
            reply.append((char)0x00);
            reply.append(0xc1);
            writeCharacteristic(service, characteristic, reply);
        }
        // f0 b0 01 01 a2
        else if (((uint8_t)newValue.at(1)) == 0xB0) {

            // f0 d0 01 01 c2
            reply.append(0xf0);
            reply.append(0xd0);
            reply.append(0x01);
            reply.append(0x01);
            reply.append(0xc2);
            writeCharacteristic(service, characteristic2, reply);
            echelonWriteResistance();
        } else if (((uint8_t)newValue.at(1)) == 0xA0) {

            reply = newValue;
            writeCharacteristic(service, characteristic, reply);
        }
    }
}

int virtualbike::iFit_pelotonToBikeResistance(int pelotonResistance) {
    if (pelotonResistance <= 10) {
        return 2;
    }
    if (pelotonResistance <= 20) {
        return 4;
    }
    if (pelotonResistance <= 25) {
        return 7;
    }
    if (pelotonResistance <= 30) {
        return 9;
    }
    if (pelotonResistance <= 35) {
        return 0xb;
    }
    if (pelotonResistance <= 40) {
        return 0xe;
    }
    if (pelotonResistance <= 45) {
        return 0x10;
    }
    if (pelotonResistance <= 50) {
        return 0x13;
    }
    if (pelotonResistance <= 55) {
        return 0x15;
    }
    if (pelotonResistance <= 60) {
        return 0x18;
    }
    if (pelotonResistance <= 65) {
        return 0x1a;
    }
    if (pelotonResistance <= 70) {
        return 0x1d;
    }
    if (pelotonResistance <= 75) {
        return 0x1f;
    }
    if (pelotonResistance <= 80) {
        return 0x21;
    }
    if (pelotonResistance <= 85) {
        return 0x24;
    }
    if (pelotonResistance <= 100) {
        return 0x26;
    }
    return 0x02;
}

int virtualbike::iFit_resistanceToIfit(int ifitResistance) {
    switch(ifitResistance) {
    case 38:
        return 15;
    case 36:
        return 14;
    case 33:
        return 13;
    case 31:
        return 12;
    case 29:
        return 11;
    case 26:
        return 10;
    case 24:
        return 9;
    case 21:
        return 8;
    case 19:
        return 7;
    case 16:
        return 6;
    case 14:
        return 5;
    case 11:
        return 4;
    case 9:
        return 3;
    case 7:
        return 2;
    case 4:
        return 1;
    default:
    case 2:
        return 0;

    }
}

void virtualbike::writeCharacteristic(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic,
                                      const QByteArray &value) {
    try {
        qDebug() << QStringLiteral("virtualbike::writeCharacteristic ") + service->serviceName() + QStringLiteral(" ") +
                        characteristic.name() + QStringLiteral(" ") + value.toHex(' ');
        service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    } catch (...) {
        qDebug() << QStringLiteral("virtual bike error!");
    }
}

void virtualbike::reconnect() {

    QSettings settings;
    bool bluetooth_relaxed =
        settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();

    if (bluetooth_relaxed) {
        return;
    }

    bool zwift_play_emulator = settings.value(QZSettings::zwift_play_emulator, QZSettings::default_zwift_play_emulator).toBool();
    bool watt_bike_emulator = settings.value(QZSettings::watt_bike_emulator, QZSettings::default_watt_bike_emulator).toBool();
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool battery = settings.value(QZSettings::battery_service, QZSettings::default_battery_service).toBool();
    bool power = settings.value(QZSettings::bike_power_sensor, QZSettings::default_bike_power_sensor).toBool();
    bool service_changed = settings.value(QZSettings::service_changed, QZSettings::default_service_changed).toBool();
    bool heart_only =
        settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();
    bool echelon =
        settings.value(QZSettings::virtual_device_echelon, QZSettings::default_virtual_device_echelon).toBool();
    bool ifit = settings.value(QZSettings::virtual_device_ifit, QZSettings::default_virtual_device_ifit).toBool();

    qDebug() << QStringLiteral("virtualbike::reconnect");
    leController->disconnectFromDevice();

#ifndef Q_OS_IOS
    if (service_changed) {
        serviceChanged = leController->addService(serviceDataChanged);
    }

    if (!echelon && !ifit) {
        if (!heart_only) {
            if (!cadence && !power) {

                serviceFIT = leController->addService(serviceDataFIT);

                if(zwift_play_emulator) {
                    QThread::msleep(100); // give time to Android to add the service async.ly
                    serviceZwiftPlayBike = leController->addService(serviceDataZwiftPlayBike);
                } else if(watt_bike_emulator) {
                    QThread::msleep(100); // give time to Android to add the service async.ly
                    serviceWattAtomBike = leController->addService(serviceDataWattAtomBike);
                }
            } else {
                service = leController->addService(serviceData);
            }
        }
    } else if (ifit) {
        service = leController->addService(serviceData);
    } else {

        service = leController->addService(serviceEchelon);
        QThread::msleep(100); // give time to Android to add the service async.ly
        service = leController->addService(serviceData);
    }
    QThread::msleep(100); // give time to Android to add the service async.ly

    if (battery)
        serviceBattery = leController->addService(serviceDataBattery);

    if (!this->noHeartService || heart_only)
        serviceHR = leController->addService(serviceDataHR);
#endif

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);
    leController->startAdvertising(pars, advertisingData, advertisingData);
}

void virtualbike::bikeProvider() {

    QSettings settings;
    bool zwift_play_emulator = settings.value(QZSettings::zwift_play_emulator, QZSettings::default_zwift_play_emulator).toBool();
    bool watt_bike_emulator = settings.value(QZSettings::watt_bike_emulator, QZSettings::default_watt_bike_emulator).toBool();
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool battery = settings.value(QZSettings::battery_service, QZSettings::default_battery_service).toBool();
    bool power = settings.value(QZSettings::bike_power_sensor, QZSettings::default_bike_power_sensor).toBool();
    bool heart_only =
        settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();
    bool echelon =
        settings.value(QZSettings::virtual_device_echelon, QZSettings::default_virtual_device_echelon).toBool();
    bool ifit = settings.value(QZSettings::virtual_device_ifit, QZSettings::default_virtual_device_ifit).toBool();
    bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool();

    double normalizeWattage = Bike->wattsMetricforUI();
    if (normalizeWattage < 0)
        normalizeWattage = 0;

    uint16_t normalizeSpeed = (uint16_t)qRound(Bike->currentSpeed().value() * 100);

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    if (h) {
        // really connected to a device
        if (h->virtualbike_updateFTMS(normalizeSpeed, (char)Bike->currentResistance().value(),
                                      (uint16_t)Bike->currentCadence().value() * 2, (uint16_t)normalizeWattage,
                                      Bike->currentCrankRevolutions(), Bike->lastCrankEventTime(), ((bike*)Bike)->gears(), Bike->calories().value(), Bike->odometer() * 1000.0, static_cast<uint8_t>(Bike->deviceType()))) {
            h->virtualbike_setHeartRate(Bike->currentHeart().value());

            uint8_t ftms_message[255];
            int ret = h->virtualbike_getLastFTMSMessage(ftms_message);
            if (ret > 0) {
                lastFTMSFrameReceived = QDateTime::currentMSecsSinceEpoch();
                qDebug() << "FTMS rcv << " << QByteArray::fromRawData((char *)ftms_message, ret).toHex(' ');
                emit ftmsCharacteristicChanged(QLowEnergyCharacteristic(),
                                               QByteArray::fromRawData((char *)ftms_message, ret));
            }
            qDebug() << "last FTMS rcv" << lastFTMSFrameReceived;
            if (lastFTMSFrameReceived > 0) {
                if (!erg_mode)
                    writeP2AD9->changeSlope(h->virtualbike_getCurrentSlope(), h->virtualbike_getCurrentCRR(),
                                            h->virtualbike_getCurrentCW());
                else {
                    qDebug() << "ios workaround power changed request" << h->virtualbike_getPowerRequested();
                    writeP2AD9->changePower(h->virtualbike_getPowerRequested());
                }
            }
        }
        return;
    }
#endif
#else
    Q_UNUSED(erg_mode);
#endif

    qDebug() << QStringLiteral("bikeProvider") << whenLastFTMSFrameReceived()
             << (qint64)(whenLastFTMSFrameReceived() + ((qint64)2000)) << erg_mode;
    // zwift with the last update, seems to sending power request only when it actually wants to change it
    // so i need to keep this on to the bike
    if (whenLastFTMSFrameReceived() > 0 &&
        (QDateTime::currentMSecsSinceEpoch() > (qint64)(whenLastFTMSFrameReceived() + ((qint64)2000))) && erg_mode) {
        qDebug() << QStringLiteral("zwift is not sending the power anymore, let's continue with the last value");
        writeP2AD9->changePower(((bike *)Bike)->lastRequestedPower().value());
    }

    if (leController->state() != QLowEnergyController::ConnectedState) {
        qDebug() << QStringLiteral("virtual bike bluetooth not connected");

        return;
    } else {
        bool bluetooth_relaxed =
            settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
        bool bluetooth_30m_hangs =
            settings.value(QZSettings::bluetooth_30m_hangs, QZSettings::default_bluetooth_30m_hangs).toBool();
        if (bluetooth_relaxed) {

            leController->stopAdvertising();
        }

        if (lastFTMSFrameReceived > 0 && QDateTime::currentMSecsSinceEpoch() > (lastFTMSFrameReceived + 5000) &&
            bluetooth_30m_hangs) {
            lastFTMSFrameReceived = 0;
            qDebug() << QStringLiteral("virtual bike timeout, reconnecting...");

            reconnect();
            return;
        }

        qDebug() << QStringLiteral("virtual bike connected");
    }

    QByteArray value;

    if (!echelon && !ifit) {
        if (!heart_only) {
            if (!cadence && !power) {
                value.clear();
                if (notif2AD2->notify(value) == CN_OK) {
                    if (!serviceFIT) {
                        qDebug() << QStringLiteral("serviceFIT not available");

                        return;
                    }

                    QLowEnergyCharacteristic characteristic =
                        serviceFIT->characteristic((QBluetoothUuid::CharacteristicType)0x2AD2);
                    Q_ASSERT(characteristic.isValid());
                    if (leController->state() != QLowEnergyController::ConnectedState) {
                        qDebug() << QStringLiteral("virtual bike not connected");

                        return;
                    }
                    writeCharacteristic(serviceFIT, characteristic, value);

                    if(zwift_play_emulator) {
                        QLowEnergyCharacteristic characteristic1 =
                            serviceZwiftPlayBike->characteristic(QBluetoothUuid(QStringLiteral("00000002-19ca-4651-86e5-fa29dcdd09d1")));
                        QByteArray response = CharacteristicWriteProcessor0003::encodeHubRidingData(Bike->wattsMetricforUI(), Bike->currentCadence().value(), 0,
                                                                              Bike->wattsMetricforUI(),
                                                                              CharacteristicWriteProcessor0003::calculateUnknown1(Bike->wattsMetricforUI()),
                                                                              0);
                        writeCharacteristic(serviceZwiftPlayBike, characteristic1, response);
                    } else if (watt_bike_emulator) {
                        QLowEnergyCharacteristic characteristic1 =
                            serviceWattAtomBike->characteristic(QBluetoothUuid(QStringLiteral("b4cc1224-bc02-4cae-adb9-1217ad2860d1")));
                        value.clear();
                        static uint8_t seq = 0;
                        ++seq;
                        value.append(seq);
                        value.append(0x03);
                        value.append(0xB6);
                        int8_t g = ((bike*)Bike)->gears();
                        if(g < 1)
                            g = 1;
                        value.append(g);
                        writeCharacteristic(serviceWattAtomBike, characteristic1, value);
                    }
                }
            } else if (power) {
                value.clear();
                if (notif2A63->notify(value) == CN_OK) {

                    if (!service) {
                        qDebug() << QStringLiteral("service not available");

                        return;
                    }

                    QLowEnergyCharacteristic characteristic =
                        service->characteristic(QBluetoothUuid::CharacteristicType::CyclingPowerMeasurement);
                    Q_ASSERT(characteristic.isValid());
                    if (leController->state() != QLowEnergyController::ConnectedState) {
                        qDebug() << QStringLiteral("virtual bike not connected");

                        return;
                    }
                    writeCharacteristic(service, characteristic, value);
                }
            } else {
                value.clear();
                if (notif2A5B->notify(value) == CN_OK) {

                    if (!service) {
                        qDebug() << QStringLiteral("service not available");

                        return;
                    }

                    QLowEnergyCharacteristic characteristic =
                        service->characteristic(QBluetoothUuid::CharacteristicType::CSCMeasurement);
                    Q_ASSERT(characteristic.isValid());
                    if (leController->state() != QLowEnergyController::ConnectedState) {
                        qDebug() << QStringLiteral("virtual bike not connected");

                        return;
                    }
                    writeCharacteristic(service, characteristic, value);
                }
            }
        }
    } else if (ifit) {
        // timeout di 500 ms
        qDebug() << QStringLiteral("iFit Last Frame") << iFit_TSLastFrame;
        if (iFit_TSLastFrame != 0 && iFit_TSLastFrame + 500 < QDateTime::currentMSecsSinceEpoch()) {
            qDebug() << QStringLiteral("iFit timeout!");
            /*
            QLowEnergyCharacteristic characteristic =
                service->characteristic(QBluetoothUuid(QStringLiteral("00001534-1412-efde-1523-785feabcd123")));
            QByteArray copy;
            foreach(uint8_t a, iFit_LastFrameReceived)
                copy.append(a);
            characteristicChanged(characteristic, copy);
*/
        }
    } else {

        if (echelonInitDone) {
            echelonWriteStatus();
            echelonWriteResistance();
        }
    }
    // characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    // Q_ASSERT(characteristic.isValid());
    // service->readCharacteristic(characteristic);

    if (battery) {
        if (!serviceBattery) {
            qDebug() << QStringLiteral("serviceBattery not available");

            return;
        }

        QByteArray valueBattery;
        valueBattery.append(100); // Actual value.
        QLowEnergyCharacteristic characteristicBattery = serviceBattery->characteristic(QBluetoothUuid::CharacteristicType::BatteryLevel);

        Q_ASSERT(characteristicBattery.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }
        writeCharacteristic(serviceBattery, characteristicBattery, valueBattery);
    }

    if (!this->noHeartService || heart_only) {
        if (!serviceHR) {
            qDebug() << QStringLiteral("serviceHR not available");

            return;
        }

        QByteArray valueHR;
        if (notif2A37->notify(valueHR) == CN_OK) {
            QLowEnergyCharacteristic characteristicHR = serviceHR->characteristic(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);

            Q_ASSERT(characteristicHR.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual bike not connected");

                return;
            }
            writeCharacteristic(serviceHR, characteristicHR, valueHR);
        }
    }
}

void virtualbike::echelonWriteStatus() {
    QByteArray value;

    // f0 d1 09 00 00 00 00 00 01 00 5f 00 2a
    value.append(0xf0);
    value.append(0xd1);
    value.append(0x09);
    value.append((char)0x00);                                                      // elapsed
    value.append((char)0x00);                                                      // elapsed
    value.append((uint8_t)(((uint32_t)(Bike->odometer() * 1.60934 * 100)) >> 24)); // distance
    value.append((uint8_t)(((uint32_t)(Bike->odometer() * 1.60934 * 100)) >> 16)); // distance
    value.append((uint8_t)(((uint32_t)(Bike->odometer() * 1.60934 * 100)) >> 8));  // distance
    value.append((uint8_t)(Bike->odometer() * 1.60934 * 100));                     // distance
    value.append((char)0x00);
    value.append(Bike->currentCadence().value());
    value.append((uint8_t)Bike->currentHeart().value());

    uint8_t sum = 0;
    for (uint8_t i = 0; i < value.length(); i++) {

        sum += value[i]; // the last byte is a sort of a checksum
    }
    value.append(sum);

    if (!service) {
        qDebug() << QStringLiteral("service not available");

        return;
    }

    QLowEnergyCharacteristic characteristic =
        service->characteristic(QBluetoothUuid(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66")));
    Q_ASSERT(characteristic.isValid());
    if (leController->state() != QLowEnergyController::ConnectedState) {
        qDebug() << QStringLiteral("virtual bike not connected");

        return;
    }
    writeCharacteristic(service, characteristic, value);
}

void virtualbike::echelonWriteResistance() {

    QSettings settings;
    double bikeResistanceOffset =
        settings.value(QZSettings::echelon_resistance_offset, QZSettings::default_echelon_resistance_offset).toInt();
    double bikeResistanceGain =
        settings.value(QZSettings::echelon_resistance_gain, QZSettings::default_echelon_resistance_gain).toDouble();
    double CurrentResistance = (Bike->currentResistance().value() * bikeResistanceGain) + bikeResistanceOffset;

    // resistance change notification
    // f0 d2 01 0b ce
    QByteArray resistance;
    static resistance_t oldresistance = 255;
    resistance.append(0xf0);
    resistance.append(0xd2);
    resistance.append(0x01);
    resistance.append(CurrentResistance);

    uint8_t sum = 0;
    for (uint8_t i = 0; i < resistance.length(); i++) {

        sum += resistance[i]; // the last byte is a sort of a checksum
    }
    resistance.append(sum);
    if (oldresistance != ((resistance_t)Bike->currentResistance().value())) {
        QLowEnergyCharacteristic characteristic =
            service->characteristic(QBluetoothUuid(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66")));
        Q_ASSERT(characteristic.isValid());
        if (leController->state() != QLowEnergyController::ConnectedState) {
            qDebug() << QStringLiteral("virtual bike not connected");

            return;
        }

        writeCharacteristic(service, characteristic, resistance);
    }
    oldresistance = ((resistance_t)CurrentResistance);
}

bool virtualbike::connected() {
    if (!leController) {

        return false;
    }
    return leController->state() == QLowEnergyController::ConnectedState;
}

void virtualbike::error(QLowEnergyController::Error newError) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("virtualbike::controller:ERROR ") +
                    QString::fromLocal8Bit(metaEnum.valueToKey(newError));

    if (newError != QLowEnergyController::RemoteHostClosedError) {
        reconnect();
    }
}

void virtualbike::dirconFtmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                  const QByteArray &newValue) {
    lastDirconFTMSFrameReceived = QDateTime::currentMSecsSinceEpoch();
    qDebug() << QStringLiteral("lastDirconFTMSFrameReceived") << lastDirconFTMSFrameReceived;
}
