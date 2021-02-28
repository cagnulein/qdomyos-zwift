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

bool KeiserM3iDeviceSimulator::inPause() const {
    return this->equalTime >= M3I_EQUAL_TIME_THRESHOLD || QDateTime::currentMSecsSinceEpoch() - this->lastUpdateTime >= M3I_PAUSE_DELAY_DETECT_THRESHOLD;
}

void KeiserM3iDeviceSimulator::detectPause(const keiser_m3i_out_t * f) {
    if (f->time == this->old_time_orig) {
        if (this->equalTime < M3I_EQUAL_TIME_THRESHOLD)
            this->equalTime += 1;
        qDebug() << "EqualTime " << this->equalTime;
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
    bool wasinpause = this->inPause();
    this->detectPause(f);
    if (!wasinpause && !this->inPause()) {
        this->sumTime += (now - this->lastUpdateTime);
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
    return !this->inPause();
}


m3ibike::m3ibike(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    detectDisc = new QTimer(this);
    connect(detectDisc, &QTimer::timeout, this, [this]() {
        Q_UNUSED(this);
        emit disconnected();
        initDone = false;
        detectDisc->stop();
    });
    detectDisc->start(2000);
}

m3ibike::~m3ibike() {
    if (detectDisc) {
        detectDisc->stop();
        delete detectDisc;
    }
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
    if (h)
        delete h;
#endif
}

bool m3ibike::valid_id(int id) {
    return id >= 0 || id <= 255;
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

void m3ibike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    debug("m3ibike new device: " + device.name() + " (" + device.address().toString() + ')');
    if (device.name().startsWith("M3")) {
        bluetoothDevice = device;
        QSettings settings;
        int id = settings.value("m3i_bike_id", 256).toInt();
        int buffSize = settings.value("m3i_bike_speed_buffsize", 150).toInt();
        QHash<quint16, QByteArray> datas = device.manufacturerData();
        QHashIterator<quint16, QByteArray> i(datas);
        while (i.hasNext()) {
            i.next();
            debug(" << " + i.value().toHex(' '));
            if (parse_data(i.value(), &k3) && k3.system_id == id) {
                if (!initDone) {
                    k3s.inner_reset(buffSize);
                    initDone = true;
                    if (!virtualBike
        #if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
                        && !h
        #endif
                        ) {
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
                    }
                    emit connectedAndDiscovered();
                }
                k3s.inner_step(&k3);
                QSettings settings;
                QString heartRateBeltName = settings.value("heart_rate_belt_name", "Disabled").toString();


                Resistance = k3.incline;
                Cadence = k3.rpm;
                m_watts = k3.watt;
                Speed = k3.speed;
                KCal = k3.calorie;
                Distance = k3.distance;
                elapsed = k3.time;

                if (Cadence.value() > 0) {
                    CrankRevs++;
                    LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
                }

#ifdef Q_OS_ANDROID
                if (settings.value("ant_heart", false).toBool())
                    Heart = (uint8_t)KeepAwakeHelper::heart();
                else
#endif
                {
                    if (heartRateBeltName.startsWith("Disabled")) {
#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
                        long appleWatchHeartRate = h->heartRate();
                        Heart = appleWatchHeartRate;
                        debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
#else
                        Heart = k3.pulse;
#endif
                    }
                }

#if defined(Q_OS_IOS) && !defined(IO_UNDER_QT)
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
                debug("Current Calculate Distance: " + QString::number(Distance.value()));
                debug("Current Cadence: " + QString::number(Cadence.value()));
                debug("Current Distance: " + QString::number(Distance.value()));
                debug("Current CrankRevs: " + QString::number(CrankRevs));
                debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));
                debug("Current Watt: " + QString::number(watts()));
                debug("Current Heart: " + QString::number(Heart.value()));
                return;
            }
        }
    }
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

    return m_watts;
}
