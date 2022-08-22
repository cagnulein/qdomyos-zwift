#include "trainprogram.h"
#include "zwiftworkout.h"
#include <QFile>
#include <QMutexLocker>
#include <QtXml/QtXml>
#include <chrono>

using namespace std::chrono_literals;

trainprogram::trainprogram(const QList<trainrow> &rows, bluetooth *b, QString *description, QString *tags) {
    this->bluetoothManager = b;
    this->rows = rows;
    this->loadedRows = rows;
    if (description)
        this->description = *description;
    if (tags)
        this->tags = *tags;
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
    rv += QStringLiteral(" cadence = %1").arg(cadence);
    rv += QStringLiteral(" lower_cadence = %1").arg(lower_cadence);
    rv += QStringLiteral(" average_cadence = %1").arg(average_cadence); // used for peloton
    rv += QStringLiteral(" upper_cadence = %1").arg(upper_cadence);
    rv += QStringLiteral(" forcespeed = %1").arg(forcespeed);
    rv += QStringLiteral(" loopTimeHR = %1").arg(loopTimeHR);
    rv += QStringLiteral(" zoneHR = %1").arg(zoneHR);
    rv += QStringLiteral(" maxSpeed = %1").arg(maxSpeed);
    rv += QStringLiteral(" power = %1").arg(power);
    rv += QStringLiteral(" mets = %1").arg(mets);
    rv += QStringLiteral(" latitude = %1").arg(latitude);
    rv += QStringLiteral(" longitude = %1").arg(longitude);
    rv += QStringLiteral(" altitude = %1").arg(altitude);
    rv += QStringLiteral(" azimuth = %1").arg(azimuth);
    return rv;
}

uint32_t trainprogram::calculateTimeForRow(int32_t row) {
    if (row >= rows.length())
        return 0;

    if (rows.at(row).distance == -1)
        return (rows.at(row).duration.second() + (rows.at(row).duration.minute() * 60) +
                (rows.at(row).duration.hour() * 3600));
    else
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

double trainprogram::avgInclinationNext100Meters() {
    int c = currentStep;
    double km = 0;
    double avg = 0;
    int sum = 0;

    while (1) {
        if (c < rows.length()) {
            if (km > 0.1) {
                return avg / (double)sum;
            }
            if (c == currentStep)
                km += (rows.at(c).distance - currentStepDistance);
            else
                km += (rows.at(c).distance);
            avg += rows.at(c).inclination;
            sum++;

        } else {
            return avg / (double)sum;
        }
        c++;
    }
    return avg / (double)sum;
}

double trainprogram::avgAzimuthNext300Meters() {
    int c = currentStep;
    double km = 0;
    double sinTotal = 0;
    double cosTotal = 0;

    if (rows.at(c).latitude != 0 || rows.at(c).longitude != 0) {
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

void trainprogram::scheduler() {

    QMutexLocker(&this->schedulerMutex);
    QSettings settings;
    if (rows.count() == 0 || started == false || enabled == false || bluetoothManager->device() == nullptr ||
        (bluetoothManager->device()->currentSpeed().value() <= 0 &&
         !settings.value(QStringLiteral("continuous_moving"), false).toBool()) ||
        bluetoothManager->device()->isPaused()) {

        return;
    }

    ticks++;

    // entry point
    if (ticks == 1 && currentStep == 0) {
        currentStepDistance = 0;
        lastOdometer = bluetoothManager->device()->odometer();
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            if (rows.at(0).forcespeed && rows.at(0).speed) {
                qDebug() << QStringLiteral("trainprogram change speed") + QString::number(rows.at(0).speed);
                emit changeSpeed(rows.at(0).speed);
            }
            if (rows.at(0).inclination != -200) {
                double inc;
                if (!isnan(rows.at(0).latitude) && !isnan(rows.at(0).longitude)) {
                    inc = avgInclinationNext100Meters();
                } else {
                    inc = rows.at(0).inclination;
                }
                qDebug() << QStringLiteral("trainprogram change inclination") + QString::number(inc);
                emit changeInclination(inc, inc);
                emit changeNextInclination300Meters(inclinationNext300Meters());
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

            if (rows.at(0).inclination != -200 && bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                // this should be converted in a signal as all the other signals...
                double bikeResistanceOffset = settings.value(QStringLiteral("bike_resistance_offset"), 0).toInt();
                double bikeResistanceGain = settings.value(QStringLiteral("bike_resistance_gain_f"), 1).toDouble();

                double inc;
                if (!isnan(rows.at(0).latitude) && !isnan(rows.at(0).longitude)) {
                    inc = avgInclinationNext100Meters();
                } else {
                    inc = rows.at(0).inclination;
                }
                bluetoothManager->device()->changeResistance((resistance_t)(round(inc * bikeResistanceGain)) +
                                                             bikeResistanceOffset + 1); // resistance start from 1)
                if (!((bike *)bluetoothManager->device())->inclinationAvailableByHardware())
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
            qDebug() << QStringLiteral("trainprogram change GEO position") + QString::number(rows.at(0).latitude) +
                            " " + QString::number(rows.at(0).longitude) + " " + QString::number(rows.at(0).altitude) +
                            " " + QString::number(rows.at(0).azimuth);
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

        if (calculatedElapsedTime > static_cast<uint32_t>(ticks)) {
            break;
        }
    }

    bool distanceEvaluation = false;
    int sameIteration = 0;

    do {

        currentStepDistance += (bluetoothManager->device()->odometer() - lastOdometer);
        lastOdometer = bluetoothManager->device()->odometer();
        bool distanceStep = (rows.at(currentStep).distance > 0);
        distanceEvaluation = (distanceStep && currentStepDistance >= rows.at(currentStep).distance);
        qDebug() << QStringLiteral("currentStepDistance") << currentStepDistance << QStringLiteral("distanceStep")
                 << distanceStep << QStringLiteral("distanceEvaluation") << distanceEvaluation
                 << QStringLiteral("rows distance") << rows.at(currentStep).distance << QStringLiteral("same iteration")
                 << sameIteration;

        if ((calculatedLine != currentStep && !distanceStep) || distanceEvaluation) {
            if (calculateTimeForRow(calculatedLine) || calculateDistanceForRow(currentStep + 1) > 0) {

                lastOdometer -= (currentStepDistance - rows.at(currentStep).distance);

                if (!distanceStep)
                    currentStep = calculatedLine;
                else
                    currentStep++;

                currentStepDistance = 0;
                if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                    if (rows.at(currentStep).forcespeed && rows.at(currentStep).speed) {
                        qDebug() << QStringLiteral("trainprogram change speed ") +
                                        QString::number(rows.at(currentStep).speed);
                        emit changeSpeed(rows.at(currentStep).speed);
                    }
                    if (rows.at(currentStep).inclination != -200) {
                        double inc;
                        if (!isnan(rows.at(currentStep).latitude) && !isnan(rows.at(currentStep).longitude)) {
                            inc = avgInclinationNext100Meters();
                        } else {
                            inc = rows.at(currentStep).inclination;
                        }
                        qDebug() << QStringLiteral("trainprogram change inclination") + QString::number(inc);
                        emit changeInclination(inc, inc);
                        emit changeNextInclination300Meters(inclinationNext300Meters());
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
                        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                        // this should be converted in a signal as all the other signals...
                        double bikeResistanceOffset =
                            settings.value(QStringLiteral("bike_resistance_offset"), 0).toInt();
                        double bikeResistanceGain =
                            settings.value(QStringLiteral("bike_resistance_gain_f"), 1).toDouble();

                        double inc;
                        if (!isnan(rows.at(currentStep).latitude) && !isnan(rows.at(currentStep).longitude)) {
                            inc = avgInclinationNext100Meters();
                        } else {
                            inc = rows.at(currentStep).inclination;
                        }
                        bluetoothManager->device()->changeResistance((resistance_t)(round(inc * bikeResistanceGain)) +
                                                                     bikeResistanceOffset +
                                                                     1); // resistance start from 1)
                        if (!((bike *)bluetoothManager->device())->inclinationAvailableByHardware())
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
                    qDebug() << QStringLiteral("trainprogram change GEO position") +
                                    QString::number(rows.at(currentStep).latitude) + " " +
                                    QString::number(rows.at(currentStep).longitude) + " " +
                                    QString::number(rows.at(currentStep).altitude) + " " +
                                    QString::number(rows.at(currentStep).distance) + " " +
                                    QString::number(rows.at(currentStep).azimuth);

                    QGeoCoordinate p;
                    p.setLatitude(rows.at(currentStep).latitude);
                    p.setLongitude(rows.at(currentStep).longitude);
                    p.setAltitude(rows.at(currentStep).altitude);
                    // qDebug() << c << rows.at(currentStep+1).latitude << rows.at(currentStep + 1).longitude <<
                    /*QGeoCoordinate c;
                    c.setLatitude(rows.at(currentStep+1).latitude);
                    c.setLongitude(rows.at(currentStep+1).longitude);
                    c.setAltitude(rows.at(currentStep+1).altitude);
                    qDebug() << "distance" << p.distanceTo(c) << rows.at(currentStep).distance;*/

                    if (bluetoothManager->device()->odometer() - lastOdometer > 0)
                        p = p.atDistanceAndAzimuth((bluetoothManager->device()->odometer() - lastOdometer),
                                                   rows.at(currentStep).azimuth);
                    emit changeGeoPosition(p, rows.at(currentStep).azimuth, avgAzimuthNext300Meters());
                    emit changeTimestamp(rows.at(currentStep).rampElapsed, QTime(0, 0, 0).addSecs(ticks));
                }
            } else {
                qDebug() << QStringLiteral("trainprogram ends!");

                // circuit?
                if (!isnan(rows.first().latitude) && !isnan(rows.first().longitude) &&
                    QGeoCoordinate(rows.first().latitude, rows.first().longitude)
                            .distanceTo(bluetoothManager->device()->currentCordinate()) < 50) {
                    emit lap();
                    restart();
                } else {
                    started = false;
                    emit stop();
                    distanceEvaluation = false;
                }
            }
        } else {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {

            } else {
                if (rows.length() > currentStep && rows.at(currentStep).power != -1) {
                    qDebug() << QStringLiteral("trainprogram change power ") +
                                    QString::number(rows.at(currentStep).power);
                    emit changePower(rows.at(currentStep).power);
                }
            }

            if (rows.at(currentStep).inclination != -200 &&
                (!isnan(rows.at(currentStep).latitude) && !isnan(rows.at(currentStep).longitude))) {
                double inc = avgInclinationNext100Meters();
                double bikeResistanceOffset = settings.value(QStringLiteral("bike_resistance_offset"), 0).toInt();
                double bikeResistanceGain = settings.value(QStringLiteral("bike_resistance_gain_f"), 1).toDouble();

                if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                    bluetoothManager->device()->changeResistance((resistance_t)(round(inc * bikeResistanceGain)) +
                                                                 bikeResistanceOffset + 1); // resistance start from 1)
                    if (!((bike *)bluetoothManager->device())->inclinationAvailableByHardware())
                        bluetoothManager->device()->setInclination(inc);
                }
                qDebug() << QStringLiteral("trainprogram change inclination due to gps") + QString::number(inc);
                emit changeInclination(inc, inc);
                emit changeNextInclination300Meters(inclinationNext300Meters());
            }
        }
        sameIteration++;
    } while (distanceEvaluation);
}

void trainprogram::increaseElapsedTime(uint32_t i) {

    offset += i;
    ticks += i;
}

void trainprogram::decreaseElapsedTime(uint32_t i) {

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
            if (row.zoneHR >= 0) {
                stream.writeAttribute(QStringLiteral("zonehr"), QString::number(row.zoneHR));
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

void trainprogram::save(const QString &filename) { saveXML(filename, rows); }

trainprogram *trainprogram::load(const QString &filename, bluetooth *b) {
    if (!filename.right(3).toUpper().compare(QStringLiteral("ZWO"))) {

        QString description = "";
        QString tags = "";
        return new trainprogram(zwiftworkout::load(filename, &description, &tags), b, &description, &tags);
    } else {

        return new trainprogram(loadXML(filename), b);
    }
}

QList<trainrow> trainprogram::loadXML(const QString &filename) {

    QList<trainrow> list;
    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    QXmlStreamReader stream(&input);
    while (!stream.atEnd()) {

        stream.readNext();
        trainrow row;
        QXmlStreamAttributes atts = stream.attributes();
        if (!atts.isEmpty()) {
            row.duration =
                QTime::fromString(atts.value(QStringLiteral("duration")).toString(), QStringLiteral("hh:mm:ss"));
            if (atts.hasAttribute(QStringLiteral("distance"))) {
                row.distance = atts.value(QStringLiteral("distance")).toDouble();
            }
            if (atts.hasAttribute(QStringLiteral("speed"))) {
                row.speed = atts.value(QStringLiteral("speed")).toDouble();
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
                row.longitude = atts.value(QStringLiteral("altitude")).toDouble();
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
                row.lower_requested_peloton_resistance =
                    atts.value(QStringLiteral("lower_requested_peloton_resistance")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("upper_requested_peloton_resistance"))) {
                row.upper_requested_peloton_resistance =
                    atts.value(QStringLiteral("upper_requested_peloton_resistance")).toInt();
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
                row.maxSpeed = atts.value(QStringLiteral("maxspeed")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("zonehr"))) {
                row.zoneHR = atts.value(QStringLiteral("zonehr")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("looptimehr"))) {
                row.loopTimeHR = atts.value(QStringLiteral("looptimehr")).toInt();
            }
            if (atts.hasAttribute(QStringLiteral("forcespeed"))) {
                row.forcespeed = atts.value(QStringLiteral("forcespeed")).toInt() ? true : false;
            }

            list.append(row);
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
