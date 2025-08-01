#include "virtualdevices/virtualtreadmill.h"
#include <QThread>
#include <QSettings>
#include <QtMath>
#include <chrono>

#ifdef Q_OS_ANDROID
#include "androidactivityresultreceiver.h"
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

using namespace std::chrono_literals;

virtualtreadmill::virtualtreadmill(bluetoothdevice *t, bool noHeartService) {
    QSettings settings;
    treadMill = t;

    int bikeResistanceOffset =
        settings.value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset).toInt();
    double bikeResistanceGain =
        settings.value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f).toDouble();
    this->noHeartService = noHeartService;
    if (settings.value(QZSettings::dircon_yes, QZSettings::default_dircon_yes).toBool()) {
        dirconManager = new DirconManager(t, bikeResistanceOffset, bikeResistanceGain, this);
        connect(dirconManager, SIGNAL(changeInclination(double, double)), this,
                SIGNAL(changeInclination(double, double)));
        connect(dirconManager, SIGNAL(changeInclination(double, double)), this,
                SLOT(dirconChangedInclination(double, double)));
        connect(dirconManager, SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
                SIGNAL(ftmsCharacteristicChanged(QLowEnergyCharacteristic, QByteArray)));
    }
    if (!settings.value(QZSettings::virtual_device_bluetooth, QZSettings::default_virtual_device_bluetooth).toBool())
        return;
    notif2AD9 = new CharacteristicNotifier2AD9(t, this);
    notif2AD2 = new CharacteristicNotifier2AD2(t, this);
    notif2ACD = new CharacteristicNotifier2ACD(t, this);
    notif2A53 = new CharacteristicNotifier2A53(t, this);
    notif2A37 = new CharacteristicNotifier2A37(t, this);
    writeP2AD9 = new CharacteristicWriteProcessor2AD9(bikeResistanceGain, bikeResistanceOffset, t, notif2AD9, this);
    connect(writeP2AD9, SIGNAL(changeInclination(double, double)), this, SIGNAL(changeInclination(double, double)));
    connect(writeP2AD9, SIGNAL(slopeChanged()), this, SLOT(slopeChanged()));

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    bool garmin_bluetooth_compatibility = settings.value(QZSettings::garmin_bluetooth_compatibility, QZSettings::default_garmin_bluetooth_compatibility).toBool();
    if (ios_peloton_workaround) {
        qDebug() << "ios_zwift_workaround activated!";
        h = new lockscreen();
        h->virtualtreadmill_zwift_ios(garmin_bluetooth_compatibility);
    } else
#endif
#endif
    {
        //! [Advertising Data]
        advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
        advertisingData.setIncludePowerLevel(true);
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
        advertisingData.setLocalName(QStringLiteral("KICKR RUN"));
#else            
        advertisingData.setLocalName(QStringLiteral("DomyosBridge"));
#endif
        QList<QBluetoothUuid> services;

        // Add Wahoo Run Service UUID
        /*QBluetoothUuid wahooRunServiceUuid(QString("A026EE0E-0A7D-4AB3-97FA-F1500F9FEB8B"));
        services << wahooRunServiceUuid;*/

        if (ftmsServiceEnable()) {
            services << ((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
        }

        if (RSCEnable()) {
            services << (QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence);
        }

        if (noHeartService == false) {
            services << QBluetoothUuid::HeartRate;
        }

        /*services << ((QBluetoothUuid::ServiceClassUuid)0xFF00);
        services << QBluetoothUuid::DeviceInformation;*/

        advertisingData.setServices(services);
        //! [Advertising Data]

        // Add Device Information Service
        QLowEnergyCharacteristicData manufacturerNameChar;
        manufacturerNameChar.setUuid(QBluetoothUuid::CharacteristicType::ManufacturerNameString);
        manufacturerNameChar.setProperties(QLowEnergyCharacteristic::Read);
        manufacturerNameChar.setValue(QByteArray("Wahoo Fitness")); // Changed to Wahoo Fitness

        QLowEnergyCharacteristicData firmwareRevChar;
        firmwareRevChar.setUuid(QBluetoothUuid::CharacteristicType::FirmwareRevisionString);
        firmwareRevChar.setProperties(QLowEnergyCharacteristic::Read);
        firmwareRevChar.setValue(QByteArray("1.0.11"));

        QLowEnergyCharacteristicData hardwareRevChar;
        hardwareRevChar.setUuid(QBluetoothUuid::CharacteristicType::HardwareRevisionString);
        hardwareRevChar.setProperties(QLowEnergyCharacteristic::Read);
        hardwareRevChar.setValue(QByteArray("1"));

        // Create Device Information Service        
        serviceDataDIS.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceDataDIS.setUuid(QBluetoothUuid::DeviceInformation);
        serviceDataDIS.addCharacteristic(manufacturerNameChar);
        serviceDataDIS.addCharacteristic(firmwareRevChar);
        serviceDataDIS.addCharacteristic(hardwareRevChar);

        // Create Wahoo Run Service        
        serviceDataWahoo.setType(QLowEnergyServiceData::ServiceTypePrimary);
        serviceDataWahoo.setUuid(QBluetoothUuid(QString("A026EE0E-0A7D-4AB3-97FA-F1500F9FEB8B")));

        // Add Wahoo Run Notify Characteristic
        QLowEnergyCharacteristicData wahooNotifyChar;
        wahooNotifyChar.setUuid(QBluetoothUuid(QString("A026E03D-0A7D-4AB3-97FA-F1500F9FEB8B")));
        wahooNotifyChar.setProperties(QLowEnergyCharacteristic::Read |
                                    QLowEnergyCharacteristic::WriteNoResponse |
                                    QLowEnergyCharacteristic::Notify);
        const QLowEnergyDescriptorData wahooNotifyConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                        QByteArray(2, 0));
        wahooNotifyChar.addDescriptor(wahooNotifyConfig);

        // Add Wahoo Run Write Characteristic
        QLowEnergyCharacteristicData wahooWriteChar;
        wahooWriteChar.setUuid(QBluetoothUuid(QString("A026E03E-0A7D-4AB3-97FA-F1500F9FEB8B")));
        wahooWriteChar.setProperties(QLowEnergyCharacteristic::WriteNoResponse |
                                   QLowEnergyCharacteristic::Indicate);
        const QLowEnergyDescriptorData wahooWriteConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                       QByteArray(2, 0));
        wahooWriteChar.addDescriptor(wahooWriteConfig);

        serviceDataWahoo.addCharacteristic(wahooNotifyChar);
        serviceDataWahoo.addCharacteristic(wahooWriteChar);

        //! [Service Data]
        if (ftmsServiceEnable()) {
            QLowEnergyCharacteristicData charData;
            charData.setUuid((QBluetoothUuid::CharacteristicType)0x2ACC);  // FitnessMachineFeatureCharacteristicUuid
            QByteArray value;
            value.append(0x08);
            value.append((char)0x14);  // heart rate and elapsed time
            value.append((char)0x00);
            value.append((char)0x00);
            value.append((char)0x00);
            value.append((char)0x00);
            value.append((char)0x00);
            value.append((char)0x00);
            charData.setValue(value);
            charData.setProperties(QLowEnergyCharacteristic::Read);

            /*    const QLowEnergyDescriptorData clientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                               QByteArray(2, 0));
           charData.addDescriptor(clientConfig);*/

               QLowEnergyCharacteristicData charData2;
               if (ftmsTreadmillEnable()) {
                   charData2.setUuid((QBluetoothUuid::CharacteristicType)0x2ACD); // TreadmillDataCharacteristicUuid
                   charData2.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
                   QByteArray descriptor;
                   descriptor.append((char)0x01);
                   descriptor.append((char)0x00);
                   const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                                descriptor);
                   charData2.addDescriptor(clientConfig2);
               }

               QLowEnergyCharacteristicData charData3;
               charData3.setUuid((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
               charData3.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
               const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                             QByteArray(2, 0));
               charData3.addDescriptor(cpClientConfig);

               QLowEnergyCharacteristicData charDataFIT5;
               charDataFIT5.setUuid((QBluetoothUuid::CharacteristicType)0x2ADA); // Fitness Machine status
               charDataFIT5.setProperties(QLowEnergyCharacteristic::Notify);
               QByteArray descriptor5;
               descriptor5.append((char)0x01);
               descriptor5.append((char)0x00);
               const QLowEnergyDescriptorData clientConfig5(QBluetoothUuid::ClientCharacteristicConfiguration,
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
               const QLowEnergyDescriptorData clientConfig6(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                            descriptor6);
               charDataFIT6.addDescriptor(clientConfig6);
               charDataFIT6.setProperties(QLowEnergyCharacteristic::Read);

               // indoor bike in order to exploit a zwift bug that allows treadmill to get the incline values
               QLowEnergyCharacteristicData charDataFIT7;
               charDataFIT7.setUuid((QBluetoothUuid::CharacteristicType)0x2AD2); // indoor bike
               charDataFIT7.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
               QByteArray descriptor7;
               descriptor7.append((char)0x01);
               descriptor7.append((char)0x00);
               const QLowEnergyDescriptorData clientConfig7(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                            descriptor7);
               charDataFIT7.addDescriptor(clientConfig7);

               QLowEnergyCharacteristicData charDataFIT2;
               charDataFIT2.setUuid(
                   (QBluetoothUuid::CharacteristicType)0x2AD6); // supported_resistance_level_rangeCharacteristicUuid
               charDataFIT2.setProperties(QLowEnergyCharacteristic::Read);
               QByteArray valueFIT2;
               valueFIT2.append((char)0x0A); // min resistance value
               valueFIT2.append((char)0x00); // min resistance value
               valueFIT2.append((char)0x96); // max resistance value
               valueFIT2.append((char)0x00); // max resistance value
               valueFIT2.append((char)0x0A); // step resistance
               valueFIT2.append((char)0x00); // step resistance
               charDataFIT2.setValue(valueFIT2);

               serviceDataFTMS.setType(QLowEnergyServiceData::ServiceTypePrimary);
               serviceDataFTMS.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); // FitnessMachineServiceUuid
               serviceDataFTMS.addCharacteristic(charData);
               if (ftmsTreadmillEnable())
                   serviceDataFTMS.addCharacteristic(charData2);
               serviceDataFTMS.addCharacteristic(charData3);
               serviceDataFTMS.addCharacteristic(charDataFIT5);
               serviceDataFTMS.addCharacteristic(charDataFIT6);
               serviceDataFTMS.addCharacteristic(charDataFIT7);
               serviceDataFTMS.addCharacteristic(charDataFIT2);
           }

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
        qDebug() << "Raspberry workaround for sending metrics to the peloton app";
        QLowEnergyCharacteristicData charDataFIT;
        charDataFIT.setUuid((QBluetoothUuid::CharacteristicType)0x2A00); 
        QByteArray valueFIT;
        valueFIT.append((char)'K'); // average speed, cadence and resistance level supported
        valueFIT.append((char)'I'); // heart rate and elapsed time
        valueFIT.append((char)'C');
        valueFIT.append((char)'K');
        valueFIT.append((char)'R'); // resistance and power target supported
        valueFIT.append((char)' '); // indoor simulation, wheel and spin down supported
        valueFIT.append((char)'R');
        valueFIT.append((char)'U');
        valueFIT.append((char)'N');
        valueFIT.append((char)0x00);
        charDataFIT.setValue(valueFIT);
        charDataFIT.setProperties(QLowEnergyCharacteristic::Read);

        QLowEnergyCharacteristicData charDataFIT2;
        charDataFIT2.setUuid((QBluetoothUuid::CharacteristicType)0x2A01);
        QByteArray valueFIT2;
        valueFIT2.append((char)0x00);
        charDataFIT2.setValue(valueFIT2);
        charDataFIT2.setProperties(QLowEnergyCharacteristic::Read);

        genericAccessServerData.setUuid((QBluetoothUuid::ServiceClassUuid)0x1800);
        genericAccessServerData.addCharacteristic(charDataFIT);
        genericAccessServerData.addCharacteristic(charDataFIT2);

        QLowEnergyCharacteristicData charDataFIT3;
        charDataFIT3.setUuid((QBluetoothUuid::CharacteristicType)0x2A05);
        charDataFIT3.setProperties(QLowEnergyCharacteristic::Indicate);
        QByteArray descriptor33;
        descriptor33.append((char)0x02);
        descriptor33.append((char)0x00);
        const QLowEnergyDescriptorData clientConfig43(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                        descriptor33);
        charDataFIT3.addDescriptor(clientConfig43);

        genericAttributeServiceData.setUuid((QBluetoothUuid::ServiceClassUuid)0x1801);
        genericAttributeServiceData.addCharacteristic(charDataFIT3);
#endif              

           if (RSCEnable()) {
               QLowEnergyCharacteristicData charData;
               charData.setUuid(QBluetoothUuid::CharacteristicType::RSCFeature);
               charData.setProperties(QLowEnergyCharacteristic::Read);
               QByteArray value;
               value.append((char)0x02); // Total Distance Measurement Supported
               value.append((char)0x00);
               charData.setValue(value);

               QLowEnergyCharacteristicData charData2;
               charData2.setUuid(QBluetoothUuid::CharacteristicType::SensorLocation);
               charData2.setProperties(QLowEnergyCharacteristic::Read);
               QByteArray valueLocaltion;
               valueLocaltion.append((char)1); // Top of shoe
               charData2.setValue(valueLocaltion);
               /*const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                           QByteArray(2, 0));
               charData2.addDescriptor(clientConfig2);*/

               QLowEnergyCharacteristicData charData3;
               charData3.setUuid(QBluetoothUuid::CharacteristicType::RSCMeasurement);
               charData3.setProperties(QLowEnergyCharacteristic::Read | QLowEnergyCharacteristic::Notify);
               charData3.setValue(valueLocaltion);
               QByteArray descriptor;
               descriptor.append((char)0x01);
               descriptor.append((char)0x00);
               const QLowEnergyDescriptorData clientConfig4(QBluetoothUuid::ClientCharacteristicConfiguration, descriptor);
               charData3.addDescriptor(clientConfig4);

               QLowEnergyCharacteristicData charData4;
               charData4.setUuid(QBluetoothUuid::CharacteristicType::SCControlPoint);
               charData4.setProperties(QLowEnergyCharacteristic::Write | QLowEnergyCharacteristic::Indicate);
               const QLowEnergyDescriptorData cpClientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                             QByteArray(2, 0));
               charData4.addDescriptor(cpClientConfig);

               serviceDataRSC.setType(QLowEnergyServiceData::ServiceTypePrimary);
               serviceDataRSC.setUuid(QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence);
               serviceDataRSC.addCharacteristic(charData);
               serviceDataRSC.addCharacteristic(charData3);
               serviceDataRSC.addCharacteristic(charData2);
               serviceDataRSC.addCharacteristic(charData4);
           }
           //! [Service Data]

           if (noHeartService == false) {
               QLowEnergyCharacteristicData charDataHR;
               charDataHR.setUuid(QBluetoothUuid::HeartRateMeasurement);
               charDataHR.setValue(QByteArray(2, 0));
               charDataHR.setProperties(QLowEnergyCharacteristic::Notify);
                const QLowEnergyDescriptorData clientConfigHR(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                              QByteArray(2, 0));
                charDataHR.addDescriptor(clientConfigHR);

                serviceDataHR.setType(QLowEnergyServiceData::ServiceTypePrimary);
                serviceDataHR.setUuid(QBluetoothUuid::HeartRate);
                serviceDataHR.addCharacteristic(charDataHR);
            }

        //! [Start Advertising]
        leController = QLowEnergyController::createPeripheral();
        Q_ASSERT(leController);

        if (ftmsServiceEnable())
            serviceFTMS = leController->addService(serviceDataFTMS);
        QThread::msleep(100);
        
        if (RSCEnable())
            serviceRSC = leController->addService(serviceDataRSC);
        QThread::msleep(100);
        
        serviceDIS = leController->addService(serviceDataDIS);
        QThread::msleep(100);
        
        serviceWahoo = leController->addService(serviceDataWahoo);
        QThread::msleep(100);

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)        
        genericAccessServer = leController->addService(genericAccessServerData);
        genericAttributeService = leController->addService(genericAttributeServiceData);
#endif          
        
        if (noHeartService == false) {
            serviceHR = leController->addService(serviceDataHR);
        }

        if (serviceFTMS)
            QObject::connect(serviceFTMS, &QLowEnergyService::characteristicChanged, this,
                             &virtualtreadmill::characteristicChanged);

        if (serviceWahoo)
            QObject::connect(serviceWahoo, &QLowEnergyService::characteristicChanged, this,
                             &virtualtreadmill::wahooCharacteristicChanged);

        bool bluetooth_relaxed =
            settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
        QLowEnergyAdvertisingParameters pars = QLowEnergyAdvertisingParameters();
        if (!bluetooth_relaxed) {
#if !defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
            pars.setInterval(100, 100);
#endif            
        }  

#ifdef Q_OS_ANDROID
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/BleAdvertiser",
                                                 "startAdvertisingTreadmill",
                                                 "(Landroid/content/Context;)V",
                                                 QtAndroid::androidContext().object());

#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
        pars.setInterval(30, 50);
        leController->startAdvertising(pars, advertisingData);
#else
        leController->startAdvertising(pars, advertisingData, advertisingData);
#endif
        //! [Start Advertising]

        QObject::connect(leController, &QLowEnergyController::disconnected, this, &virtualtreadmill::reconnect);
    }

    QObject::connect(&treadmillTimer, &QTimer::timeout, this, &virtualtreadmill::treadmillProvider);
    if (settings.value(QZSettings::race_mode, QZSettings::default_race_mode).toBool())
        treadmillTimer.start(100ms);
    else
        treadmillTimer.start(1s);
}

void virtualtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicChanged ") + QString::number(characteristic.uuid().toUInt16()) +
                    QStringLiteral(" ") << newValue.toHex(' ');
    QByteArray reply;

    switch (characteristic.uuid().toUInt16()) {
    case 0x2AD9: // Fitness Machine Control Point
        if (writeP2AD9->writeProcess(0x2AD9, newValue, reply) == CP_OK) {

            QLowEnergyCharacteristic characteristic =
                serviceFTMS->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");
    
                return;
            }
            try {
                qDebug() << QStringLiteral("virtualtreadmill::writeCharacteristic ") + serviceFTMS->serviceName() +
                                QStringLiteral(" ") + characteristic.name() + QStringLiteral(" ") + reply.toHex(' ');
                serviceFTMS->writeCharacteristic(characteristic, reply); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtual treadmill error!");
            }
        }
        break;
    }
}

void virtualtreadmill::slopeChanged() {
    lastSlopeChanged = QDateTime::currentSecsSinceEpoch();
    
    if (treadMill && treadMill->autoResistance())
        m_autoInclinationEnabled = true;
    else
        m_autoInclinationEnabled = false;

}

void virtualtreadmill::wahooCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    qDebug() << "Wahoo characteristic changed: " << characteristic.uuid().toString()
             << " value: " << newValue.toHex(' ');
    
    // Handle Wahoo characteristic changes here
    if (characteristic.uuid() == QBluetoothUuid(QString("A026E03D-0A7D-4AB3-97FA-F1500F9FEB8B"))) {
        // Handle notify characteristic
    } else if (characteristic.uuid() == QBluetoothUuid(QString("A026E03E-0A7D-4AB3-97FA-F1500F9FEB8B"))) {
        if(newValue.length() >= 3) {
              if(newValue.at(0) == 0x11) {
                  int16_t slope = (((uint8_t)newValue.at(1)) + (newValue.at(2) << 8));
                  writeP2AD9->changeSlope(slope, 0, 0);
              }
          }
    }
}

void virtualtreadmill::reconnect() {
    QSettings settings;
    bool bluetooth_relaxed =
        settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();

    if (bluetooth_relaxed) {
        return;
    }

    qDebug() << "virtualtreadmill reconnect " << treadMill->connected();
    
    if (ftmsServiceEnable())
        serviceFTMS = leController->addService(serviceDataFTMS);
    QThread::msleep(100);
    
    if (RSCEnable())
        serviceRSC = leController->addService(serviceDataRSC);
    QThread::msleep(100);
    
    serviceDIS = leController->addService(serviceDataDIS);
    QThread::msleep(100);
    
    serviceWahoo = leController->addService(serviceDataWahoo);
    QThread::msleep(100);

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    genericAccessServer = leController->addService(genericAccessServerData);
    genericAttributeService = leController->addService(genericAttributeServiceData);
#endif      
    
    if (noHeartService == false) {
        serviceHR = leController->addService(serviceDataHR);
    }

    QLowEnergyAdvertisingParameters pars;
    pars.setInterval(100, 100);

    if (serviceFTMS || serviceRSC || serviceWahoo) {
#ifdef Q_OS_ANDROID
        QAndroidJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/BleAdvertiser",
                                                 "startAdvertisingTreadmill",
                                                 "(Landroid/content/Context;)V",
                                                 QtAndroid::androidContext().object());
#else
        leController->startAdvertising(pars, advertisingData, advertisingData);
#endif
    }
}


void virtualtreadmill::treadmillProvider() {
    const uint64_t slopeTimeoutSecs = 30;
    QSettings settings;

    if ((uint64_t)QDateTime::currentSecsSinceEpoch() > lastSlopeChanged + slopeTimeoutSecs)
        m_autoInclinationEnabled = false;

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool double_cadence = settings
                              .value(QZSettings::powr_sensor_running_cadence_double,
                                     QZSettings::default_powr_sensor_running_cadence_double)
                              .toBool();
    double cadence_multiplier = 2.0;
    if (double_cadence)
        cadence_multiplier = 1.0;

    if (h) {
        bool real_inclination_to_virtual_treamill_bridge = settings.value(QZSettings::real_inclination_to_virtual_treamill_bridge, QZSettings::default_real_inclination_to_virtual_treamill_bridge).toBool();
        double inclination = treadMill->currentInclination().value();
        if(real_inclination_to_virtual_treamill_bridge) {
              double offset = settings.value(QZSettings::zwift_inclination_offset,
                                             QZSettings::default_zwift_inclination_offset).toDouble();
              double gain = settings.value(QZSettings::zwift_inclination_gain,
                                           QZSettings::default_zwift_inclination_gain).toDouble();
              inclination -= offset;
              inclination /= gain;
        }
    uint16_t normalizeSpeed = (uint16_t)qRound(treadMill->currentSpeed().value() * 100);

    // really connected to a device
    //if (h->virtualtreadmill_updateFTMS(
    //        normalizeSpeed, 0, (uint16_t)((treadmill *)treadMill)->currentCadence().value() * cadence_multiplier,
    //        (uint16_t)((treadmill *)treadMill)->wattsMetric().value(),
    //        inclination * 10, (uint64_t)(((treadmill *)treadMill)->odometer() * 1000.0))) {
    //    h->virtualtreadmill_setHeartRate(((treadmill *)treadMill)->currentHeart().value());
    //    lastSlopeChanged = h->virtualtreadmill_lastChangeCurrentSlope();

    uint16_t swiftSpeed = normalizeSpeed;
    uint16_t swiftCadence = (uint16_t)(((treadmill *)treadMill)->currentCadence().value() * cadence_multiplier);
    uint8_t swiftResistance = 0;
    uint16_t swiftWatt = (uint16_t)((treadmill *)treadMill)->wattsMetric().value();
    uint16_t swiftInclination = (uint16_t)(inclination * 10.0);
    uint64_t swiftDistance = (uint64_t)(((treadmill *)treadMill)->odometer() * 1000.0);
    uint16_t swiftCalories = ((treadmill *)treadMill)->calories().value();
    qint32 swiftSteps = ((treadmill *)treadMill)->currentStepCount().value();

    // Calculate Elapsed Time to pass
    QTime swift_elapsed = treadMill->elapsedTime();
    double swift_elapsed_seconds_double = (double)swift_elapsed.hour() * 3600.0 +
                                          (double)swift_elapsed.minute() * 60.0 +
                                          (double)swift_elapsed.second() +
                                          (double)swift_elapsed.msec() / 1000.0;
    uint16_t swiftElapsedTimeSeconds = (uint16_t)qRound(swift_elapsed_seconds_double);

    if (h->virtualtreadmill_updateFTMS(  // uses @objc public func updateFTMS in virtualtreadmill_zwift.swift
            swiftSpeed,
            swiftResistance,
            swiftCadence,
            swiftWatt,
            swiftInclination,
            swiftDistance,
            swiftCalories,
            swiftSteps,
            swiftElapsedTimeSeconds
            )) {
        h->virtualtreadmill_setHeartRate(((treadmill *)treadMill)->currentHeart().value());

        if ((uint64_t)QDateTime::currentSecsSinceEpoch() < lastSlopeChanged + slopeTimeoutSecs)
            writeP2AD9->changeSlope(h->virtualtreadmill_getCurrentSlope(), 0, 0);
        
        // Check for requested speed from FTMS and apply it
        double requestedSpeed = h->virtualtreadmill_getRequestedSpeed();
        if (requestedSpeed > 0 && requestedSpeed != treadMill->currentSpeed().value()) {
            if (treadMill->deviceType() == bluetoothdevice::TREADMILL) {
                ((treadmill *)treadMill)->changeSpeed(requestedSpeed);
                qDebug() << "virtualtreadmill: Applied requested speed from FTMS:" << requestedSpeed;
            }
        }
        }
        return;
    }
#endif
#endif

    if (leController->state() != QLowEnergyController::ConnectedState) {
        qDebug() << QStringLiteral("virtualtreadmill connection error");
        return;
    } else {
        QSettings settings;
        bool bluetooth_relaxed =
            settings.value(QZSettings::bluetooth_relaxed, QZSettings::default_bluetooth_relaxed).toBool();
        if (bluetooth_relaxed) {
            leController->stopAdvertising();
        }
    }

    QByteArray value;

    if (ftmsServiceEnable()) {
        if (ftmsTreadmillEnable()) {
            value.clear();
            if (notif2ACD->notify(value) == CN_OK) {
                if (!serviceFTMS) {
                    qDebug() << QStringLiteral("service not available");

                    return;
                }

                QLowEnergyCharacteristic characteristic =
                    serviceFTMS->characteristic((QBluetoothUuid::CharacteristicType)0x2ACD);
                Q_ASSERT(characteristic.isValid());
                if (leController->state() != QLowEnergyController::ConnectedState) {
                    qDebug() << QStringLiteral("virtual treadmill not connected");

                    return;
                }
                try {
                    serviceFTMS->writeCharacteristic(characteristic, value); // Potentially causes notification.
                } catch (...) {
                    qDebug() << QStringLiteral("virtualtreadmill error!");
                }
            }
        }
        value.clear();
        if (notif2AD2->notify(value) == CN_OK) {
            if (!serviceFTMS) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceFTMS->characteristic((QBluetoothUuid::CharacteristicType)0x2AD2);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                serviceFTMS->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    }
    if (RSCEnable()) {
        value.clear();
        if (notif2A53->notify(value) == CN_OK) {
            if (!serviceRSC) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceRSC->characteristic(QBluetoothUuid::CharacteristicType::RSCMeasurement);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                serviceRSC->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    }

    // characteristic
    //        = service->characteristic((QBluetoothUuid::CharacteristicType)0x2AD9); // Fitness Machine Control Point
    // Q_ASSERT(characteristic.isValid());
    // service->readCharacteristic(characteristic);

    if (noHeartService == false) {
        value.clear();
        if (notif2A53->notify(value) == CN_OK) {
            if (!serviceHR) {
                qDebug() << QStringLiteral("serviceFIT not available");

                return;
            }

            QLowEnergyCharacteristic characteristic =
                serviceHR->characteristic(QBluetoothUuid::CharacteristicType::HeartRateMeasurement);
            Q_ASSERT(characteristic.isValid());
            if (leController->state() != QLowEnergyController::ConnectedState) {
                qDebug() << QStringLiteral("virtual treadmill not connected");

                return;
            }
            try {
                serviceHR->writeCharacteristic(characteristic, value); // Potentially causes notification.
            } catch (...) {
                qDebug() << QStringLiteral("virtualtreadmill error!");
            }
        }
    }
}

bool virtualtreadmill::connected() {
    if (!leController) {
        return false;
    }
    return leController->state() == QLowEnergyController::ConnectedState;
}

// Setup           |  FTMS Service | FTMS Treadmill Data | FTMS Bike | RSC | Heart
// -------------------------------------------------------------------------------
// iOS FTMS        |      X        |          X          |     X     |     |   X
// iOS RSC         |      X        |                     |     X     |  X  |   X
// Android<10 FTMS |      X        |          X          |     X     |     |   X
// Android<10 RSC  |      X        |                     |     X     |  X  |   X
// Android>9 FTMS  |      X        |          X          |     X     |     |
// Android>9 RSC   |               |                     |           |  X  |

bool virtualtreadmill::ftmsServiceEnable() {
    QSettings settings;
    bool cadence = settings.value(QZSettings::run_cadence_sensor, QZSettings::default_run_cadence_sensor).toBool();
    if (!cadence)
        return true;
    if (noHeartService == false)
        return true;
    return false;
}

bool virtualtreadmill::ftmsTreadmillEnable() {
    QSettings settings;
    bool cadence = settings.value(QZSettings::run_cadence_sensor, QZSettings::default_run_cadence_sensor).toBool();
    if (!cadence)
        return true;
    return false;
}

bool virtualtreadmill::RSCEnable() {
    QSettings settings;
    bool cadence = settings.value(QZSettings::run_cadence_sensor, QZSettings::default_run_cadence_sensor).toBool();
    if (cadence)
        return true;
    return false;
}

void virtualtreadmill::dirconChangedInclination(double grade, double percentage) {
    Q_UNUSED(grade);
    Q_UNUSED(percentage);
    slopeChanged();
}
