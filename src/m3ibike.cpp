#include "m3ibike.h"
#include "virtualbike.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QSettings>
#include <QBluetoothLocalDevice>
#include <math.h>
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#if defined(Q_OS_ANDROID)
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include "scanrecordresult.h"
#include <QMetaObject>
#endif

static m3ibike * m_instance = 0;

#define RESISTANCE_OP(OP,V,D) \
    OP(1,4,10,V,D)\
    OP(5,9,20,V,D)\
    OP(10,11,30,V,D)\
    OP(12,14,40,V,D)\
    OP(15,16,50,V,D)\
    OP(17,18,60,V,D)\
    OP(19,20,70,V,D)\
    OP(21,22,80,V,D)\
    OP(23,24,90,V,D)

#define SET_PELOTON_RESISTANCE(MINV,MAXV,ZWIFTV,KEISERV,DEST)\
    else if (KEISERV>=MINV && KEISERV<=MAXV) {\
        angular_coeff = 10.0/(MAXV+1-MINV); \
        DEST = angular_coeff * KEISERV + ZWIFTV - angular_coeff * MINV; \
    }

KeiserM3iDeviceSimulator::~KeiserM3iDeviceSimulator() {
    if (dist_buff) delete[] dist_buff;
    if (dist_buff_time) delete[] dist_buff_time;
}
KeiserM3iDeviceSimulator::KeiserM3iDeviceSimulator() {
}

void KeiserM3iDeviceSimulator::_set_offsets() {
    this->time_o = this->time_old;
    this->calorie_o = this->calorie_old;
    this->distance_o = this->distance_old;
}

void KeiserM3iDeviceSimulator::fillTimeRFields(keiser_m3i_out_t * f, qint64 updateTime) {
    if (!sessionStart)
        sessionStart = updateTime;
    f->timeRms = this->sumTime;
    f->timeRAbsms = updateTime - sessionStart;
    f->timeR = (int)(this->sumTime / 1000.0 + 0.5);
}

void KeiserM3iDeviceSimulator::inner_reset(int buffSize) {
    if (dist_buff && this->buffSize != buffSize) {
        delete[] dist_buff;
        dist_buff = 0;
        delete[] dist_buff_time;
        dist_buff_time = 0;
    }
    this->buffSize = buffSize;
    if (!dist_buff) {
        this->dist_buff = new double[this->buffSize];
        this->dist_buff_time = new int [this->buffSize];
    }
    this->dist_buff_idx = 0;
    this->dist_buff_size = 0;
    this->dist_acc = 0.0;
    this->old_dist = -1.0;
    this->timeRms_acc = 0;
    this->old_timeRms = 0;
    this->time_o = 0;
    this->time_old = 0;
    this->calorie_o = 0;
    this->calorie_old = 0;
    this->distance_o = 0.0;
    this->distance_old = 0.0;
    this->nPulses = 0;
    this->sumWatt = 0;
    this->sumTime = 0;
    this->sumSpeed = 0.0;
    this->sumPulse = 0;
    this->sumRpm = 0;
    this->lastUpdatePostedTime = 0;
    this->equalTime = 0;
    this->old_time_orig = -1;
    this->nActiveUpdates = 0;
    this->lastUpdateTime = 0;
    this->oldPause = true;
}

double KeiserM3iDeviceSimulator::calcSpeed(keiser_m3i_out_t * f, bool pause) {
    double realdist = f->distance + this->distance_o;
    int realtime = f->time + this->time_o;
    if (this->old_dist < 0) {
        this->old_dist = realdist;
        this->old_timeRms = realtime;
        qDebug() << "Init: old_dist = " << realdist << " old_time = " << realtime;
        f->speed = 0.0;
        this->lastUpdatePostedTime = f->timeRAbsms;
    }
    else {
        int acc_time = realtime - this->old_timeRms;
        double acc = realdist - this->old_dist;
        if (!pause && (acc > 1e-6 || acc_time > 0)) {
            double rem = 0.0;
            int rem_time = 0;
            if (this->dist_buff_size == this->buffSize) {
                if (this->dist_buff_idx == this->buffSize)
                    this->dist_buff_idx = 0;
                rem = this->dist_buff[this->dist_buff_idx];
                this->dist_acc -= rem;
                rem_time = this->dist_buff_time[this->dist_buff_idx];
                this->timeRms_acc -= rem_time;
            }
            else
                this->dist_buff_size += 1;
            this->dist_buff_time[this->dist_buff_idx] = acc_time;
            this->dist_buff[this->dist_buff_idx] = acc;
            this->dist_buff_idx += 1;
            this->dist_acc += acc;
            this->timeRms_acc += acc_time;
            this->lastUpdatePostedTime = f->timeRAbsms;

            this->old_dist = realdist;
            this->old_timeRms = realtime;
            qDebug() << "D = (" << realdist << "," << acc << "->" << rem << "," << this->dist_acc << ") T = (" << realtime << "," << acc_time << "->" << rem_time << "," << this->timeRms_acc << ") => ";
        }
        else {
            if (f->timeRAbsms - this->lastUpdatePostedTime >= 1000)
                this->lastUpdatePostedTime = f->timeRAbsms;
            qDebug() << "P D = (" << realdist << ",- -> -," << this->dist_acc << ") T = (" << realtime << ",- -> -," << this->timeRms_acc << ") => ";
        }

        if (this->timeRms_acc == 0)
            f->speed = 0;
        else
            f->speed = this->dist_acc / (this->timeRms_acc / 3600.00);
        qDebug() << f->speed;
    }
    return f->speed;
}

bool KeiserM3iDeviceSimulator::inPause(qint64 ud) const {
    return this->equalTime >= M3I_EQUAL_TIME_THRESHOLD || ud >= M3I_PAUSE_DELAY_DETECT_THRESHOLD;
}

void KeiserM3iDeviceSimulator::detectPause(const keiser_m3i_out_t * f) {
    if (f->time == this->old_time_orig) {
        if (this->equalTime < M3I_EQUAL_TIME_THRESHOLD)
            this->equalTime += 1;
    }
    else {
        this->equalTime = 0;
        this->old_time_orig = f->time;
    }
}

bool KeiserM3iDeviceSimulator::inner_step(keiser_m3i_out_t * f) {
    qint64 nowms = QDateTime::currentMSecsSinceEpoch();
    if (this->old_time_orig > f->time) {
        qDebug() << "Setting offsets Km3i because " << this->old_time_orig << " > " << f->time;
        this->_set_offsets();
    }
    f->pulseMn = 0.0;
    f->rpmMn = 0.0;
    f->speedMn = 0.0;
    f->wattMn = 0.0;
    bool out = this->step_cyc(f, nowms);
    f->pulse /= 10;
    f->pulseMn /= 10.0;
    f->rpm /= 10;
    f->rpmMn /= 10.0;
    qDebug() << "Returning " << out;
    return out;
}

bool KeiserM3iDeviceSimulator::step_cyc(keiser_m3i_out_t * f, qint64 now) {
    this->detectPause(f);
    qint64 updateDiff = now - lastUpdateTime;
    bool nowpause = this->inPause(updateDiff);
    qDebug() << "ET=" << this->equalTime << " UD="<< updateDiff << " OP=" << oldPause << " NP=" << nowpause;
    if (!this->oldPause && !nowpause) {
        this->sumTime += updateDiff;
        this->fillTimeRFields(f, now);
        this->nActiveUpdates += 1;
        this->sumSpeed += this->calcSpeed(f, false);
        this->sumRpm += f->rpm;
        this->sumWatt += f->watt;
        if (f->pulse > M3I_VALID_PULSE_THRESHOLD) {
            this->sumPulse += f->pulse;
            this->nPulses += 1;
        }
    }
    else {
        this->fillTimeRFields(f, now);
        this->calcSpeed(f, true);
    }
    if (this->nPulses > 0)
        f->pulseMn = this->sumPulse / this->nPulses;
    if (this->nActiveUpdates > 0) {
        f->rpmMn = this->sumRpm / this->nActiveUpdates;
        f->wattMn = this->sumWatt / this->nActiveUpdates;
        f->speedMn = this->sumSpeed / this->nActiveUpdates;
        if (this->sumTime <= 0)
            f->distanceR = 0.0;
        else
            f->distanceR = f->speedMn * (this->sumTime / 3600000.0);
    }
    f->time += this->time_o;
    f->calorie += this->calorie_o;
    f->distance += this->distance_o;
    this->time_old = f->time;
    this->calorie_old = f->calorie;
    this->distance_old = f->distance;
    this->lastUpdateTime = now;
    this->oldPause = nowpause;
    return !nowpause;
}

m3ibike::m3ibike(bool noWriteResistance, bool noHeartService) {
    QSettings settings;
    antHeart = settings.value("ant_heart", false).toBool();
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    qt_search = settings.value("m3i_bike_qt_search", false).toBool();
#endif
    heartRateBeltDisabled = settings.value("heart_rate_belt_name", "Disabled").toString().startsWith("Disabled");
    m_watt.setType(metric::METRIC_WATT);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    detectDisc = new QTimer(this);
    m_instance = this;
    connect(detectDisc, &QTimer::timeout, this, [this]() {
        Q_UNUSED(this);
        emit disconnected();
        debug("M3i detected disconnection");
        initDone = false;
        detectDisc->stop();
        restartScan();
    });
}

m3ibike::~m3ibike() {
    if (detectDisc) {
        detectDisc->stop();
        delete detectDisc;
    }
    if(virtualBike)
        delete virtualBike;
    m_instance = 0;
    disconnecting = true;
#if defined(Q_OS_ANDROID)
    if (bluetoothScanner.isValid() && scannerActive)
        bluetoothScanner.callMethod<void>("stopScan", "(Landroid/bluetooth/le/ScanCallback;)V", scanCallback.object());
    scannerActive = false;
#endif
    if (discoveryAgent) {
        if (discoveryAgent->isActive()) {
            QObject::disconnect(discoveryAgent, SIGNAL(canceled()),
                        this, SLOT(discoveryFinishedPriv()));
            QObject::disconnect(discoveryAgent, SIGNAL(finished()),
                        this, SLOT(discoveryFinishedPriv()));
            discoveryAgent->stop();
        }
        delete discoveryAgent;
    }
#if defined(Q_OS_IOS)
#if !defined(IO_UNDER_QT)
    if (h)
        delete h;
#endif
    if (m3iIOS) {
        m3iIOS->stopScan();
        delete m3iIOS;
    }
#endif
}

void m3ibike::disconnectBluetooth() {
    if (detectDisc)
        detectDisc->stop();
    disconnecting = true;
#if defined(Q_OS_IOS)
    if (m3iIOS)
        m3iIOS->stopScan();
#endif
#if defined(Q_OS_ANDROID)
    if (bluetoothScanner.isValid() && scannerActive)
        bluetoothScanner.callMethod<void>("stopScan", "(Landroid/bluetooth/le/ScanCallback;)V", scanCallback.object());
    scannerActive = false;
#endif
    if (discoveryAgent) {
        discoveryAgent->stop();
    }
    emit disconnected();
    debug("M3i detected disconnection");
    initDone = false;
}

#if defined(Q_OS_ANDROID)
void m3ibike::newAndroidScanResult(JNIEnv * env, jobject /*thiz*/, jobject record) {
    if (m_instance) {
        ScanRecordResult srr(ScanRecordResult::fromJObject(env, record));
        qDebug() << "NEW ADV " << srr.toString();
        QByteArray data = srr.getData();
        QMetaObject::invokeMethod(m_instance, "processAdvertising", Qt::QueuedConnection,
                                  Q_ARG(QByteArray, data));
        //m_instance->processAdvertising(srr.getData());
    }
}

void m3ibike::newAndroidScanError(JNIEnv *, jobject /*thiz*/, jint code) {
    if (m_instance) {
        qDebug() << "SCAN ERROR " << code;
        //m_instance->restartScan();
        QMetaObject::invokeMethod(m_instance, "restartScan", Qt::QueuedConnection);
    }
}
#endif
void m3ibike::restartScan() {
    initScan();
#if defined(Q_OS_IOS)
    if (!qt_search) {
        if (m3iIOS->isScanning()) {
            qDebug() << "Stop scan Needed";
            m3iIOS->stopScan();
            qDebug() << "Stop Called";
        }
        m3iIOS->startScan(&m3i_ios_result);
        qDebug() << "Start called";
    }
    else
#endif
#if defined(Q_OS_ANDROID)
    if (!qt_search) {
        if (scannerActive) {
            qDebug() << "Stop scan Needed";
            bluetoothScanner.callMethod<void>("stopScan", "(Landroid/bluetooth/le/ScanCallback;)V", scanCallback.object());
            qDebug() << "Stop Called";
            scannerActive = false;
        }
        bluetoothScanner.callMethod<void>("startScan", "(Ljava/util/List;Landroid/bluetooth/le/ScanSettings;Landroid/bluetooth/le/ScanCallback;)V", listOfFilters.object(), settingsObject.object(), scanCallback.object());
        qDebug() << "Start called";
        scannerActive = true;
    }
    else
#endif
    {
        if (discoveryAgent->isActive())
            discoveryAgent->stop();
        else
            discoveryFinishedPriv();
    }

}
void m3ibike::initScan() {
#if defined(Q_OS_IOS)
    if (!qt_search) {
        if (!m3iIOS) {
            m3iIOS = new M3iIOS();
            m3iIOS->init(this);
        }
    }
    else
#endif
#if defined(Q_OS_ANDROID)
    if (!qt_search) {
        if (!bluetoothScanner.isValid()) {
            QAndroidJniObject bluetoothManager = QtAndroid::androidActivity().callObjectMethod(
                        "getSystemService",
                        "(Ljava/lang/String;)Ljava/lang/Object;",
                        QAndroidJniObject::fromString("bluetooth").object<jstring>());
            bluetoothAdapter =  bluetoothManager.callObjectMethod("getAdapter","()Landroid/bluetooth/BluetoothAdapter;");
            if (!bluetoothAdapter.isValid()) {
                debug("BluetoothAdapter Invalid!");
                return;
            }
            bluetoothScanner = bluetoothAdapter.callObjectMethod("getBluetoothLeScanner","()Landroid/bluetooth/le/BluetoothLeScanner;");
            if (!bluetoothScanner.isValid()) {
                debug("BluetoothScanner Invalid!");
                return;
            }
            JNINativeMethod methods[] {
                {"newScanResult", "(Lorg/cagnulen/qdomyoszwift/ScanRecordResult;)V", reinterpret_cast<void *>(m3ibike::newAndroidScanResult)},
                {"scanError", "(I)V", reinterpret_cast<void *>(m3ibike::newAndroidScanError)}};
            QAndroidJniObject javaClass("org/cagnulen/qdomyoszwift/NativeScanCallback");
            qDebug() << " nscc = " << javaClass.isValid();
            QAndroidJniEnvironment env;
            jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
            qDebug() << "oc = " << objectClass;
            jint res = env->RegisterNatives(objectClass,
                                 methods,
                                 sizeof(methods) / sizeof(methods[0]));
            qDebug() << "reg natives = "<<res;
            env->DeleteLocalRef(objectClass);
            qDebug() << "Del object class";
            listOfFilters = QAndroidJniObject("java/util/ArrayList");
            qDebug() << "lof "<<listOfFilters.isValid();
            QAndroidJniObject filterBuilder("android/bluetooth/le/ScanFilter$Builder");
            qDebug() << "fib "<<filterBuilder.isValid() << " add = "<< bluetoothDevice.address().toString();
            QAndroidJniObject nameString = QAndroidJniObject::fromString(bluetoothDevice.address().toString());
            QAndroidJniObject filterBuilder2 = filterBuilder.callObjectMethod("setDeviceAddress","(Ljava/lang/String;)Landroid/bluetooth/le/ScanFilter$Builder;", nameString.object<jstring>());
            qDebug() << "fib3 "<<filterBuilder2.isValid();
            filterObject0 = filterBuilder2.callObjectMethod("build","()Landroid/bluetooth/le/ScanFilter;");
            qDebug() << "fo = "  << filterObject0.isValid();
            jboolean added = listOfFilters.callMethod<jboolean>("add","(Ljava/lang/Object;)Z", filterObject0.object());
            qDebug() << "ad0 = "  << added << " sz = " << listOfFilters.callMethod<jint>("size");
            QAndroidJniObject settingsBuilder("android/bluetooth/le/ScanSettings$Builder");
            qDebug() << "sb0 = "  << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod("setScanMode","(I)Landroid/bluetooth/le/ScanSettings$Builder;", 2);//SCAN_MODE_LOW_LATENCY
            qDebug() << "sb1 = "  << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod("setMatchMode","(I)Landroid/bluetooth/le/ScanSettings$Builder;", 1);//MATCH_MODE_AGGRESSIVE
            qDebug() << "sb2 = "  << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod("setNumOfMatches","(I)Landroid/bluetooth/le/ScanSettings$Builder;", 3);//MATCH_NUM_MAX_ADVERTISEMENT
            qDebug() << "sb3 = "  << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod("setCallbackType","(I)Landroid/bluetooth/le/ScanSettings$Builder;", 1);//CALLBACK_TYPE_ALL_MATCHES
            qDebug() << "sb4 = "  << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod("setReportDelay","(J)Landroid/bluetooth/le/ScanSettings$Builder;", (jlong)0);//0ms
            qDebug() << "sb5 = "  << settingsBuilder.isValid();
            settingsObject = settingsBuilder.callObjectMethod("build","()Landroid/bluetooth/le/ScanSettings;");
            qDebug() << "so = "  << settingsObject.isValid();
            scanCallback = QAndroidJniObject("org/cagnulen/qdomyoszwift/NativeScanCallback");
            qDebug() << "sca = "  << scanCallback.isValid();
        }
    }
    else
#endif
    {
        if (!discoveryAgent) {
            discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
            connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
                    this, SLOT(deviceDiscoveredPriv(QBluetoothDeviceInfo)));
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            connect(discoveryAgent, SIGNAL(deviceUpdated(const QBluetoothDeviceInfo&, QBluetoothDeviceInfo::Fields)),
                    this, SLOT(deviceUpdatedPriv(const QBluetoothDeviceInfo&, QBluetoothDeviceInfo::Fields)));
        #endif
            connect(discoveryAgent, SIGNAL(canceled()),
                    this, SLOT(discoveryFinishedPriv()));
            connect(discoveryAgent, SIGNAL(finished()),
                    this, SLOT(discoveryFinishedPriv()));

            // Start a discovery
            discoveryAgent->setLowEnergyDiscoveryTimeout(600000);
        }
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
void m3ibike::deviceUpdatedPriv(const QBluetoothDeviceInfo &device, QBluetoothDeviceInfo::Fields updateFields)
{
    debug("deviceUpdated " + device.name() + " " + updateFields);
}
#endif

void m3ibike::deviceDiscoveredPriv(const QBluetoothDeviceInfo& device) {
    if (device.address() == bluetoothDevice.address()) {
        debug("NEW ADV " + bluetoothDevice.name());
        QHash<quint16, QByteArray> datas = device.manufacturerData();
        QHashIterator<quint16, QByteArray> i(datas);
        while (i.hasNext()) {
            i.next();
            processAdvertising(i.value());
            return;
        }
    }
}

void m3ibike::discoveryFinishedPriv() {
    if (!disconnecting && discoveryAgent)
        discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}


void m3ibike::searchingStop() {
    restartScan();
}

bool m3ibike::valid_id(int id) {
    return id >= 0 && id <= 255;
}

bool m3ibike::parse_data(const QByteArray& data, keiser_m3i_out_t * k3) {
    const uint8_t * arr = (const uint8_t*)data.constData();
    int len = data.size();
    if (len < 4 || len > 19)
        return false;
    int index = 0;
    if (arr[index] == 2 && arr[index + 1] == 1)
        index += 2;
    uint8_t mayor = arr[index];
    index += 1;
    uint8_t minor = arr[index];
    index += 1;
    if (mayor == 0x06 && len > index + 13) {
        uint8_t dt = arr[index];
        if (dt == 0 || dt >= 128 || dt <= 227) {
            k3->firmware = mayor;
            k3->software = minor;
            k3->system_id = arr[index + 1];
        }
        ;
        k3->rpm = arr[index + 2] | arr[index + 3] << 8;
        k3->pulse = arr[index + 4] | arr[index + 5] << 8;
        k3->watt = arr[index + 6] | arr[index + 7] << 8;
        k3->calorie = arr[index + 8] | arr[index + 9] << 8;
        k3->time = arr[index + 10] * 60;
        k3->time += arr[index + 11];
        uint16_t dist = arr[index + 12] | arr[index + 13] << 8;
        if (dist & 32768)
            k3->distance = (dist & 0x7FFF) / 10.0;
        else
            k3->distance = dist / 10.0 * 1.60934;
        if (minor >= 0x21 && len > (index + 14))
            k3->incline = arr[index + 14];
        else
            k3->incline = 0;
        return true;
    }
    else
        return false;
}

bool m3ibike::isCorrectUnit(const QBluetoothDeviceInfo &device) {
    if (device.name().startsWith("M3")) {
        QSettings settings;
        keiser_m3i_out_t k3;
        int id = settings.value("m3i_bike_id", 256).toInt();
        QHash<quint16, QByteArray> datas = device.manufacturerData();
        QHashIterator<quint16, QByteArray> i(datas);
        while (i.hasNext()) {
            i.next();
            if (parse_data(i.value(), &k3) && (!valid_id(id) || k3.system_id == id)) {
                return true;
            }
        }
    }
    return false;
}

void m3ibike::processAdvertising(const QByteArray& data) {
    if (disconnecting)
        return;
    debug(" << " + data.toHex(' '));
    if (parse_data(data, &k3)) {
#if defined(Q_OS_IOS) //IOS devices cannot be distinguished by bluetooth address (always 0)
        if (qt_search && (m3i_ios_result.major != k3.firmware || m3i_ios_result.minor != k3.software || m3i_ios_result.idval != k3.system_id))
            return;
#endif
        detectDisc->start(6000);
        if (!initDone) {
            initDone = true;
            if (!virtualBike
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
                && !h
#endif
                ) {
                QSettings settings;
                bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
                h = new lockscreen();
                bool cadence = settings.value("bike_cadence_sensor", false).toBool();
                bool ios_peloton_workaround = settings.value("ios_peloton_workaround", false).toBool();
                if (ios_peloton_workaround && cadence) {
                    qDebug() << "ios_peloton_workaround activated!";
                    h->virtualbike_ios();
                }
                else
#endif
                if (virtual_device_enabled) {
                    debug("creating virtual bike interface...");
                    virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                    connect(virtualBike, &virtualbike::debug, this, &m3ibike::debug);
                }
                int buffSize = settings.value("m3i_bike_speed_buffsize", 90).toInt();
                k3s.inner_reset(buffSize);
            }
            emit connectedAndDiscovered();
            emit bikeStarted();
            debug("M3i (re)connected");
        }
        k3s.inner_step(&k3);
        double angular_coeff;

        if ((int)(Resistance.value() + 0.5) != k3.incline) {
            Resistance = k3.incline;
            if (!k3.incline)
                m_pelotonResistance = 0.0;
            RESISTANCE_OP(SET_PELOTON_RESISTANCE,k3.incline, m_pelotonResistance)
        }
        else
        {
            Resistance = Resistance.value();
            m_pelotonResistance = m_pelotonResistance.value();
        }

        Cadence = k3.rpm;
        m_watt = k3.watt;
        watts(); // to update avg and max
        Speed = k3.speed;
        KCal = k3.calorie;
        Distance = k3.distance;
        m_jouls += (m_watt.value() * (k3.time - elapsed.value()));
        elapsed = k3.time;

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

#ifdef Q_OS_ANDROID
        if (antHeart)
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (heartRateBeltDisabled) {
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
                long appleWatchHeartRate = h->heartRate();
                if(appleWatchHeartRate == 0)
                    Heart = k3.pulse;
                else
                    Heart = appleWatchHeartRate;
                debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));                                    
#else
                Heart = k3.pulse;
#endif
            }
        }

#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
        QSettings settings;
        bool cadence = settings.value("bike_cadence_sensor", false).toBool();
        bool ios_peloton_workaround = settings.value("ios_peloton_workaround", false).toBool();
        if (ios_peloton_workaround && cadence && h && firstStateChanged) {
            h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualbike_setHeartRate((uint8_t)currentHeart().value());
        }
#endif

        debug("Current Elapsed: " + QString::number(elapsed.value()));
        debug("Current Resistance: " + QString::number(Resistance.value()));
        debug("Current Speed: " + QString::number(Speed.value()));
        debug("Current Calculate Distance: " + QString::number(k3.distanceR));
        debug("Current Cadence: " + QString::number(Cadence.value()));
        debug("Current Distance: " + QString::number(Distance.value()));
        debug("Current CrankRevs: " + QString::number(CrankRevs));
        debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));
        debug("Current Watt: " + QString::number(watts()));
        debug("Current Heart: " + QString::number(Heart.value()));
    }
}

void m3ibike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    initDone = false;
    disconnecting = false;
#if defined(Q_OS_IOS)
    QHash<quint16, QByteArray> datas = device.manufacturerData();
    QHashIterator<quint16, QByteArray> i(datas);
    while (i.hasNext()) {
        i.next();
        if (parse_data(i.value(), &k3)) {
            m3i_ios_result.major = k3.firmware;
            m3i_ios_result.minor = k3.software;
            m3i_ios_result.idval = k3.system_id;
            break;
        }
    }
#endif
}

bool m3ibike::connected() {
    return initDone;
}

void* m3ibike::VirtualBike() {
    return virtualBike;
}

void* m3ibike::VirtualDevice() {
    return VirtualBike();
}

uint16_t m3ibike::watts() {
    if (currentCadence().value() == 0) return 0;

    return m_watt.value();
}
