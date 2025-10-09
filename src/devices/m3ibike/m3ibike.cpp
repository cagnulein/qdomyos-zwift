#include "m3ibike.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;
#if defined(Q_OS_ANDROID)
#include "scanrecordresult.h"
#include <QJniEnvironment>
#include <QJniObject>
#include <QMetaObject>
#include <QCoreApplication>
#endif

static m3ibike *m_instance = 0;

#define RESISTANCE_OP(OP, V, D)                                                                                        \
    OP(1, 4, 10, V, D)                                                                                                 \
    OP(5, 9, 20, V, D)                                                                                                 \
    OP(10, 11, 30, V, D)                                                                                               \
    OP(12, 14, 40, V, D)                                                                                               \
    OP(15, 16, 50, V, D)                                                                                               \
    OP(17, 18, 60, V, D)                                                                                               \
    OP(19, 20, 70, V, D)                                                                                               \
    OP(21, 22, 80, V, D)                                                                                               \
    OP(23, 24, 90, V, D)

#define SET_PELOTON_RESISTANCE(MINV, MAXV, ZWIFTV, KEISERV, DEST)                                                      \
    else if (KEISERV >= MINV && KEISERV <= MAXV) {                                                                     \
        angular_coeff = 10.0 / (MAXV + 1 - MINV);                                                                      \
        DEST = angular_coeff * KEISERV + ZWIFTV - angular_coeff * MINV;                                                \
    }

KeiserM3iDeviceSimulator::~KeiserM3iDeviceSimulator() {
    if (dist_buff) {
        delete[] dist_buff;
    }
    if (dist_buff_time) {
        delete[] dist_buff_time;
    }
}
KeiserM3iDeviceSimulator::KeiserM3iDeviceSimulator() {}

void KeiserM3iDeviceSimulator::_set_offsets() {
    this->time_o = this->time_old;
    this->calorie_o = this->calorie_old;
    this->distance_o = this->distance_old;
}

void KeiserM3iDeviceSimulator::fillTimeRFields(keiser_m3i_out_t *f, qint64 updateTime) {
    if (!sessionStart)
        sessionStart = updateTime;
    f->timeRms = this->sumTime;
    f->timeRAbsms = (int)(updateTime - sessionStart);
    f->timeR = (int)(this->sumTime / 1000.0 + 0.5);
}

void KeiserM3iDeviceSimulator::inner_reset(int buffSize, int equalTimeDist) {
    if (dist_buff && this->buffSize != buffSize) {
        delete[] dist_buff;
        dist_buff = 0;
        delete[] dist_buff_time;
        dist_buff_time = 0;
    }
    this->buffSize = buffSize;
    if (!dist_buff) {
        this->dist_buff = new double[this->buffSize];
        this->dist_buff_time = new int[this->buffSize];
    }
    this->equalTimeDistanceThreshold = equalTimeDist;
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
    this->equalTimeDistance = 0;
}

double KeiserM3iDeviceSimulator::calcSpeed(keiser_m3i_out_t *f, bool pause) {
    double realdist = f->distance + this->distance_o;
    int realtime = f->time_orig + this->time_o;
    if (this->old_dist < 0) {
        this->old_dist = realdist;
        this->old_timeRms = realtime;
        qDebug() << QStringLiteral("Init: old_dist = ") << realdist << QStringLiteral(" old_time = ") << realtime;
        f->speed = 0.0;
        this->lastUpdatePostedTime = f->timeRAbsms;
    } else {
        int acc_time = realtime - this->old_timeRms;
        double acc = realdist - this->old_dist;
        if (!pause && acc > 0 && acc_time >= 0 && (acc > 1e-6 || acc_time > 0)) {
            double rem = 0.0;
            int rem_time = 0;
            if (this->dist_buff_size == this->buffSize) {
                if (this->dist_buff_idx == this->buffSize) {
                    this->dist_buff_idx = 0;
                }
                rem = this->dist_buff[this->dist_buff_idx];
                this->dist_acc -= rem;
                rem_time = this->dist_buff_time[this->dist_buff_idx];
                this->timeRms_acc -= rem_time;
            } else {
                this->dist_buff_size += 1;
            }
            this->dist_buff_time[this->dist_buff_idx] = acc_time;
            this->dist_buff[this->dist_buff_idx] = acc;
            this->dist_buff_idx += 1;
            this->dist_acc += acc;
            this->timeRms_acc += acc_time;
            this->lastUpdatePostedTime = f->timeRAbsms;

            this->old_dist = realdist;
            this->old_timeRms = realtime;
            qDebug() << QStringLiteral("D = (") << realdist << QStringLiteral(",") << acc << QStringLiteral("->") << rem
                     << QStringLiteral(",") << this->dist_acc << QStringLiteral(") T = (") << realtime
                     << QStringLiteral(",") << acc_time << QStringLiteral("->") << rem_time << QStringLiteral(",")
                     << this->timeRms_acc << QStringLiteral(") => ");
        } else {
            if (f->timeRAbsms - this->lastUpdatePostedTime >= 1000) {
                this->lastUpdatePostedTime = f->timeRAbsms;
            }
            qDebug() << QStringLiteral("P D = (") << realdist << QStringLiteral(",- -> -,") << this->dist_acc
                     << QStringLiteral(") T = (") << realtime << QStringLiteral(",- -> -,") << this->timeRms_acc
                     << QStringLiteral(") => ");
        }

        if (this->timeRms_acc == 0) {
            f->speed = 0;
        } else {
            f->speed = this->dist_acc / (this->timeRms_acc / 3600.00);
        }
        qDebug() << f->speed;
    }
    return f->speed;
}

bool KeiserM3iDeviceSimulator::inPause(qint64 ud) const {
    return this->equalTime >= M3I_EQUAL_TIME_THRESHOLD || ud >= M3I_PAUSE_DELAY_DETECT_THRESHOLD;
}

void KeiserM3iDeviceSimulator::detectPause(const keiser_m3i_out_t *f, qint64 ud) {
    if (f->time_orig == this->old_time_orig) {
        this->equalTimeDistance += ud;
        if (this->equalTimeDistance >= equalTimeDistanceThreshold) {
            this->equalTime = M3I_EQUAL_TIME_THRESHOLD;
        } else if (this->equalTime < M3I_EQUAL_TIME_THRESHOLD) {
            this->equalTime += 1;
        }
    } else {
        this->equalTime = 0;
        this->equalTimeDistance = 0;
        this->old_time_orig = f->time_orig;
    }
}

bool KeiserM3iDeviceSimulator::inner_step(keiser_m3i_out_t *f) {
    qint64 nowms = QDateTime::currentMSecsSinceEpoch();
    if (this->old_time_orig > f->time_orig) {
        qDebug() << QStringLiteral("Setting offsets Km3i because ") << this->old_time_orig << QStringLiteral(" > ")
                 << f->time_orig;
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
    qDebug() << QStringLiteral("Returning ") << out;
    return out;
}

bool KeiserM3iDeviceSimulator::step_cyc(keiser_m3i_out_t *f, qint64 now) {
    qint64 updateDiff = now - lastUpdateTime;
    this->detectPause(f, updateDiff);
    bool nowpause = this->inPause(updateDiff);
    qDebug() << QStringLiteral("ET=") << this->equalTime << QStringLiteral("ETD=") << this->equalTimeDistance
             << QStringLiteral(" UD=") << updateDiff << QStringLiteral(" OP=") << oldPause << QStringLiteral(" NP=")
             << nowpause;
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
    } else {
        this->fillTimeRFields(f, now);
        this->calcSpeed(f, true);
    }
    if (this->nPulses > 0) {
        f->pulseMn = this->sumPulse / this->nPulses;
    }
    if (this->nActiveUpdates > 0) {
        f->rpmMn = this->sumRpm / this->nActiveUpdates;
        f->wattMn = this->sumWatt / this->nActiveUpdates;
        f->speedMn = this->sumSpeed / this->nActiveUpdates;
        if (this->sumTime <= 0) {
            f->distanceR = 0.0;
        } else {
            f->distanceR = f->speedMn * (this->sumTime / 3600000.0);
        }
    }
    f->time = f->time_orig + this->time_o;
    f->calorie += this->calorie_o;
    f->distance += this->distance_o;
    if (nowpause) {
        f->speed = 0.0;
        f->watt = 0;
        f->rpm = 0;
    }
    this->time_old = f->time;
    this->calorie_old = f->calorie;
    this->distance_old = f->distance;
    this->lastUpdateTime = now;
    this->oldPause = nowpause;
    return !nowpause;
}

m3ibike::m3ibike(bool noWriteResistance, bool noHeartService) {
    QSettings settings;
    antHeart = settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool();
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    qt_search =
        (QT_VERSION < QT_VERSION_CHECK(5, 12, 0)) ? false : settings.value(QZSettings::m3i_bike_qt_search, QZSettings::default_m3i_bike_qt_search).toBool();
#endif
    heartRateBeltDisabled = settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name)
                                .toString()
                                .startsWith(QStringLiteral("Disabled"));
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    detectDisc = new QTimer(this);
    elapsedTimer = new QTimer(this);
    elapsedTimer->setSingleShot(false);
    lastTimerRestart = -1;
    m_instance = this;
    connect(detectDisc, &QTimer::timeout, this, [this]() {
        Q_UNUSED(this);
        emit disconnected();
        emit debug(QStringLiteral("M3i detected disconnection"));
        initDone = false;
        detectDisc->stop();
        elapsedTimer->stop();
        lastTimerRestart = -1;
        elapsed = lastTimerRestartOffset = k3.time;
        moving = elapsed;
        restartScan();
    });
    connect(elapsedTimer, &QTimer::timeout, this, [this]() {
        Q_UNUSED(this);
        if (lastTimerRestart > 0) {
            elapsed = lastTimerRestartOffset + (QDateTime::currentMSecsSinceEpoch() - lastTimerRestart) / 1000.0;
            moving = elapsed;
        }
    });
}

m3ibike::~m3ibike() {
    if (detectDisc) {
        detectDisc->stop();
        delete detectDisc;
    }
    if (elapsedTimer) {
        elapsedTimer->stop();
        delete elapsedTimer;
    }
    m_instance = 0;
    disconnecting = true;
#if defined(Q_OS_ANDROID)
    if (bluetoothScanner.isValid() && scannerActive)
        bluetoothScanner.callMethod<void>("stopScan", "(Landroid/bluetooth/le/ScanCallback;)V", scanCallback.object());
    scannerActive = false;
#endif
    if (discoveryAgent) {
        if (discoveryAgent->isActive()) {
            QObject::disconnect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this,
                                &m3ibike::discoveryFinishedPriv);
            QObject::disconnect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this,
                                &m3ibike::discoveryFinishedPriv);
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
    if (detectDisc) {
        detectDisc->stop();
    }
    if (elapsedTimer) {
        elapsedTimer->stop();
    }
    disconnecting = true;
    lastTimerRestart = -1;
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
    emit debug(QStringLiteral("M3i detected disconnection"));
    initDone = false;
}

#if defined(Q_OS_ANDROID)
void m3ibike::newAndroidScanResult(JNIEnv *env, jobject /*thiz*/, jobject record) {
    if (m_instance) {
        ScanRecordResult srr(ScanRecordResult::fromJObject(env, record));
        qDebug() << "NEW ADV " << srr.toString();
        QByteArray data = srr.getData();
        QMetaObject::invokeMethod(m_instance, "processAdvertising", Qt::QueuedConnection, Q_ARG(QByteArray, data));
        // m_instance->processAdvertising(srr.getData());
    }
}

void m3ibike::newAndroidScanError(JNIEnv *, jobject /*thiz*/, jint code) {
    if (m_instance) {
        qDebug() << "SCAN ERROR " << code;
        // m_instance->restartScan();
        QMetaObject::invokeMethod(m_instance, "restartScan", Qt::QueuedConnection);
    }
}
#endif
void m3ibike::restartScan() {
    initScan();
#if defined(Q_OS_IOS)
    if (!qt_search) {
        qDebug() << "Starting scan";
        if (m3iIOS->isScanning()) {
            qDebug() << "Stop scan Needed";
            m3iIOS->stopScan();
            qDebug() << "Stop Called";
        }
        QString uuid = bluetoothDevice.deviceUuid().toString();
        strncpy(m3i_ios_result.uuid, uuid.mid(1, uuid.length() - 2).toUpper().toUtf8().constData(),
                sizeof(m3i_ios_result.uuid) / sizeof(*m3i_ios_result.uuid) - 1);
        m3iIOS->startScan(&m3i_ios_result);
        qDebug() << "Start called";
    } else
#endif
#if defined(Q_OS_ANDROID)
        if (!qt_search) {
        if (scannerActive) {
            qDebug() << "Stop scan Needed";
            bluetoothScanner.callMethod<void>("stopScan", "(Landroid/bluetooth/le/ScanCallback;)V",
                                              scanCallback.object());
            qDebug() << "Stop Called";
            scannerActive = false;
        }
        bluetoothScanner.callMethod<void>(
            "startScan", "(Ljava/util/List;Landroid/bluetooth/le/ScanSettings;Landroid/bluetooth/le/ScanCallback;)V",
            listOfFilters.object(), settingsObject.object(), scanCallback.object());
        qDebug() << "Start called";
        scannerActive = true;
    } else
#endif
    {
        if (discoveryAgent->isActive()) {
            discoveryAgent->stop();
        } else {
            discoveryFinishedPriv();
        }
    }
}
void m3ibike::initScan() {
#if defined(Q_OS_IOS)
    if (!qt_search) {
        if (!m3iIOS) {
            debug("New m3ios");
            m3iIOS = new M3iIOS();
            debug("init m3ios");
            m3iIOS->init(this);
            debug("init m3ios done");
        }
    } else
#endif
#if defined(Q_OS_ANDROID)
        if (!qt_search) {
        if (!bluetoothScanner.isValid()) {
            QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
            QJniObject bluetoothManager = activity.callObjectMethod(
                "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;",
                QJniObject::fromString("bluetooth").object<jstring>());
            bluetoothAdapter =
                bluetoothManager.callObjectMethod("getAdapter", "()Landroid/bluetooth/BluetoothAdapter;");
            if (!bluetoothAdapter.isValid()) {
                debug("BluetoothAdapter Invalid!");
                return;
            }
            bluetoothScanner = bluetoothAdapter.callObjectMethod("getBluetoothLeScanner",
                                                                 "()Landroid/bluetooth/le/BluetoothLeScanner;");
            if (!bluetoothScanner.isValid()) {
                debug("BluetoothScanner Invalid!");
                return;
            }
            JNINativeMethod methods[]{{"newScanResult", "(Lorg/cagnulen/qdomyoszwift/ScanRecordResult;)V",
                                       reinterpret_cast<void *>(m3ibike::newAndroidScanResult)},
                                      {"scanError", "(I)V", reinterpret_cast<void *>(m3ibike::newAndroidScanError)}};
            QJniObject javaClass("org/cagnulen/qdomyoszwift/NativeScanCallback");
            qDebug() << " nscc = " << javaClass.isValid();
            QJniEnvironment env;
            jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
            qDebug() << "oc = " << objectClass;
            jint res = env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0]));
            qDebug() << "reg natives = " << res;
            env->DeleteLocalRef(objectClass);
            qDebug() << "Del object class";
            listOfFilters = QJniObject("java/util/ArrayList");
            qDebug() << "lof " << listOfFilters.isValid();
            QJniObject filterBuilder("android/bluetooth/le/ScanFilter$Builder");
            qDebug() << "fib " << filterBuilder.isValid() << " add = " << bluetoothDevice.address().toString();
            QJniObject nameString = QJniObject::fromString(bluetoothDevice.address().toString());
            QJniObject filterBuilder2 = filterBuilder.callObjectMethod(
                "setDeviceAddress", "(Ljava/lang/String;)Landroid/bluetooth/le/ScanFilter$Builder;",
                nameString.object<jstring>());
            qDebug() << "fib3 " << filterBuilder2.isValid();
            filterObject0 = filterBuilder2.callObjectMethod("build", "()Landroid/bluetooth/le/ScanFilter;");
            qDebug() << "fo = " << filterObject0.isValid();
            jboolean added = listOfFilters.callMethod<jboolean>("add", "(Ljava/lang/Object;)Z", filterObject0.object());
            qDebug() << "ad0 = " << added << " sz = " << listOfFilters.callMethod<jint>("size");
            QJniObject settingsBuilder("android/bluetooth/le/ScanSettings$Builder");
            qDebug() << "sb0 = " << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod(
                "setScanMode", "(I)Landroid/bluetooth/le/ScanSettings$Builder;", 2); // SCAN_MODE_LOW_LATENCY
            qDebug() << "sb1 = " << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod(
                "setMatchMode", "(I)Landroid/bluetooth/le/ScanSettings$Builder;", 1); // MATCH_MODE_AGGRESSIVE
            qDebug() << "sb2 = " << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod(
                "setNumOfMatches", "(I)Landroid/bluetooth/le/ScanSettings$Builder;", 3); // MATCH_NUM_MAX_ADVERTISEMENT
            qDebug() << "sb3 = " << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod(
                "setCallbackType", "(I)Landroid/bluetooth/le/ScanSettings$Builder;", 1); // CALLBACK_TYPE_ALL_MATCHES
            qDebug() << "sb4 = " << settingsBuilder.isValid();
            settingsBuilder = settingsBuilder.callObjectMethod(
                "setReportDelay", "(J)Landroid/bluetooth/le/ScanSettings$Builder;", (jlong)0); // 0ms
            qDebug() << "sb5 = " << settingsBuilder.isValid();
            settingsObject = settingsBuilder.callObjectMethod("build", "()Landroid/bluetooth/le/ScanSettings;");
            qDebug() << "so = " << settingsObject.isValid();
            scanCallback = QJniObject("org/cagnulen/qdomyoszwift/NativeScanCallback");
            qDebug() << "sca = " << scanCallback.isValid();
        }
    } else
#endif
    {
        if (!discoveryAgent) {
            discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
            connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this,
                    &m3ibike::deviceDiscoveredPriv);
            connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this, &m3ibike::deviceUpdatedPriv);
#endif
            connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &m3ibike::discoveryFinishedPriv);
            connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &m3ibike::discoveryFinishedPriv);

            // Start a discovery
            discoveryAgent->setLowEnergyDiscoveryTimeout(600000);
        }
    }
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
void m3ibike::deviceUpdatedPriv(const QBluetoothDeviceInfo &device, QBluetoothDeviceInfo::Fields updateFields) {
    qDebug() << QStringLiteral("deviceUpdated ") << device.name() << " " << updateFields;
}

void m3ibike::deviceDiscoveredPriv(const QBluetoothDeviceInfo &device) {
    if (SAME_BLUETOOTH_DEVICE(device, bluetoothDevice)) {
        emit debug(QStringLiteral("NEW ADV ") + bluetoothDevice.name());
        QMultiHash<quint16, QByteArray> datas = device.manufacturerData();
        QHash<quint16, QByteArray> uniqueDatas;
        for (auto it = datas.begin(); it != datas.end(); ++it) {
            if (!uniqueDatas.contains(it.key())) {
                uniqueDatas[it.key()] = it.value();
            }
        }
        QHashIterator<quint16, QByteArray> i(uniqueDatas);
        while (i.hasNext()) {
            i.next();
            processAdvertising(i.value());
            return;
        }
    }
}
#endif

void m3ibike::discoveryFinishedPriv() {
    if (!disconnecting && discoveryAgent) {
        discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    }
}

void m3ibike::searchingStop() { restartScan(); }

bool m3ibike::valid_id(int id) { return id >= 0 && id <= 255; }

bool m3ibike::parse_data(const QByteArray &data, keiser_m3i_out_t *k3) {
    const uint8_t *arr = (const uint8_t *)data.constData();
    int len = data.size();
    if (len < 4 || len > 19) {
        return false;
    }
    int index = 0;
    if (arr[index] == 2 && arr[index + 1] == 1) {
        index += 2;
    }
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
        k3->rpm = arr[index + 2] | arr[index + 3] << 8;
        k3->pulse = arr[index + 4] | arr[index + 5] << 8;
        k3->watt = arr[index + 6] | arr[index + 7] << 8;
        k3->calorie = arr[index + 8] | arr[index + 9] << 8;
        k3->time_orig = arr[index + 10] * 60;
        k3->time_orig += arr[index + 11];
        uint16_t dist = arr[index + 12] | arr[index + 13] << 8;
        if (dist & 32768) {
            k3->distance = (dist & 0x7FFF) / 10.0;
        } else {
            k3->distance = dist / 10.0 * 1.60934;
        }
        if (minor >= 0x21 && len > (index + 14)) {
            k3->incline = arr[index + 14];
        } else {
            k3->incline = 0;
        }
        return true;
    } else
        return false;
}

bool m3ibike::isCorrectUnit(const QBluetoothDeviceInfo &device) {
    if (device.name().startsWith(QStringLiteral("M3"))) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
        QSettings settings;
        keiser_m3i_out_t k3;
        int id = settings.value(QZSettings::m3i_bike_id, QZSettings::default_m3i_bike_id).toInt();
        QMultiHash<quint16, QByteArray> datas = device.manufacturerData();
        QHash<quint16, QByteArray> uniqueDatas;
        for (auto it = datas.begin(); it != datas.end(); ++it) {
            if (!uniqueDatas.contains(it.key())) {
                uniqueDatas[it.key()] = it.value();
                if (parse_data(it.value(), &k3) && (!valid_id(id) || k3.system_id == id)) {
                    return true;
                }
            }
        }
#else
        return true;
#endif
    }
    return false;
}

void m3ibike::processAdvertising(const QByteArray &data) {
    if (disconnecting) {
        return;
    }
    emit debug(QStringLiteral(" << ") + data.toHex(' '));
    if (parse_data(data, &k3)) {
        QSettings settings;
        detectDisc->start(M3i_DISCONNECT_THRESHOLD);
        if (!initDone) {
            initDone = true;
            if (!this->hasVirtualDevice()
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
                && !h
#endif
                ) {
                bool virtual_device_enabled = settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
                h = new lockscreen();
                bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
                bool ios_peloton_workaround = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
                if (ios_peloton_workaround && cadence) {
                    qDebug() << "ios_peloton_workaround activated!";
                    h->virtualbike_ios();
                } else
#endif
                    if (virtual_device_enabled) {
                    emit debug(QStringLiteral("creating virtual bike interface..."));
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                    // connect(virtualBike, &virtualbike::debug, this, &m3ibike::debug);
                    connect(virtualBike, &virtualbike::changeInclination, this, &m3ibike::changeInclination);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                }
                int buffSize = settings.value(QZSettings::m3i_bike_speed_buffsize, QZSettings::default_m3i_bike_speed_buffsize).toInt();
                k3s.inner_reset(buffSize,
#if defined(Q_OS_ANDROID)
                                qt_search ? 2500 : 5000
#else
                                2500
#endif
                                );
                m_jouls = 0;
            }
            emit connectedAndDiscovered();
            emit bikeStarted();
            emit debug(QStringLiteral("M3i (re)connected"));
        }
        int oldtime = k3.time;
        bool not_in_pause = k3s.inner_step(&k3);
        double angular_coeff;

        if ((int)(Resistance.value() + 0.5) != k3.incline) {
            Resistance = k3.incline;
            if (!k3.incline)
                m_pelotonResistance = 0.0;
            RESISTANCE_OP(SET_PELOTON_RESISTANCE, k3.incline, m_pelotonResistance)
        } else {
            Resistance = Resistance.value();
            m_pelotonResistance = m_pelotonResistance.value();
        }
        emit resistanceRead(Resistance.value());

        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            Cadence = k3.rpm;
        }
        if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")))
            m_watt = k3.watt;
        watts(); // to update avg and max
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            Speed = k3.speed;
        } else {
            Speed = metric::calculateSpeedFromPower(watts(),  Inclination.value(), Speed.value(),fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        }
        if (settings.value(QZSettings::m3i_bike_kcal, QZSettings::default_m3i_bike_kcal).toBool()) {
            KCal = k3.calorie;
        } else {
            if (watts())
                KCal += ((((0.048 * ((double)watts()) + 1.19) *
                           settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                          200.0) /
                         (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
        }
        Distance = k3.distance;
        if (!not_in_pause || k3.time_orig <= 10) {
            lastTimerRestart = -1;
            elapsedTimer->stop();
            elapsed = lastTimerRestartOffset = k3.time;
            moving = elapsed;
        } else if (lastTimerRestart < 0) {
            elapsed = lastTimerRestartOffset = k3.time;
            moving = elapsed;
            lastTimerRestart = QDateTime::currentMSecsSinceEpoch();
            elapsedTimer->start(1s);
        }
        m_jouls += (m_watt.value() * (k3.time - oldtime));
        WeightLoss = metric::calculateWeightLoss(KCal.value());
        WattKg = m_watt.value() / settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
        if (antHeart)
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (heartRateBeltDisabled && (k3.pulse == 0 || settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool())) {
                update_hr_from_external();
            } else if(!heartRateBeltDisabled) {
                // we don't have to anything in this case
            } else {
                Heart = k3.pulse;
            }
        }

#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
        bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence && h) {
            h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
        }
#endif

        emit debug(QStringLiteral("Current Elapsed: ") + QString::number(elapsed.value()));
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(k3.distanceR));
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
        emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
        emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
    }
}

void m3ibike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    initDone = false;
    disconnecting = false;
}

bool m3ibike::connected() { return initDone; }

uint16_t m3ibike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}
