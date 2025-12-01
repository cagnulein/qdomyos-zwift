#include "trainprogram.h"
#include "zwiftworkout.h"
#include "homeform.h"
#include <QFile>
#include <QMutexLocker>
#include <QtXml/QtXml>
#include <algorithm>
#include <chrono>
#ifdef Q_OS_ANDROID
#include "androidactivityresultreceiver.h"
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#elif defined(Q_OS_WINDOWS)
#include "windows_zwift_incline_paddleocr_thread.h"
#include "windows_zwift_workout_paddleocr_thread.h"
#endif
#ifdef Q_CC_MSVC
#include "zwift-api/zwift_messages.pb.h"
#endif
#include "localipaddress.h"

using namespace std::chrono_literals;

trainprogram::trainprogram(const QList<trainrow> &rows, bluetooth *b, QString *description, QString *tags,
                           bool videoAvailable) {
    QSettings settings;
    bool treadmill_force_speed =
        settings.value(QZSettings::treadmill_force_speed, QZSettings::default_treadmill_force_speed).toBool();
    this->bluetoothManager = b;
    this->rows = rows;
    this->loadedRows = rows;
    if (description)
        this->description = *description;
    if (tags)
        this->tags = *tags;
    
    if(settings.value(QZSettings::zwift_username, QZSettings::default_zwift_username).toString().length() > 0) {
        static bool zwift_auth_toast_shown = false;

        zwift_auth_token = new AuthToken(settings.value(QZSettings::zwift_username, QZSettings::default_zwift_username).toString(), settings.value(QZSettings::zwift_password, QZSettings::default_zwift_password).toString());

        // Connect signal to show toast when token response arrives
        if(!zwift_auth_toast_shown && homeform::singleton()) {
            connect(zwift_auth_token, &AuthToken::tokenReceived, [&zwift_auth_toast_shown](bool success, const QString& message) {
                if(!zwift_auth_toast_shown && homeform::singleton()) {
                    homeform::singleton()->setToastRequested(message);
                    zwift_auth_toast_shown = true;
                }
            });
        }

        zwift_auth_token->getAccessToken();
    }

    /*
    int c = 0;
    for (c = 0; c < rows.length(); c++) {
        qDebug()  << qSetRealNumberPrecision(10)<< "Trainprogramdata"
                 << c
                 << rows.at(c).latitude
                 << rows.at(c).longitude
                 << rows.at(c).altitude
                 << QTime(0, 0, 0).secsTo(rows.at(c).gpxElapsed)
                 << rows.at(c).distance
                 << rows.at(c).inclination
                 << QTime(0, 0, 0).secsTo(rows.at(c).duration)
                 << QTime(0, 0, 0).secsTo(rows.at(c).rampDuration)
                 << QTime(0, 0, 0).secsTo(rows.at(c).rampElapsed)
                 << rows.at(c).speed;
    }
    */

    // speed filter only to GPX workouts with timestamp

    if (rows.length() && !isnan(rows.at(0).latitude) && !isnan(rows.at(0).longitude) &&
        QTime(0, 0, 0).secsTo(rows.at(0).gpxElapsed) != 0 && !treadmill_force_speed && videoAvailable) {
        applySpeedFilter();
    }

    this->videoAvailable = videoAvailable;

    connect(&timer, SIGNAL(timeout()), this, SLOT(scheduler()));
    timer.setInterval(1s);
    timer.start();
}

QString trainrow::toString() const {
    QString rv;
    rv += QStringLiteral("duration = %1").arg(duration.toString());
    rv += QStringLiteral(" distance = %1").arg(distance);
    rv += QStringLiteral(" speed = %1").arg(speed);
    rv += QStringLiteral(" lower_speed = %1").arg(lower_speed);     // used for peloton
    rv += QStringLiteral(" average_speed = %1").arg(average_speed); // used for peloton
    rv += QStringLiteral(" upper_speed = %1").arg(upper_speed);     // used for peloton
    rv += QStringLiteral(" fanspeed = %1").arg(fanspeed);
    rv += QStringLiteral(" inclination = %1").arg(inclination);
    rv += QStringLiteral(" lower_inclination = %1").arg(lower_inclination);     // used for peloton
    rv += QStringLiteral(" average_inclination = %1").arg(average_inclination); // used for peloton
    rv += QStringLiteral(" upper_inclination = %1").arg(upper_inclination);     // used for peloton
    rv += QStringLiteral(" resistance = %1").arg(resistance);
    rv += QStringLiteral(" lower_resistance = %1").arg(lower_resistance);
    rv += QStringLiteral(" average_resistance = %1").arg(average_resistance); // used for peloton
    rv += QStringLiteral(" upper_resistance = %1").arg(upper_resistance);
    rv += QStringLiteral(" requested_peloton_resistance = %1").arg(requested_peloton_resistance);
    rv += QStringLiteral(" lower_requested_peloton_resistance = %1").arg(lower_requested_peloton_resistance);
    rv += QStringLiteral(" average_requested_peloton_resistance = %1")
              .arg(average_requested_peloton_resistance); // used for peloton
    rv += QStringLiteral(" upper_requested_peloton_resistance = %1").arg(upper_requested_peloton_resistance);
    rv += QStringLiteral(" pace_intensity = %1").arg(pace_intensity);
    rv += QStringLiteral(" cadence = %1").arg(cadence);
    rv += QStringLiteral(" lower_cadence = %1").arg(lower_cadence);
    rv += QStringLiteral(" average_cadence = %1").arg(average_cadence); // used for peloton
    rv += QStringLiteral(" upper_cadence = %1").arg(upper_cadence);
    rv += QStringLiteral(" forcespeed = %1").arg(forcespeed);
    rv += QStringLiteral(" loopTimeHR = %1").arg(loopTimeHR);
    rv += QStringLiteral(" zoneHR = %1").arg(zoneHR);
    rv += QStringLiteral(" HRmin = %1").arg(HRmin);
    rv += QStringLiteral(" HRmax = %1").arg(HRmax);
    rv += QStringLiteral(" maxSpeed = %1").arg(maxSpeed);
    rv += QStringLiteral(" minSpeed = %1").arg(minSpeed);
    rv += QStringLiteral(" maxResistance = %1").arg(maxResistance);
    rv += QStringLiteral(" power = %1").arg(power);
    rv += QStringLiteral(" mets = %1").arg(mets);
    rv += QStringLiteral(" latitude = %1").arg(latitude);
    rv += QStringLiteral(" longitude = %1").arg(longitude);
    rv += QStringLiteral(" altitude = %1").arg(altitude);
    rv += QStringLiteral(" azimuth = %1").arg(azimuth);
    rv += QStringLiteral(" rampElapsed = %1").arg(rampElapsed.toString());
    rv += QStringLiteral(" rampDuration = %1").arg(rampDuration.toString());
    return rv;
}

void trainprogram::applySpeedFilter() {
    if (rows.length() == 0)
        return;
    int r = 0;
    double weight[] = {0.15, 0.15, 0.1, 0.05, 0.05, 0.1, 0.1, 0.15, 0.15};
    QList<double> newdistance;
    newdistance.reserve(rows.length() + 1);

    while (r < rows.length()) {
        int ws = (r - 4);
        int we = (r + 4);

        // filtering starting point
        if (ws < 1)
            ws = 1;

        if (we >= rows.length())
            we = (rows.length() -
                  2); // Subtract 2 Points because duration is calculated with row+1! Fixes inf calculation in #973
        int wc = 0;
        double wma = 0;
        int rowduration = 0;
        for (wc = 0; wc <= (we - ws); wc++) {
            int currow = (ws + wc);

            // filtering starting point
            if (currow <= 1)
                rowduration = QTime(0, 0, 0).secsTo(rows.at(currow).gpxElapsed);
            else
                rowduration = ((QTime(0, 0, 0).secsTo(rows.at(currow).gpxElapsed)) -
                               (QTime(0, 0, 0).secsTo(rows.at(currow - 1).gpxElapsed)));
            // generally avoid a division by 0 or negative (who knows what's coming from gpx)
            if (rowduration > 0)
                wma += ((rows.at(currow).distance) / ((double)(rowduration)) * weight[wc]);
        }

        // filtering starting point
        if (r <= 1)
            rowduration = QTime(0, 0, 0).secsTo(rows.at(r).gpxElapsed);
        else
            rowduration =
                ((QTime(0, 0, 0).secsTo(rows.at(r).gpxElapsed)) - (QTime(0, 0, 0).secsTo(rows.at(r - 1).gpxElapsed)));

        /* it takes a lot of time during the opening of the file*/
        /*
        qDebug()  << qSetRealNumberPrecision(10)<< "TrainprogramapplySpeedFilter"
                 << r
                 << rows.at(r).latitude
                 << rows.at(r).longitude
                 << rows.at(r).altitude
                 << QTime(0, 0, 0).secsTo(rows.at(r).gpxElapsed)
                 << rows.at(r).distance
                 << (wma * ((double)(rowduration)))
                 << wma
                 << rowduration
                 << rows.at(r).inclination;*/

        newdistance.append(wma * ((double)(rowduration)));
        r++;
    }
    for (r = 0; r < rows.length(); r++) {
        rows[r].distance = newdistance.at(r);
    }
}

uint32_t trainprogram::calculateTimeForRow(int32_t row) {
    if (row >= rows.length())
        return 0;

    if (rows.at(row).distance == -1)
        return (rows.at(row).duration.second() + (rows.at(row).duration.minute() * 60) +
                (rows.at(row).duration.hour() * 3600));
    else {
        if(rows.at(row).started.isValid() && rows.at(row).ended.isValid())
            return rows.at(row).started.secsTo(rows.at(row).ended);
    }
    return 0;
}

double trainprogram::calculateDistanceForRow(int32_t row) {
    if (row >= rows.length())
        return 0;

    if (rows.at(row).distance == -1)
        return 0;
    else
        return rows.at(row).distance;
}

// meters, inclination
QList<MetersByInclination> trainprogram::inclinationNext300Meters() {
    int c = currentStep;
    double km = 0;
    QList<MetersByInclination> next300;

    while (1) {
        if (c < rows.length()) {
            if (km > 0.3) {
                return next300;
            }
            MetersByInclination p;
            if (c == currentStep) {
                p.meters = (rows.at(c).distance - currentStepDistance) * 1000.0;
                km += (rows.at(c).distance - currentStepDistance);
            } else {
                p.meters = (rows.at(c).distance) * 1000.0;
                km += (rows.at(c).distance);
            }
            p.inclination = rows.at(c).inclination;
            next300.append(p);

        } else {
            return next300;
        }
        c++;
    }
    return next300;
}

// meters, inclination
QList<MetersByInclination> trainprogram::avgInclinationNext300Meters() {
    int c = currentStep;
    double km = 0;
    QList<MetersByInclination> next300;

    while (1) {
        if (c < rows.length()) {
            if (km > 0.3) {
                return next300;
            }
            MetersByInclination p;
            if (c == currentStep) {
                p.meters = (rows.at(c).distance - currentStepDistance) * 1000.0;
                km += (rows.at(c).distance - currentStepDistance);
            } else {
                p.meters = (rows.at(c).distance) * 1000.0;
                km += (rows.at(c).distance);
            }
            p.inclination = avgInclinationNext100Meters(c);
            next300.append(p);

        } else {
            return next300;
        }
        c++;
    }
    return next300;
}

// speed in Km/h
double trainprogram::avgSpeedFromGpxStep(int gpxStep, int seconds) {
    int start = gpxStep;
    if (gpxStep >= rows.length())
        return 0.0;
    double km = (rows.at(gpxStep).distance);
    int timesum = 0;
    if (gpxStep > 0)
        timesum = (QTime(0, 0, 0).secsTo(rows.at(gpxStep).gpxElapsed) -
                   QTime(0, 0, 0).secsTo(rows.at(gpxStep - 1).gpxElapsed));
    else
        timesum = QTime(0, 0, 0).secsTo(rows.at(gpxStep).gpxElapsed);
    int c = gpxStep + 1;
    while (1) {
        if ((timesum >= seconds) || (c >= rows.length())) {
            return (km / ((double)timesum) * 3600.0);
        }
        km += (rows.at(c).distance);
        if (c > 0)
            timesum = (timesum + QTime(0, 0, 0).secsTo(rows.at(c).gpxElapsed) -
                       QTime(0, 0, 0).secsTo(rows.at(c - 1).gpxElapsed));
        c++;
    }
    return (km / ((double)timesum) * 3600.0);
}

int trainprogram::TotalGPXSecs() {
    if (rows.length() == 0)
        return 0;
    return QTime(0, 0, 0).secsTo(rows.at(rows.length() - 1).gpxElapsed);
}

double trainprogram::TimeRateFromGPX(double gpxsecs, double videosecs, double currentspeed, int recordingFactor) {
    // no rows available, return 1
    if (rows.length() <= 0) {
        qDebug() << "TimeRateFromGPX no Rows";
        return 1.0;
    }
    if (videosecs == 0.0) {
        qDebug() << "TimeRateFromGPX Videopos = 0";
        return 1.0;
    }
    double prevAvgSpeed = lastGpxSpeedSet;
    double avgNextSpeed = -1.0;
    if (prevAvgSpeed == 0.0)
        avgNextSpeed = avgSpeedFromGpxStep(currentStep, 5);
    else {
        int testpos = currentStep;
        while (testpos < (currentStep + 6)) {
            double avgTestSpeed = avgSpeedFromGpxStep(testpos, 5);
            double deviation = (avgTestSpeed / prevAvgSpeed);
            if (deviation >= 0.85 && deviation <= 1.15) {
                avgNextSpeed = avgTestSpeed;
                testpos = (currentStep + 6);
            }
            testpos++;
        }
    }
    if (avgNextSpeed == -1.0) {
        avgNextSpeed = avgSpeedFromGpxStep(currentStep, 5);
    }
    // Avoid a Division by Zero
    if (avgNextSpeed == 0.0) {
        qDebug() << "TimeRateFromGPX Nextspeed = 0";
        return 1.0;
    }

    // set the maximum Speed that the player can reached based on the Video speed.
    // if Rate get too high the Video jumps
    if (bluetoothManager->device()->deviceType() == BIKE) {
        double avgSpeedForLimit = avgSpeedFromGpxStep(currentStep + 1, 5);
        if (avgSpeedForLimit > 0.0) {
            bike *dev = (bike *)bluetoothManager->device();
            // bepo70: Replay allows Factor 2 max, so set the speed Limit to 2 * Video recording Factor speed to
            //         avoid any jumps in Video
            dev->setSpeedLimit(avgSpeedForLimit * (double)recordingFactor * 2.0 / 3.0);
        }
    }
    if (gpxsecs == lastGpxRateSetAt) {
        qDebug() << "TimeRateFromGPX Gpxpos=lastPos" << lastGpxRateSet;
        return lastGpxRateSet;
    }
    // Calculate the Factor between current Players Speed and the next average GPX Speed
    double playedToGpxSpeedFactor = (currentspeed / avgNextSpeed);
    // Calculate where the gpx would be in 1 Second
    double gpxTarget = (gpxsecs + playedToGpxSpeedFactor);
    // Get needed Rate for the next second
    double rate = (gpxTarget - videosecs);

    // If rate < 0 Video is highly before the gpx and Video would be rewinded. Wait with Video for gpx to reach it
    if (rate < 0.0) {
        rate = 0.1;
    }

    qDebug() << qSetRealNumberPrecision(10) << "TimeRateFromGPX" << gpxsecs << videosecs << (gpxsecs - videosecs)
             << currentspeed << avgNextSpeed << gpxTarget << lastGpxRateSetAt << lastGpxRateSet << rate;

    // Save the last Gpx Timestamp and the last Rate for later calls.
    lastGpxSpeedSet = avgNextSpeed;
    if (lastGpxRateSetAt != gpxsecs) {
        lastGpxRateSetAt = gpxsecs;
        lastGpxRateSet = rate;
    }
    return rate;
}

// Calculate the Median Inclination for a given Step. Median is built from the given Step -2 Steps and +2 Steps (5 Steps
// in total)
double trainprogram::medianInclination(int step) {
    QList<double> inclinations;
    inclinations.reserve(5);
    if (rows.length() == 0)
        return 0;
    if ((step > 1) && (rows.length() > step - 2))
        inclinations.append(rows.at(step - 2).inclination);
    else
        inclinations.append(0);
    if ((step > 0) && (rows.length() > step - 1))
        inclinations.append(rows.at(step - 1).inclination);
    else
        inclinations.append(0);
    if (rows.length() > step)
        inclinations.append(rows.at(step).inclination);
    else
        inclinations.append(0);
    if (rows.length() > step + 1)
        inclinations.append(rows.at(step + 1).inclination);
    else
        inclinations.append(0);
    if (rows.length() > step + 2)
        inclinations.append(rows.at(step + 2).inclination);
    else
        inclinations.append(0);
    std::sort(inclinations.begin(), inclinations.end());
    return (inclinations.at(2));
}

// Calculates a weighted Inclination for a given Step. Inclination is calculated for the given Step + windowsize Steps
// (7) The inclination for each Point needed goes through a Median Filter first to eliminate/minimize Errors in the
// recorded elevation Data
double trainprogram::weightedInclination(int step) {
    int windowsize = 7;
    int firststep = step;
    double inc = 0;
    double sumweights = 0;
    double pointweight = 0;
    if (rows.length() == 0)
        return 0;
    // Determine first and last possible Steps
    if (firststep < 0)
        firststep = 0;
    int laststep = step + windowsize;
    if (laststep >= rows.length()) {
        firststep = rows.length() - 1 - (windowsize * 2);
        if (firststep < 0)
            firststep = 0;
    }
    // Loop through the determined Steps
    for (int s = firststep; s <= laststep; s++) {
        // Calculate the Weight used for the inclination
        pointweight = ((((double)windowsize * 2.0) - 1.0) - ((s - firststep) * 2.0));
        // Calculate the sum of weights
        sumweights = (sumweights + pointweight);
        // Calculate the sum of weighted median inclinations
        inc = (inc + (medianInclination(s)) * pointweight);
    }
    // avoid a Division by 0
    if (sumweights == 0)
        return 0;
    // Return the sum of weighted median inclinations / sum of all weights
    return (inc / sumweights);
}

double trainprogram::avgInclinationNext100Meters(int step) {
    int c = step;
    double km = 0;
    double avg = 0;
    int sum = 0;

    while (1) {
        if (c < rows.length()) {
            if (km > 0.1) {
                if (sum == 1) {
                    return rows.at(currentStep).inclination;
                }
                return avg / (double)km;
            }
            if (c == currentStep)
                km += (rows.at(c).distance - currentStepDistance);
            else
                km += (rows.at(c).distance);
            avg += rows.at(c).inclination * rows.at(c).distance;
            sum++;

        } else {
            if (sum == 1) {
                return rows.at(currentStep).inclination;
            }
            return avg / (double)km;
        }
        c++;
    }
    if (sum == 1) {
        return rows.at(currentStep).inclination;
    }
    return avg / (double)km;
}

double trainprogram::avgAzimuthNext300Meters() {
    int c = currentStep;
    double km = 0;
    double sinTotal = 0;
    double cosTotal = 0;

    if (!isnan(rows.at(c).latitude) && !isnan(rows.at(c).longitude)) {
        while (1) {
            if (c < rows.length()) {
                if (km > 0.3) {
                    double averageDirection = atan(sinTotal / cosTotal) * (180 / M_PI);

                    if (cosTotal < 0) {
                        averageDirection += 180;
                    } else if (sinTotal < 0) {
                        averageDirection += 360;
                    }
                    return averageDirection;
                }

                for (double i = 0; i < rows.at(c).distance; i += 0.001) {
                    sinTotal += sin(rows.at(c).azimuth * (M_PI / 180));
                    cosTotal += cos(rows.at(c).azimuth * (M_PI / 180));
                }

                km += rows.at(c).distance;

            } else {
                double averageDirection = atan(sinTotal / cosTotal) * (180 / M_PI);

                if (cosTotal < 0) {
                    averageDirection += 180;
                } else if (sinTotal < 0) {
                    averageDirection += 360;
                }
                return averageDirection;
            }
            c++;
        }
    }
    return 0;
}

void trainprogram::clearRows() {
    QMutexLocker(&this->schedulerMutex);
    rows.clear();
}

void trainprogram::pelotonOCRprocessPendingDatagrams() {
    qDebug() << "in !";
    QHostAddress sender;
    QSettings settings;
    uint16_t port;
    while (pelotonOCRsocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(pelotonOCRsocket->pendingDatagramSize());
        pelotonOCRsocket->readDatagram(datagram.data(), datagram.size(), &sender, &port);
        qDebug() << "PelotonOCR Message From :: " << sender.toString();
        qDebug() << "PelotonOCR Port From :: " << port;
        qDebug() << "PelotonOCR Message :: " << datagram;

        QString s = datagram;
        pelotonOCRcomputeTime(s);

        QString url = "http://" + localipaddress::getIP(sender).toString() + ":" +
                      QString::number(settings.value("template_inner_QZWS_port", 6666).toInt()) +
                      "/floating/floating.htm";
        int r = pelotonOCRsocket->writeDatagram(QByteArray(url.toLatin1()), sender, 8003);
        qDebug() << "url floating" << url << r;
    }
}

void trainprogram::pelotonOCRcomputeTime(QString t) {
    static bool pelotonOCRcomputeTime_intro = false;
    static bool pelotonOCRcomputeTime_syncing = false;
    QRegularExpression re("\\d\\d:\\d\\d");
    QRegularExpressionMatch match = re.match(t.left(5));
    if (t.contains(QStringLiteral("INTRO")) || t.contains(QStringLiteral("UNTIL START"))) {
        qDebug() << QStringLiteral("PELOTON OCR: SKIPPING INTRO, restarting training program");
        if (!pelotonOCRcomputeTime_intro) {
            pelotonOCRcomputeTime_intro = true;
            emit toastRequest("Peloton Syncing! Skipping intro...");
        }
        restart();
    } else if (match.hasMatch()) {
        int minutes = t.left(2).toInt();
        int seconds = t.left(5).right(2).toInt();
        seconds -= 1; //(due to the OCR delay)
        seconds += minutes * 60;
        QTime ocrRemaining = QTime(0, 0, 0, 0).addSecs(seconds);
        QTime currentRemaining = remainingTime();
        qDebug() << QStringLiteral("PELOTON OCR USING: ocrRemaining") << ocrRemaining
                 << QStringLiteral("currentRemaining") << currentRemaining;
        uint32_t abs = qAbs(ocrRemaining.secsTo(currentRemaining));
        if (abs < 120) {
            qDebug() << QStringLiteral("PELOTON OCR SYNCING!");
            if (!pelotonOCRcomputeTime_syncing) {
                pelotonOCRcomputeTime_syncing = true;
                emit toastRequest("Peloton Syncing!");
            }
            // applying the differences
            if (ocrRemaining > currentRemaining)
                decreaseElapsedTime(abs);
            else
                increaseElapsedTime(abs);
        }
    }
}

void trainprogram::scheduler() {

    QMutexLocker(&this->schedulerMutex);
    QSettings settings;
    QDateTime now = QDateTime::currentDateTime();
    qint64 msecsElapsed = lastSchedulerCall.msecsTo(now);

    // Reset jitter if it's getting too large
    if (qAbs(currentTimerJitter) > 5000) {
        currentTimerJitter = 0;
    }

    currentTimerJitter += msecsElapsed - 1000;
    lastSchedulerCall = now;

    // outside the if case about a valid train program because the information for the floating window url should be
    // sent anyway
    if (settings.value(QZSettings::peloton_companion_workout_ocr, QZSettings::default_companion_peloton_workout_ocr)
            .toBool()) {
        if (!pelotonOCRsocket) {
            pelotonOCRsocket = new QUdpSocket(this);
            bool result = pelotonOCRsocket->bind(QHostAddress::AnyIPv4, 8003);
            qDebug() << result;
            pelotonOCRprocessPendingDatagrams();
            connect(pelotonOCRsocket, SIGNAL(readyRead()), this, SLOT(pelotonOCRprocessPendingDatagrams()));
        }
    }

    if (rows.count() == 0 || started == false || enabled == false || bluetoothManager->device() == nullptr ||
        (bluetoothManager->device()->currentSpeed().value() <= 0 &&
         !settings.value(QZSettings::continuous_moving, QZSettings::default_continuous_moving).toBool()) ||
        bluetoothManager->device()->isPaused()) {
        
        if(bluetoothManager->device() && (bluetoothManager->device()->deviceType() == TREADMILL || bluetoothManager->device()->deviceType() == ELLIPTICAL) &&
           settings.value(QZSettings::zwift_username, QZSettings::default_zwift_username).toString().length() > 0 && zwift_auth_token &&
           zwift_auth_token->access_token.length() > 0) {
            if(!zwift_world) {
                zwift_world = new World(1, zwift_auth_token->getAccessToken());
                qDebug() << "creating zwift api world";
            }
            else {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
                if(!h)
                    h = new lockscreen();
#endif
#endif
                if(zwift_player_id == -1) {
                    QString id = zwift_world->player_id();
                    QJsonParseError parseError;
                    QJsonDocument document = QJsonDocument::fromJson(id.toLocal8Bit(), &parseError);
                    QJsonObject ride = document.object();
                    qDebug() << "zwift api player" << ride;
                    zwift_player_id = ride[QStringLiteral("id")].toInt();
                    emit zwiftLoginState(true);
                } else {                    
                    static int zwift_counter = 5;
                    int timeout = settings.value(QZSettings::zwift_api_poll, QZSettings::default_zwift_api_poll).toInt();
                    if(timeout < 5)
                        timeout = 5;
                    if(zwift_counter++ >= (timeout - 1)) {
                        zwift_counter = 0;
                        QByteArray bb = zwift_world->playerStatus(zwift_player_id);
                        qDebug() << " ZWIFT API PROTOBUF << " + bb.toHex(' ');
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
                        h->zwift_api_decodemessage_player(bb.data(), bb.length());
                        float alt = h->zwift_api_getaltitude();
                        float distance = h->zwift_api_getdistance();
#else
                        float alt = 0;
                        float distance = 0;
#endif
#elif defined(Q_OS_ANDROID)
                        QAndroidJniEnvironment env;
                        jbyteArray d = env->NewByteArray(bb.length());
                        jbyte *b = env->GetByteArrayElements(d, 0);
                        for (int i = 0; i < bb.length(); i++)
                            b[i] = bb[i];
                        env->SetByteArrayRegion(d, 0, bb.length(), b);

                        QAndroidJniObject::callStaticMethod<void>(
                            "org/cagnulen/qdomyoszwift/ZwiftAPI", "zwift_api_decodemessage_player", "([B)V", d);
                        env->DeleteLocalRef(d);

                        float alt = QAndroidJniObject::callStaticMethod<float>("org/cagnulen/qdomyoszwift/ZwiftAPI", "getAltitude", "()F");
                        float distance = QAndroidJniObject::callStaticMethod<float>("org/cagnulen/qdomyoszwift/ZwiftAPI", "getDistance", "()F");
#elif defined Q_CC_MSVC
                        PlayerState state;
                        float alt = 0;
                        float distance = 0;
                        if (state.ParseFromArray(bb.constData(), bb.size())) {
                            // Parsing riuscito, ora puoi accedere ai dati in `state`
                            alt = state.altitude();
                            distance = state.distance();
                        } else {
                            // Errore durante il parsing
                            qDebug() << "Error parsing PlayerState";
                        }
#else
                        float alt = 0;
                        float distance = 0;
#endif
                        static float old_distance = 0;
                        static float old_alt = 0;
                        
                        qDebug() << "zwift api incline1" << old_distance << old_alt << distance << alt;

                        if(old_distance > 0) {
                            float delta = distance - old_distance;
                            float deltaA = alt - old_alt;
                            float incline = (deltaA / delta);
                            if(delta > 1) {
                                bool zwift_negative_inclination_x2 =
                                    settings.value(QZSettings::zwift_negative_inclination_x2, QZSettings::default_zwift_negative_inclination_x2)
                                        .toBool();
                                double offset =
                                    settings.value(QZSettings::zwift_inclination_offset, QZSettings::default_zwift_inclination_offset).toDouble();
                                double gain =
                                    settings.value(QZSettings::zwift_inclination_gain, QZSettings::default_zwift_inclination_gain).toDouble();
                                double grade = (incline * gain) + offset;  
                                if (zwift_negative_inclination_x2 && incline < 0) {
                                    grade = ((incline * 2.0) * gain) + offset;
                                }                              
                                bool zwift_api_autoinclination = settings.value(QZSettings::zwift_api_autoinclination, QZSettings::default_zwift_api_autoinclination).toBool();
                                qDebug() << "zwift api incline" << incline << grade << delta << deltaA << zwift_api_autoinclination;
                                if(zwift_api_autoinclination) {
                                    if(bluetoothManager->device()->deviceType() == TREADMILL || 
                                        (bluetoothManager->device()->deviceType() == ELLIPTICAL && ((elliptical*)bluetoothManager->device())->inclinationAvailableByHardware())) {
                                        bluetoothManager->device()->changeInclination(grade, grade);
                                    }
                                    if (bluetoothManager->device()->deviceType() == ELLIPTICAL &&
                                            (!((elliptical*)bluetoothManager->device())->inclinationAvailableByHardware() ||
                                             ((elliptical*)bluetoothManager->device())->inclinationSeparatedFromResistance())) {
                                        QSettings settings;
                                        double bikeResistanceOffset = settings.value(QZSettings::bike_resistance_offset, bikeResistanceOffset).toInt();
                                        double bikeResistanceGain = settings.value(QZSettings::bike_resistance_gain_f, bikeResistanceGain).toDouble();

                                        bluetoothManager->device()->changeResistance((resistance_t)(round(grade * bikeResistanceGain)) + bikeResistanceOffset + 1); // resistance start from 1
                                    }
                                }
                            }
                        }
                        old_distance = distance;
                        old_alt = alt;
                    }
                }
            }
        }

        // in case no workout has been selected
        // Zwift OCR
        if ((settings.value(QZSettings::zwift_ocr, QZSettings::default_zwift_ocr).toBool() ||
             settings.value(QZSettings::zwift_ocr_climb_portal, QZSettings::default_zwift_ocr_climb_portal).toBool()) &&
            bluetoothManager && bluetoothManager->device() &&
            (bluetoothManager->device()->deviceType() == TREADMILL ||
             bluetoothManager->device()->deviceType() == ELLIPTICAL)) {

#ifdef Q_OS_ANDROID
            {
                QAndroidJniObject text = QAndroidJniObject::callStaticObjectMethod<jstring>(
                    "org/cagnulen/qdomyoszwift/ScreenCaptureService", "getLastText");
                QString t = text.toString();
                QAndroidJniObject textExtended = QAndroidJniObject::callStaticObjectMethod<jstring>(
                    "org/cagnulen/qdomyoszwift/ScreenCaptureService", "getLastTextExtended");
                // 2272 1027
                jint w = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/ScreenCaptureService",
                                                                   "getImageWidth", "()I");
                jint h = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/ScreenCaptureService",
                                                                   "getImageHeight", "()I");
                QString tExtended = textExtended.toString();
                QAndroidJniObject packageNameJava = QAndroidJniObject::callStaticObjectMethod<jstring>(
                    "org/cagnulen/qdomyoszwift/MediaProjection", "getPackageName");
                QString packageName = packageNameJava.toString();
                if (packageName.contains("com.zwift.zwiftgame")) {
                    qDebug() << QStringLiteral("ZWIFT OCR ACCEPTED") << packageName << w << h << t << tExtended;
                    foreach (QString s, tExtended.split("§§")) {
                        // qDebug() << s;
                        QStringList ss = s.split("$$");
                        if (ss.length() > 1) {
                            // (2195, 75 - 2254, 106)"
                            qDebug() << ss[0] << ss[1];
                            QString inc = ss[1].replace("Rect(", "").replace(")", "");
                            if (inc.split(",").length() > 2) {
                                int w_minbound = w * 0.93;
                                int h_minbound = h * 0.08;
                                int h_maxbound = h * 0.15;
                                int x = inc.split(",").at(0).toInt();
                                int y = inc.split(",").at(2).toInt();
                                qDebug() << x << w_minbound << h_maxbound << y << h_minbound;
                                if (x > w_minbound && y < h_maxbound && y > h_minbound) {
                                    ss[0] = ss[0].replace("%", "");
                                    ss[0] = ss[0].replace("O", "0");
                                    ss[0] = ss[0].replace("l", "1");
                                    ss[0] = ss[0].replace(" ", "");
                                    if (ss[0].toInt() < 15 && ss[0].toInt() > -15) {
                                        bluetoothManager->device()->changeInclination(ss[0].toInt(), ss[0].toInt());
                                    } else {
                                        qDebug() << "filtering" << ss[0].toInt();
                                    }
                                }
                            }
                        }
                    }

                } else {
                    qDebug() << QStringLiteral("ZWIFT OCR IGNORING") << packageName << t;
                }
            }
#elif defined(Q_OS_WINDOWS)
            static windows_zwift_incline_paddleocr_thread *windows_zwift_ocr_thread = nullptr;
            if (!windows_zwift_ocr_thread) {
                windows_zwift_ocr_thread = new windows_zwift_incline_paddleocr_thread(bluetoothManager->device());
                connect(windows_zwift_ocr_thread, &windows_zwift_incline_paddleocr_thread::debug, bluetoothManager,
                        &bluetooth::debug);
                connect(windows_zwift_ocr_thread, &windows_zwift_incline_paddleocr_thread::onInclination, this,
                        &trainprogram::changeInclination);
                windows_zwift_ocr_thread->start();
            }
#endif
        } else if (settings.value(QZSettings::zwift_workout_ocr, QZSettings::default_zwift_workout_ocr).toBool() &&
                   bluetoothManager && bluetoothManager->device() &&
                   (bluetoothManager->device()->deviceType() == TREADMILL ||
                    bluetoothManager->device()->deviceType() == ELLIPTICAL)) {
#ifdef Q_OS_WINDOWS
            static windows_zwift_workout_paddleocr_thread *windows_zwift_workout_ocr_thread = nullptr;
            if (!windows_zwift_workout_ocr_thread) {
                windows_zwift_workout_ocr_thread =
                    new windows_zwift_workout_paddleocr_thread(bluetoothManager->device());
                connect(windows_zwift_workout_ocr_thread, &windows_zwift_workout_paddleocr_thread::debug,
                        bluetoothManager, &bluetooth::debug);
                connect(windows_zwift_workout_ocr_thread, &windows_zwift_workout_paddleocr_thread::onInclination, this,
                        &trainprogram::changeInclination);
                connect(windows_zwift_workout_ocr_thread, &windows_zwift_workout_paddleocr_thread::onSpeed, this,
                        &trainprogram::changeSpeed);
                windows_zwift_workout_ocr_thread->start();
            }
#endif
        }

        currentTimerJitter = 0;
        return;
    }

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::peloton_workout_ocr, QZSettings::default_peloton_workout_ocr).toBool()) {
        QAndroidJniObject text = QAndroidJniObject::callStaticObjectMethod<jstring>(
            "org/cagnulen/qdomyoszwift/ScreenCaptureService", "getLastText");
        QString t = text.toString();
        QAndroidJniObject packageNameJava = QAndroidJniObject::callStaticObjectMethod<jstring>(
            "org/cagnulen/qdomyoszwift/MediaProjection", "getPackageName");
        QString packageName = packageNameJava.toString();
        if (packageName.contains("com.onepeloton.callisto")) {
            qDebug() << QStringLiteral("PELOTON OCR ACCEPTED") << packageName << t;
            pelotonOCRcomputeTime(t);
        } else {
            qDebug() << QStringLiteral("PELOTON OCR IGNORING") << packageName << t;
        }
    }
#endif

    ticks++;
    qDebug() << QStringLiteral("trainprogram ticks") << ticks << QStringLiteral("currentTimerJitter") << currentTimerJitter;

    if(qAbs(currentTimerJitter) > 1000) {
        int seconds = currentTimerJitter / 1000;
        ticks += seconds;
        currentTimerJitter -= (seconds * 1000);
        qDebug() << QStringLiteral("fixing jitter!") << seconds << ticks << currentTimerJitter;
    }

    double odometerFromTheDevice = bluetoothManager->device()->odometer();

    if(ticks < 0) {
        qDebug() << "waiting for the start...";
        return;
    }
    
    // entry point
    if (ticks == 1 && currentStep == 0) {
        rows[currentStep].started = QDateTime::currentDateTime();
        currentStepDistance = 0;
        lastOdometer = odometerFromTheDevice;
        if (bluetoothManager->device()->deviceType() == TREADMILL) {
            if (rows.at(0).forcespeed && rows.at(0).speed) {
                qDebug() << QStringLiteral("trainprogram change speed") + QString::number(rows.at(0).speed);
                emit changeSpeed(rows.at(0).speed);
            }
            if (rows.at(0).inclination != -200) {
                double inc;
                if (!isnan(rows.at(0).latitude) && !isnan(rows.at(0).longitude)) {
                    inc = avgInclinationNext100Meters(currentStep);
                } else {
                    inc = rows.at(0).inclination;
                }
                qDebug() << QStringLiteral("trainprogram change inclination") + QString::number(inc);
                emit changeInclination(inc, inc);
                emit changeNextInclination300Meters(avgInclinationNext300Meters());
            }
            if (rows.at(0).power != -1) {
                qDebug() << QStringLiteral("trainprogram change power") + QString::number(rows.at(0).power);
                emit changePower(rows.at(0).power);
            }
        } else if (bluetoothManager->device()->deviceType() == ROWING) {
            if (rows.at(0).forcespeed && rows.at(0).speed) {
                qDebug() << QStringLiteral("trainprogram change speed") + QString::number(rows.at(0).speed);
                emit changeSpeed(rows.at(0).speed);
            }
            if (rows.at(0).cadence != -1) {
                qDebug() << QStringLiteral("trainprogram change cadence") + QString::number(rows.at(0).cadence);
                emit changeCadence(rows.at(0).cadence);
            }
            if (rows.at(0).power != -1) {
                qDebug() << QStringLiteral("trainprogram change power") + QString::number(rows.at(0).power);
                emit changePower(rows.at(0).power);
            }
            if (rows.at(0).resistance != -1) {
                qDebug() << QStringLiteral("trainprogram change resistance") + QString::number(rows.at(0).resistance);
                emit changeResistance(rows.at(0).resistance);
            }
        } else {
            if (rows.at(0).resistance != -1) {
                qDebug() << QStringLiteral("trainprogram change resistance") + QString::number(rows.at(0).resistance);
                emit changeResistance(rows.at(0).resistance);
            }

            if (rows.at(0).cadence != -1) {
                qDebug() << QStringLiteral("trainprogram change cadence") + QString::number(rows.at(0).cadence);
                emit changeCadence(rows.at(0).cadence);
            }

            if (rows.at(0).power != -1) {
                qDebug() << QStringLiteral("trainprogram change power") + QString::number(rows.at(0).power);
                emit changePower(rows.at(0).power);
            }

            if (rows.at(0).requested_peloton_resistance != -1) {
                qDebug() << QStringLiteral("trainprogram change requested peloton resistance") +
                                QString::number(rows.at(0).requested_peloton_resistance);
                emit changeRequestedPelotonResistance(rows.at(0).requested_peloton_resistance);
            }

            if (rows.at(0).inclination != -200 && (bluetoothManager->device()->deviceType() == BIKE ||
            (bluetoothManager->device()->deviceType() == ELLIPTICAL && !((elliptical*)bluetoothManager->device())->inclinationAvailableByHardware()))) {
                // this should be converted in a signal as all the other signals...

                double inc = rows.at(0).inclination;

                // Only convert inclination to resistance for bikes WITHOUT hardware inclination support
                // Ellipticals only enter here if they don't have hardware inclination (checked in outer condition)
                if ((bluetoothManager->device()->deviceType() == BIKE && !((bike *)bluetoothManager->device())->inclinationAvailableBySoftware()) ||
                    (bluetoothManager->device()->deviceType() == ELLIPTICAL)) {
                    double bikeResistanceOffset =
                        settings.value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset)
                            .toInt();
                    double bikeResistanceGain =
                        settings.value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f)
                            .toDouble();

                    bluetoothManager->device()->changeResistance((resistance_t)(round(inc * bikeResistanceGain)) +
                                                                 bikeResistanceOffset + 1); // resistance start from 1
                }

                if (bluetoothManager->device()->deviceType() == BIKE)
                    bluetoothManager->device()->setInclination(inc);

                qDebug() << QStringLiteral("trainprogram change inclination") + QString::number(inc);
                emit changeInclination(inc, inc);
                emit changeNextInclination300Meters(inclinationNext300Meters());
            }
        }

        if (rows.at(0).fanspeed != -1) {
            qDebug() << QStringLiteral("trainprogram change fanspeed") + QString::number(rows.at(0).fanspeed);
            emit changeFanSpeed(rows.at(0).fanspeed);
        }

        if (!isnan(rows.at(0).latitude) || !isnan(rows.at(0).longitude) || !isnan(rows.at(0).altitude)) {
            qDebug() << qSetRealNumberPrecision(10) << QStringLiteral("trainprogram change GEO position")
                     << rows.at(0).latitude << rows.at(0).longitude << rows.at(0).altitude << rows.at(0).azimuth;
            QGeoCoordinate p;
            p.setAltitude(rows.at(0).altitude);
            p.setLatitude(rows.at(0).latitude);
            p.setLongitude(rows.at(0).longitude);
            emit changeGeoPosition(p, rows.at(0).azimuth, avgAzimuthNext300Meters());
        }
    }

    uint32_t currentRowLen = calculateTimeForRow(currentStep);

    qDebug() << QStringLiteral("trainprogram elapsed ") + QString::number(ticks) + QStringLiteral("current row len") +
                    QString::number(currentRowLen);

    uint32_t calculatedLine;
    uint32_t calculatedElapsedTime = 0;
    for (calculatedLine = 0; calculatedLine < static_cast<uint32_t>(rows.length()); calculatedLine++) {

        calculatedElapsedTime += calculateTimeForRow(calculatedLine);
        
        if (calculateDistanceForRow(calculatedLine) > 0 && calculatedLine >= currentStep) {
            break;
        }

        if (calculatedElapsedTime > static_cast<uint32_t>(ticks) && calculatedLine >= currentStep) {
            break;
        }
    }

    bool distanceEvaluation = false;
    int sameIteration = 0;

    do {

        currentStepDistance += (odometerFromTheDevice - lastOdometer);
        lastOdometer = odometerFromTheDevice;

        if(currentStep >= rows.length()) {
            qDebug() << "currentStep greater than row.length" << currentStep << rows.length();
            end();
            return;
        }
        bool distanceStep = (rows.at(currentStep).distance > 0);
        distanceEvaluation = (distanceStep && currentStepDistance >= rows.at(currentStep).distance);
        qDebug() << qSetRealNumberPrecision(10) << QStringLiteral("currentStepDistance") << currentStepDistance
                 << QStringLiteral("distanceStep") << distanceStep << QStringLiteral("distanceEvaluation")
                 << distanceEvaluation << QStringLiteral("rows distance") << rows.at(currentStep).distance
                 << QStringLiteral("same iteration") << sameIteration;

        if ((calculatedLine != currentStep && !distanceStep) || distanceEvaluation) {
            if (calculateTimeForRow(calculatedLine) || calculateDistanceForRow(calculatedLine) > 0) {

                if(rows.at(currentStep).distance != -1)
                    lastOdometer -= (currentStepDistance - rows.at(currentStep).distance);

                rows[currentStep].ended = QDateTime::currentDateTime();

                if (!distanceStep)
                    currentStep = calculatedLine;
                else
                    currentStep++;

                if(currentStep >= rows.length()) {
                    qDebug() << "currentStep greater than row.length" << currentStep << rows.length();
                    end();
                    return;
                }

                calculatedLine = currentStep;

                rows[currentStep].started = QDateTime::currentDateTime();

                currentStepDistance = 0;
                if (bluetoothManager->device()->deviceType() == TREADMILL) {
                    if (rows.at(currentStep).forcespeed && rows.at(currentStep).speed) {
                        qDebug() << QStringLiteral("trainprogram change speed ") +
                                        QString::number(rows.at(currentStep).speed);
                        double speed;
                        if (!isnan(rows.at(currentStep).latitude) && !isnan(rows.at(currentStep).longitude)) {
                            speed = avgSpeedFromGpxStep(currentStep, 60);
                        } else {
                            speed = rows.at(currentStep).speed;
                        }
                        emit changeSpeed(speed);
                    }
                    if (rows.at(currentStep).inclination != -200) {
                        double inc;
                        if (!isnan(rows.at(currentStep).latitude) && !isnan(rows.at(currentStep).longitude)) {
                            inc = avgInclinationNext100Meters(currentStep);
                        } else {
                            inc = rows.at(currentStep).inclination;
                        }
                        qDebug() << QStringLiteral("trainprogram change inclination") + QString::number(inc);
                        emit changeInclination(inc, inc);
                        emit changeNextInclination300Meters(avgInclinationNext300Meters());
                    }
                    if (rows.at(currentStep).power != -1) {
                        qDebug() << QStringLiteral("trainprogram change power ") +
                                        QString::number(rows.at(currentStep).power);
                        emit changePower(rows.at(currentStep).power);
                    }
                } else if (bluetoothManager->device()->deviceType() == ROWING) {
                    if (rows.at(currentStep).forcespeed && rows.at(currentStep).speed) {
                        qDebug() << QStringLiteral("trainprogram change speed ") +
                                        QString::number(rows.at(currentStep).speed);
                        double speed;
                        if (!isnan(rows.at(currentStep).latitude) && !isnan(rows.at(currentStep).longitude)) {
                            speed = avgSpeedFromGpxStep(currentStep, 60);
                        } else {
                            speed = rows.at(currentStep).speed;
                        }
                        emit changeSpeed(speed);
                    }
                    if (rows.at(currentStep).cadence != -1) {
                        qDebug() << QStringLiteral("trainprogram change cadence ") +
                                        QString::number(rows.at(currentStep).cadence);
                        emit changeCadence(rows.at(currentStep).cadence);
                    }
                    if (rows.at(currentStep).power != -1) {
                        qDebug() << QStringLiteral("trainprogram change power ") +
                                        QString::number(rows.at(currentStep).power);
                        emit changePower(rows.at(currentStep).power);
                    }
                    if (rows.at(currentStep).resistance != -1) {
                        qDebug() << QStringLiteral("trainprogram change resistance ") +
                                        QString::number(rows.at(currentStep).resistance);
                        emit changeResistance(rows.at(currentStep).resistance);
                    }
                } else {
                    if (rows.at(currentStep).resistance != -1) {
                        qDebug() << QStringLiteral("trainprogram change resistance ") +
                                        QString::number(rows.at(currentStep).resistance);
                        emit changeResistance(rows.at(currentStep).resistance);
                    }

                    if (rows.at(currentStep).cadence != -1) {
                        qDebug() << QStringLiteral("trainprogram change cadence ") +
                                        QString::number(rows.at(currentStep).cadence);
                        emit changeCadence(rows.at(currentStep).cadence);
                    }

                    if (rows.at(currentStep).power != -1) {
                        qDebug() << QStringLiteral("trainprogram change power ") +
                                        QString::number(rows.at(currentStep).power);
                        emit changePower(rows.at(currentStep).power);
                    }

                    if (rows.at(currentStep).requested_peloton_resistance != -1) {
                        qDebug() << QStringLiteral("trainprogram change requested peloton resistance ") +
                                        QString::number(rows.at(currentStep).requested_peloton_resistance);
                        emit changeRequestedPelotonResistance(rows.at(currentStep).requested_peloton_resistance);
                    }

                    if (rows.at(currentStep).inclination != -200 &&
                        (bluetoothManager->device()->deviceType() == BIKE ||
                        (bluetoothManager->device()->deviceType() == ELLIPTICAL && !((elliptical*)bluetoothManager->device())->inclinationAvailableByHardware()))) {
                        // this should be converted in a signal as all the other signals...

                        double inc = rows.at(currentStep).inclination;

                        // Only convert inclination to resistance for bikes WITHOUT hardware inclination support
                        // Ellipticals only enter here if they don't have hardware inclination (checked in outer condition)
                        if ((bluetoothManager->device()->deviceType() == BIKE && !((bike *)bluetoothManager->device())->inclinationAvailableBySoftware()) ||
                            (bluetoothManager->device()->deviceType() == ELLIPTICAL)) {
                            double bikeResistanceOffset =
                                settings
                                    .value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset)
                                    .toInt();
                            double bikeResistanceGain =
                                settings
                                    .value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f)
                                    .toDouble();

                            bluetoothManager->device()->changeResistance((resistance_t)(round(inc * bikeResistanceGain)) +
                                                                         bikeResistanceOffset +
                                                                         1); // resistance start from 1                            
                        }

                        if (bluetoothManager->device()->deviceType() == BIKE)
                            bluetoothManager->device()->setInclination(inc);

                        qDebug() << QStringLiteral("trainprogram change inclination") + QString::number(inc);
                        emit changeInclination(inc, inc);
                        emit changeNextInclination300Meters(inclinationNext300Meters());
                    }
                }

                if (rows.at(currentStep).fanspeed != -1) {
                    qDebug() << QStringLiteral("trainprogram change fanspeed ") +
                                    QString::number(rows.at(currentStep).fanspeed);
                    emit changeFanSpeed(rows.at(currentStep).fanspeed);
                }

                if (!isnan(rows.at(currentStep).latitude) || !isnan(rows.at(currentStep).longitude) ||
                    !isnan(rows.at(currentStep).altitude)) {
                    qDebug() << qSetRealNumberPrecision(10) << QStringLiteral("trainprogram change GEO position")
                             << rows.at(currentStep).latitude << rows.at(currentStep).longitude
                             << rows.at(currentStep).altitude << rows.at(currentStep).distance
                             << rows.at(currentStep).azimuth;

                    QGeoCoordinate p;
                    p.setLatitude(rows.at(currentStep).latitude);
                    p.setLongitude(rows.at(currentStep).longitude);
                    p.setAltitude(rows.at(currentStep).altitude);
                    // qDebug()  << qSetRealNumberPrecision(10)<< c << rows.at(currentStep+1).latitude <<
                    // rows.at(currentStep + 1).longitude <<
                    /*QGeoCoordinate c;
                    c.setLatitude(rows.at(currentStep+1).latitude);
                    c.setLongitude(rows.at(currentStep+1).longitude);
                    c.setAltitude(rows.at(currentStep+1).altitude);
                    qDebug()  << qSetRealNumberPrecision(10)<< "distance" << p.distanceTo(c) <<
                    rows.at(currentStep).distance;*/

                    if (odometerFromTheDevice - lastOdometer > 0)
                        p = p.atDistanceAndAzimuth((odometerFromTheDevice - lastOdometer),
                                                   rows.at(currentStep).azimuth);
                    qDebug() << qSetRealNumberPrecision(10) << "positionOffset"
                             << (odometerFromTheDevice - lastOdometer);
                    emit changeGeoPosition(p, rows.at(currentStep).azimuth, avgAzimuthNext300Meters());
                }
            } else {
                end();
                distanceEvaluation = false;
            }
        } else {
            if (rows.length() > currentStep && rows.at(currentStep).power != -1) {
                qDebug() << QStringLiteral("trainprogram change power ") +
                                QString::number(rows.at(currentStep).power);
                emit changePower(rows.at(currentStep).power);
            }

            if (rows.at(currentStep).inclination != -200 &&
                (!isnan(rows.at(currentStep).latitude) && !isnan(rows.at(currentStep).longitude))) {
                double inc = avgInclinationNext100Meters(currentStep);
                // if Bike used and it is a gpx with Video use the new weightedInclination
                if ((videoAvailable) && (bluetoothManager->device()->deviceType() == BIKE)) {
                    inc = weightedInclination(currentStep);
                }

                // Only convert inclination to resistance for bikes WITHOUT hardware inclination support
                if (bluetoothManager->device()->deviceType() == BIKE && !((bike *)bluetoothManager->device())->inclinationAvailableBySoftware()) {
                    double bikeResistanceOffset =
                        settings.value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset)
                            .toInt();
                    double bikeResistanceGain =
                        settings.value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f)
                            .toDouble();

                    bluetoothManager->device()->changeResistance((resistance_t)(round(inc * bikeResistanceGain)) +
                                                                 bikeResistanceOffset + 1); // resistance start from 1
                    
                }

                if (bluetoothManager->device()->deviceType() == BIKE)
                    bluetoothManager->device()->setInclination(inc);

                qDebug() << QStringLiteral("trainprogram change inclination due to gps") + QString::number(inc);
                emit changeInclination(inc, inc);
                if (bluetoothManager->device()->deviceType() == TREADMILL)
                    emit changeNextInclination300Meters(avgInclinationNext300Meters());
                else
                    emit changeNextInclination300Meters(inclinationNext300Meters());

                double ratioDistance = 0.0;
                double distanceRow = rows.at(currentStep).distance;
                int steptime = 0;
                if (lastStepTimestampChanged != currentStep) {
                    lastCurrentStepDistance = 0.0;
                    lastCurrentStepTime = QTime(0, 0, 0);
                    if (currentStep > 0) {
                        lastCurrentStepTime = rows.at(currentStep - 1).gpxElapsed;
                    }
                    lastStepTimestampChanged = currentStep;
                }
                if ((currentStep > 1) && (distanceRow != 0.0)) {
                    steptime = ((QTime(0, 0, 0).secsTo(rows.at(currentStep).gpxElapsed)) -
                                (QTime(0, 0, 0).secsTo(rows.at(currentStep - 1).gpxElapsed)));
                    if (steptime == 0)
                        steptime = 1;
                    distanceRow = (distanceRow / ((double)(steptime)));
                    ratioDistance = ((currentStepDistance - lastCurrentStepDistance) / distanceRow);
                    lastCurrentStepTime = lastCurrentStepTime.addMSecs(ratioDistance * 1000.0);
                }
                lastCurrentStepDistance = currentStepDistance;
                qDebug() << qSetRealNumberPrecision(10) << QStringLiteral("changingTimestamp") << currentStep
                         << distanceRow << currentStepDistance << lastCurrentStepDistance << ratioDistance
                         << rows.at(currentStep).gpxElapsed << lastCurrentStepTime << ticks;
                emit changeTimestamp(lastCurrentStepTime, QTime(0, 0, 0).addSecs(ticks));
            }
        }

        // Check for text events that should be displayed at this time
        if (currentStep < rows.length() && !rows.at(currentStep).textEvents.isEmpty()) {
            // Calculate elapsed time in current step
            uint32_t elapsedInCurrentStep = 0;
            if (rows.at(currentStep).started.isValid()) {
                elapsedInCurrentStep = rows.at(currentStep).started.secsTo(QDateTime::currentDateTime());
            }

            // Check each text event
            foreach (const trainrow::TextEvent &evt, rows.at(currentStep).textEvents) {
                // Create unique key for this event
                QString eventKey = QString("%1:%2").arg(currentStep).arg(evt.timeoffset);

                // Check if this event should be shown now and hasn't been shown yet
                if (elapsedInCurrentStep >= evt.timeoffset && !shownTextEvents.contains(eventKey)) {
                    qDebug() << "Showing text event at step" << currentStep << "offset" << evt.timeoffset << ":" << evt.message;

                    // Emit toast request
                    emit toastRequest(evt.message);

                    // Mark as shown
                    shownTextEvents.insert(eventKey);
                }
            }
        }

        sameIteration++;
    } while (distanceEvaluation);
}

void trainprogram::end() {
    QSettings settings;
    qDebug() << QStringLiteral("trainprogram ends!");

           // circuit?
    if (!isnan(rows.first().latitude) && !isnan(rows.first().longitude) &&
        QGeoCoordinate(rows.first().latitude, rows.first().longitude)
                .distanceTo(bluetoothManager->device()->currentCordinate()) < 50) {
        emit lap();
        restart();
    } else {
        started = false;
        if (settings
                .value(QZSettings::trainprogram_stop_at_end, QZSettings::default_trainprogram_stop_at_end)
                .toBool())
            emit stop(false);
    }
}

bool trainprogram::overrideZoneHRForCurrentRow(uint8_t zone) {
    if (started && currentStep < rows.length() && currentRow().zoneHR != -1) {
        qDebug() << "overriding zoneHR from" << rows.at(currentStep).zoneHR << "to" << zone;
        rows[currentStep].zoneHR = zone;
        return true;
    }
    return false;
}

bool trainprogram::overridePowerForCurrentRow(double power) {
    if (started && currentStep < rows.length() && currentRow().power != -1) {
        qDebug() << "overriding power from" << rows.at(currentStep).power << "to" << power;
        rows[currentStep].power = power;
        return true;
    }
    return false;
}

void trainprogram::increaseElapsedTime(int32_t i) {

    offset += i;
    ticks += i;
}

void trainprogram::decreaseElapsedTime(int32_t i) {

    offset -= i;
    ticks -= i;
}

void trainprogram::onTapeStarted() { started = true; }

void trainprogram::restart() {

    if (bluetoothManager && bluetoothManager->device())
        lastOdometer = bluetoothManager->device()->odometer();
    ticks = 0;
    offset = 0;
    currentStep = 0;
    currentTimerJitter = 0;
    shownTextEvents.clear();  // Reset shown text events when restarting
    started = true;
}

bool trainprogram::saveXML(const QString &filename, const QList<trainrow> &rows) {
    QFile output(filename);
    if (!rows.isEmpty() && output.open(QIODevice::WriteOnly)) {
        QXmlStreamWriter stream(&output);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();
        stream.writeStartElement(QStringLiteral("rows"));
        for (const trainrow &row : qAsConst(rows)) {
            stream.writeStartElement(QStringLiteral("row"));
            stream.writeAttribute(QStringLiteral("duration"), row.duration.toString());
            if (row.distance >= 0) {
                stream.writeAttribute(QStringLiteral("distance"), QString::number(row.distance));
            }
            if (row.speed >= 0) {
                stream.writeAttribute(QStringLiteral("speed"), QString::number(row.speed));
            }
            if (row.minSpeed >= 0) {
                stream.writeAttribute(QStringLiteral("minspeed"), QString::number(row.minSpeed));
            }
            if (row.inclination >= -50) {
                stream.writeAttribute(QStringLiteral("inclination"), QString::number(row.inclination));
            }
            if (row.resistance >= 0) {
                stream.writeAttribute(QStringLiteral("resistance"), QString::number(row.resistance));
            }
            if (row.lower_resistance >= 0) {
                stream.writeAttribute(QStringLiteral("lower_resistance"), QString::number(row.lower_resistance));
            }
            if (row.mets >= 0) {
                stream.writeAttribute(QStringLiteral("mets"), QString::number(row.mets));
            }
            if (!isnan(row.altitude)) {
                stream.writeAttribute(QStringLiteral("altitude"), QString::number(row.altitude));
            }
            if (!isnan(row.azimuth)) {
                stream.writeAttribute(QStringLiteral("azimuth"), QString::number(row.azimuth));
            }
            if (!isnan(row.latitude)) {
                stream.writeAttribute(QStringLiteral("latitude"), QString::number(row.latitude));
            }
            if (!isnan(row.longitude)) {
                stream.writeAttribute(QStringLiteral("longitude"), QString::number(row.longitude));
            }
            if (row.upper_resistance >= 0) {
                stream.writeAttribute(QStringLiteral("upper_resistance"), QString::number(row.upper_resistance));
            }
            if (row.requested_peloton_resistance >= 0) {
                stream.writeAttribute(QStringLiteral("requested_peloton_resistance"),
                                      QString::number(row.requested_peloton_resistance));
            }
            if (row.lower_requested_peloton_resistance >= 0) {
                stream.writeAttribute(QStringLiteral("lower_requested_peloton_resistance"),
                                      QString::number(row.lower_requested_peloton_resistance));
            }
            if (row.upper_requested_peloton_resistance >= 0) {
                stream.writeAttribute(QStringLiteral("upper_requested_peloton_resistance"),
                                      QString::number(row.upper_requested_peloton_resistance));
            }
            if (row.pace_intensity >= 0) {
                stream.writeAttribute(QStringLiteral("pace_intensity"), QString::number(row.pace_intensity));
            }
            if (row.cadence >= 0) {
                stream.writeAttribute(QStringLiteral("cadence"), QString::number(row.cadence));
            }
            if (row.lower_cadence >= 0) {
                stream.writeAttribute(QStringLiteral("lower_cadence"), QString::number(row.lower_cadence));
            }
            if (row.upper_cadence >= 0) {
                stream.writeAttribute(QStringLiteral("upper_cadence"), QString::number(row.upper_cadence));
            }
            if (row.power >= 0) {
                stream.writeAttribute(QStringLiteral("power"), QString::number(row.power));
            }
            stream.writeAttribute(QStringLiteral("forcespeed"),
                                  row.forcespeed ? QStringLiteral("1") : QStringLiteral("0"));
            if (row.fanspeed >= 0) {
                stream.writeAttribute(QStringLiteral("fanspeed"), QString::number(row.fanspeed));
            }
            if (row.maxSpeed >= 0) {
                stream.writeAttribute(QStringLiteral("maxspeed"), QString::number(row.maxSpeed));
            }
            if (row.maxResistance >= 0) {
                stream.writeAttribute(QStringLiteral("maxresistance"), QString::number(row.maxResistance));
            }
            if (row.zoneHR >= 0) {
                stream.writeAttribute(QStringLiteral("zonehr"), QString::number(row.zoneHR));
            }
            if (row.HRmin >= 0) {
                stream.writeAttribute(QStringLiteral("hrmin"), QString::number(row.HRmin));
            }
            if (row.HRmax >= 0) {
                stream.writeAttribute(QStringLiteral("hrmax"), QString::number(row.HRmax));
            }
            if (row.loopTimeHR >= 0) {
                stream.writeAttribute(QStringLiteral("looptimehr"), QString::number(row.loopTimeHR));
            }
            stream.writeEndElement();
        }
        stream.writeEndElement();
        stream.writeEndDocument();
        return true;
    } else

        return false;
}

bool trainprogram::hasTargetPower(const QString &filename) {
    QFile file(filename);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open training program file:" << filename;
        return false;
    }
    
    QXmlStreamReader reader(&file);
    
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.name() == "row") {
            QXmlStreamAttributes attributes = reader.attributes();
            if (attributes.hasAttribute("power")) {
                QString powerStr = attributes.value("power").toString();
                bool ok;
                int power = powerStr.toInt(&ok);
                if (ok && power > 0) {
                    qDebug() << "Found target power > 0 in training program:" << power;
                    file.close();
                    return true;
                }
            }
        }
    }
    
    if (reader.hasError()) {
        qDebug() << "Error reading training program XML:" << reader.errorString();
    }
    
    file.close();
    return false;
}

void trainprogram::save(const QString &filename) { saveXML(filename, rows); }

trainprogram *trainprogram::load(const QString &filename, bluetooth *b, QString Extension) {
    if (!Extension.toUpper().compare(QStringLiteral("ZWO"))
#ifdef Q_OS_ANDROID
            || filename.toUpper().contains(".ZWO")
#endif
            ) {

        QString description = "";
        QString tags = "";
        return new trainprogram(zwiftworkout::load(filename, &description, &tags), b, &description, &tags);
    } else {

        BLUETOOTH_TYPE dtype = BLUETOOTH_TYPE::BIKE;
        if(b && b->device())
            dtype = b->device()->deviceType();
        return new trainprogram(loadXML(filename, dtype), b);
    }
}

QList<trainrow> trainprogram::loadXML(const QString &filename, BLUETOOTH_TYPE device_type) {
    QList<trainrow> list;
    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    QXmlStreamReader stream(&input);

    QList<trainrow> repeatRows;
    int repeatTimes = 0;
    bool insideRepeat = false;

    while (!stream.atEnd()) {
        stream.readNext();

        // Handle repeat tag start
        if (stream.isStartElement() && stream.name() == "repeat") {
            insideRepeat = true;
            repeatRows.clear();
            QXmlStreamAttributes attrs = stream.attributes();
            if (attrs.hasAttribute("times")) {
                repeatTimes = attrs.value("times").toInt();
            }
            continue;
        }

        // Handle repeat tag end
        if (stream.isEndElement() && stream.name() == "repeat") {
            insideRepeat = false;
            for (int i = 0; i < repeatTimes; i++) {
                list.append(repeatRows);
            }
            continue;
        }

        trainrow row;
        QXmlStreamAttributes atts = stream.attributes();
        bool ramp = false;
        if (!atts.isEmpty()) {
            if (atts.hasAttribute(QStringLiteral("duration"))) {
                row.duration = QTime::fromString(atts.value(QStringLiteral("duration")).toString(), QStringLiteral("hh:mm:ss"));
            }
            if (atts.hasAttribute(QStringLiteral("distance"))) {
                row.distance = atts.value(QStringLiteral("distance")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("speed"))) {
                row.speed = atts.value(QStringLiteral("speed")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("minspeed"))) {
                row.minSpeed = atts.value(QStringLiteral("minspeed")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("fanspeed"))) {
                row.fanspeed = atts.value(QStringLiteral("fanspeed")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("inclination"))) {
                row.inclination = atts.value(QStringLiteral("inclination")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("resistance"))) {
                row.resistance = atts.value(QStringLiteral("resistance")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("lower_resistance"))) {
                row.lower_resistance = atts.value(QStringLiteral("lower_resistance")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("mets"))) {
                row.mets = atts.value(QStringLiteral("mets")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("latitude"))) {
                row.latitude = atts.value(QStringLiteral("latitude")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("longitude"))) {
                row.longitude = atts.value(QStringLiteral("longitude")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("altitude"))) {
                row.altitude = atts.value(QStringLiteral("altitude")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("azimuth"))) {
                row.azimuth = atts.value(QStringLiteral("azimuth")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("upper_resistance"))) {
                row.upper_resistance = atts.value(QStringLiteral("upper_resistance")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("requested_peloton_resistance"))) {
                row.requested_peloton_resistance = atts.value(QStringLiteral("requested_peloton_resistance")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("lower_requested_peloton_resistance"))) {
                row.lower_requested_peloton_resistance = atts.value(QStringLiteral("lower_requested_peloton_resistance")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("upper_requested_peloton_resistance"))) {
                row.upper_requested_peloton_resistance = atts.value(QStringLiteral("upper_requested_peloton_resistance")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("pace_intensity"))) {
                row.pace_intensity = atts.value(QStringLiteral("pace_intensity")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("cadence"))) {
                row.cadence = atts.value(QStringLiteral("cadence")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("lower_cadence"))) {
                row.lower_cadence = atts.value(QStringLiteral("lower_cadence")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("upper_cadence"))) {
                row.upper_cadence = atts.value(QStringLiteral("upper_cadence")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("power"))) {
                row.power = atts.value(QStringLiteral("power")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("maxspeed"))) {
                row.maxSpeed = atts.value(QStringLiteral("maxspeed")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("maxresistance"))) {
                row.maxResistance = atts.value(QStringLiteral("maxresistance")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("zonehr"))) {
                row.zoneHR = atts.value(QStringLiteral("zonehr")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("hrmin"))) {
                row.HRmin = atts.value(QStringLiteral("hrmin")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("hrmax"))) {
                row.HRmax = atts.value(QStringLiteral("hrmax")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("looptimehr"))) {
                row.loopTimeHR = atts.value(QStringLiteral("looptimehr")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("forcespeed"))) {
                row.forcespeed = atts.value(QStringLiteral("forcespeed")).toInt() ? true : false;
            }
            if (atts.hasAttribute(QStringLiteral("powerzone"))) {
                QSettings settings;
                if(device_type == TREADMILL) {
                    row.power = atts.value(QStringLiteral("powerzone")).toDouble() *
                                settings.value(QZSettings::ftp_run, QZSettings::default_ftp_run).toDouble();
                } else {
                    row.power = atts.value(QStringLiteral("powerzone")).toDouble() *
                                settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                }
            }
            if (atts.hasAttribute(QStringLiteral("speedfrom")) && atts.hasAttribute(QStringLiteral("speedto")) &&
                atts.hasAttribute(QStringLiteral("duration"))) {
                double speedFrom = atts.value(QStringLiteral("speedfrom")).toDouble();
                double speedTo = atts.value(QStringLiteral("speedto")).toDouble();
                QTime duration = QTime::fromString(atts.value(QStringLiteral("duration")).toString(), QStringLiteral("hh:mm:ss"));
                int durationS = duration.hour() * 3600 + duration.minute() * 60 + duration.second();
                int speedDelta = (fabs(speedTo - speedFrom) / 0.1);    // 0.1 min step for speed
                int durationStep;
                double speedStep;
                int spareSeconds;
                if(speedDelta <= durationS) {
                    durationStep = durationS / speedDelta;
                    speedStep = 0.1;
                    spareSeconds = durationS % speedDelta;
                } else {
                    durationStep = 1;
                    speedStep = fabs(speedTo - speedFrom) / (durationS - 1);
                    speedDelta = durationS - 1;
                    spareSeconds = 0;
                }
                int spareSum = 0;
                for (int i = 0; i < speedDelta; i++) {
                    trainrow rowI(row);
                    int spare = 0;
                    if (spareSeconds)
                        spare = (i % spareSeconds == 0 && i > 0) ? 1 : 0;
                    spareSum += spare;
                    if (i == speedDelta && spareSum < spareSeconds) {
                        spare += (spareSeconds - spareSum) - durationStep;
                        spareSum = spareSeconds;
                    }
                    rowI.duration = QTime(0, 0, 0, 0).addSecs(durationStep + spare);
                    rowI.rampElapsed = QTime(0, 0, 0, 0).addSecs((durationStep * i) + spareSum);
                    rowI.rampDuration = QTime(0, 0, 0, 0).addSecs(durationS - (durationStep * i) - spareSum - durationStep + spare);
                    rowI.forcespeed = 1;
                    if (speedFrom < speedTo) {
                        rowI.speed = speedFrom + (speedStep * i);
                    } else {
                        rowI.speed = speedFrom - (speedStep * i);
                    }
                    qDebug() << "TrainRow" << rowI.toString();
                    if (insideRepeat) {
                        repeatRows.append(rowI);
                    } else {
                        list.append(rowI);
                    }
                }
                ramp = true;
            }
            if (atts.hasAttribute(QStringLiteral("powerzonefrom")) && atts.hasAttribute(QStringLiteral("powerzoneto")) &&
                atts.hasAttribute(QStringLiteral("duration"))) {
                QSettings settings;
                double speedFrom = atts.value(QStringLiteral("powerzonefrom")).toDouble();
                double speedTo = atts.value(QStringLiteral("powerzoneto")).toDouble();
                QTime duration = QTime::fromString(atts.value(QStringLiteral("duration")).toString(), QStringLiteral("hh:mm:ss"));
                int durationS = duration.hour() * 3600 + duration.minute() * 60 + duration.second();
                int speedDelta = (fabs(speedTo - speedFrom) / 0.01);    // 0.01 min step for speed
                int durationStep;
                double speedStep;
                int spareSeconds;
                if(speedDelta == 0)
                    speedDelta = 1;
                if(speedDelta <= durationS) {
                    durationStep = durationS / speedDelta;
                    speedStep = 0.01;
                    spareSeconds = durationS % speedDelta;
                } else {
                    durationStep = 1;
                    speedStep = fabs(speedTo - speedFrom) / (durationS - 1);
                    speedDelta = durationS - 1;
                    spareSeconds = 0;
                }
                int spareSum = 0;
                for (int i = 0; i < speedDelta; i++) {
                    trainrow rowI(row);
                    int spare = 0;
                    if (spareSeconds)
                        spare = (i % spareSeconds == 0 && i > 0) ? 1 : 0;
                    spareSum += spare;
                    if (i == speedDelta && spareSum < spareSeconds) {
                        spare += (spareSeconds - spareSum) - durationStep;
                        spareSum = spareSeconds;
                    }
                    rowI.duration = QTime(0, 0, 0, 0).addSecs(durationStep + spare);
                    rowI.rampElapsed = QTime(0, 0, 0, 0).addSecs((durationStep * i) + spareSum);
                    rowI.rampDuration = QTime(0, 0, 0, 0).addSecs(durationS - (durationStep * i) - spareSum - durationStep + spare);
                    rowI.forcespeed = 1;
                    if (speedFrom < speedTo) {
                        if(device_type == TREADMILL) {
                            rowI.power = (speedFrom + (speedStep * i)) *
                                         settings.value(QZSettings::ftp_run, QZSettings::default_ftp_run).toDouble();
                        } else {
                            rowI.power = (speedFrom + (speedStep * i)) *
                                         settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                        }
                    } else {
                        if(device_type == TREADMILL) {
                            rowI.power = (speedFrom - (speedStep * i)) *
                                         settings.value(QZSettings::ftp_run, QZSettings::default_ftp_run).toDouble();
                        } else {
                            rowI.power = (speedFrom - (speedStep * i)) *
                                         settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
                        }
                    }
                    qDebug() << "TrainRow" << rowI.toString();
                    if (insideRepeat) {
                        repeatRows.append(rowI);
                    } else {
                        list.append(rowI);
                    }
                }
                ramp = true;
            }

            if(!ramp) {
                if (insideRepeat) {
                    repeatRows.append(row);
                } else {
                    list.append(row);
                }
                qDebug() << row.toString();
            }
        }
    }
    return list;
}

QTime trainprogram::totalElapsedTime() { return QTime(0, 0, ticks); }

trainrow trainprogram::currentRow() {
    if (started && !rows.isEmpty()) {

        return rows.at(currentStep);
    }
    return trainrow();
}

trainrow trainprogram::getRowFromCurrent(uint32_t offset) {
    if (started && !rows.isEmpty() && (currentStep + offset) < (uint32_t)rows.length()) {
        return rows.at(currentStep + offset);
    }
    return trainrow();
}

double trainprogram::currentTargetMets() {
    if (currentRow().mets)
        return currentRow().mets;
    else
        return 0;
}

QTime trainprogram::currentRowElapsedTime() {
    uint32_t calculatedLine;
    uint32_t calculatedElapsedTime = 0;

    if (rows.length() == 0)
        return QTime(0, 0, 0);

    for (calculatedLine = 0; calculatedLine < static_cast<uint32_t>(rows.length()); calculatedLine++) {

        uint32_t currentLine = calculateTimeForRow(calculatedLine);
        calculatedElapsedTime += currentLine;
        uint32_t rampElapsed = 0;

        if (calculatedElapsedTime > static_cast<uint32_t>(ticks)) {
            if (rows.at(calculatedLine).rampElapsed != QTime(0, 0, 0)) {
                rampElapsed = (rows.at(calculatedLine).rampElapsed.second() +
                               (rows.at(calculatedLine).rampElapsed.minute() * 60) +
                               (rows.at(calculatedLine).rampElapsed.hour() * 3600));
            }
            return QTime(0, 0, 0).addSecs(rampElapsed + ticks - (calculatedElapsedTime - currentLine));
        }
    }
    return QTime(0, 0, 0);
}

QTime trainprogram::currentRowRemainingTime() {
    uint32_t calculatedLine;
    uint32_t calculatedElapsedTime = 0;

    if (rows.length() == 0)
        return QTime(0, 0, 0);

    if (currentStep < rows.length() && rows.at(currentStep).distance > 0 && bluetoothManager &&
        bluetoothManager->device()) {
        double speed = bluetoothManager->device()->currentSpeed().value();
        double distance = rows.at(currentStep).distance;
        distance -= currentStepDistance;
        int seconds = (distance / speed) * 3600.0;
        int hours = seconds / 3600;
        return QTime(hours, (seconds / 60) - (hours * 60), seconds % 60);
    } else {
        for (calculatedLine = 0; calculatedLine < static_cast<uint32_t>(rows.length()); calculatedLine++) {

            uint32_t currentLine = calculateTimeForRow(calculatedLine);
            calculatedElapsedTime += currentLine;

            if (calculatedElapsedTime > static_cast<uint32_t>(ticks)) {
                if (rows.at(calculatedLine).rampDuration != QTime(0, 0, 0)) {
                    calculatedElapsedTime += ((rows.at(calculatedLine).rampDuration.second() +
                                               (rows.at(calculatedLine).rampDuration.minute() * 60) +
                                               (rows.at(calculatedLine).rampDuration.hour() * 3600))) -
                                             1;
                }
                int seconds = calculatedElapsedTime - ticks;
                int hours = seconds / 3600;
                return QTime(hours, (seconds / 60) - (hours * 60), seconds % 60);
            }
        }
    }
    return QTime(0, 0, 0);
}

QTime trainprogram::remainingTime() {
    uint32_t calculatedLine;
    uint32_t calculatedTotalTime = 0;

    if (rows.length() == 0)
        return QTime(0, 0, 0);

    for (calculatedLine = 0; calculatedLine < static_cast<uint32_t>(rows.length()); calculatedLine++) {
        calculatedTotalTime += calculateTimeForRow(calculatedLine);
    }
    return QTime(0, 0, 0).addSecs(calculatedTotalTime - ticks);
}

QTime trainprogram::duration() {

    QTime total(0, 0, 0, 0);
    for (const trainrow &row : qAsConst(rows)) {
        total = total.addSecs((row.duration.hour() * 3600) + (row.duration.minute() * 60) + row.duration.second());
    }
    return total;
}

double trainprogram::totalDistance() {

    double distance = 0;
    for (const trainrow &row : qAsConst(rows)) {
        if (row.duration.hour() || row.duration.minute() || row.duration.second()) {
            if (!row.forcespeed) {

                return -1;
            }
            distance += ((row.duration.hour() * 3600) + (row.duration.minute() * 60) + row.duration.second()) *
                        (row.speed / 3600);
        }
    }
    return distance;
}
