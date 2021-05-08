#include "trainprogram.h"
#include <QFile>
#include <QtXml/QtXml>

trainprogram::trainprogram(QList<trainrow> rows, bluetooth* b)
{
    this->bluetoothManager = b;
    this->rows = rows;
    this->loadedRows = rows;
    connect(&timer, SIGNAL(timeout()), this, SLOT(scheduler()));
    timer.setInterval(1000);
    timer.start();
}

uint32_t trainprogram::calculateTimeForRow(int32_t row)
{
    if(row >= rows.length()) return 0;

    return (rows[row].duration.second() +
            (rows[row].duration.minute() * 60) +
            (rows[row].duration.hour() * 3600));
}

void trainprogram::scheduler()
{
    QSettings settings;
    if(
            rows.count() == 0 ||
            started == false ||
            enabled == false ||
            bluetoothManager->device() == nullptr ||
            (bluetoothManager->device()->currentSpeed().value() <= 0 && !settings.value("continuous_moving", false).toBool()) ||
            bluetoothManager->device()->isPaused()
            )
    {
        return;
    }

    ticks++;

    // entry point
    if(ticks == 1 && currentStep == 0)
    {
        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            if(rows[0].forcespeed && rows[0].speed)
            {
                qDebug() << "trainprogram change speed" + QString::number(rows[0].speed);
                emit changeSpeedAndInclination(rows[0].speed, rows[0].inclination);
            }
            else
            {
                qDebug() << "trainprogram change inclination" + QString::number(rows[0].inclination);
                emit changeInclination(rows[0].inclination);
            }
        }
        else
        {
            qDebug() << "trainprogram change resistance" + QString::number(rows[0].resistance);
            emit changeResistance(rows[0].resistance);

            if(rows[0].cadence != -1)
            {
                qDebug() << "trainprogram change cadence" + QString::number(rows[0].cadence);
                emit changeCadence(rows[0].cadence);
            }

            if(rows[0].requested_peloton_resistance != -1)
            {
                qDebug() << "trainprogram change requested peloton resistance" + QString::number(rows[0].requested_peloton_resistance);
                emit changeRequestedPelotonResistance(rows[0].requested_peloton_resistance);
            }
        }

        if(rows[0].fanspeed != -1)
        {
            qDebug() << "trainprogram change fanspeed" + QString::number(rows[0].fanspeed);
            emit changeFanSpeed(rows[0].fanspeed);
        }
    }

    uint32_t currentRowLen = calculateTimeForRow(currentStep);

    qDebug() << "trainprogram elapsed " + QString::number(ticks) + "current row len" + QString::number(currentRowLen);

    uint32_t calculatedLine;
    uint32_t calculatedElapsedTime = 0;
    for(calculatedLine = 0; calculatedLine < rows.length(); calculatedLine++)
    {
        calculatedElapsedTime += calculateTimeForRow(calculatedLine);

        if(calculatedElapsedTime > ticks)
            break;
    }

    if(calculatedLine != currentStep)
    {
        if(calculateTimeForRow(calculatedLine))
        {
            currentStep = calculatedLine;
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
            {
                if(rows[currentStep].forcespeed && rows[currentStep].speed)
                {
                    qDebug() << "trainprogram change speed" + QString::number(rows[currentStep].speed);
                    emit changeSpeedAndInclination(rows[currentStep].speed, rows[currentStep].inclination);
                }
                qDebug() << "trainprogram change inclination" + QString::number(rows[currentStep].inclination);
                emit changeInclination(rows[currentStep].inclination);
            }
            else
            {
                qDebug() << "trainprogram change resistance" + QString::number(rows[currentStep].resistance);
                emit changeResistance(rows[currentStep].resistance);

                if(rows[currentStep].cadence != -1)
                {
                    qDebug() << "trainprogram change cadence" + QString::number(rows[currentStep].cadence);
                    emit changeCadence(rows[currentStep].cadence);
                }

                if(rows[currentStep].requested_peloton_resistance != -1)
                {
                    qDebug() << "trainprogram change requested peloton resistance" + QString::number(rows[currentStep].requested_peloton_resistance);
                    emit changeRequestedPelotonResistance(rows[currentStep].requested_peloton_resistance);
                }
            }

            if(rows[currentStep].fanspeed != -1)
            {
                qDebug() << "trainprogram change fanspeed" + QString::number(rows[currentStep].fanspeed);
                emit changeFanSpeed(rows[currentStep].fanspeed);
            }
        }
        else
        {
            qDebug() << "trainprogram ends!";
            started = false;
            emit stop();
        }
    }
}

void trainprogram::increaseElapsedTime(uint32_t i)
{
    offset += i;
    ticks += i;
}

void trainprogram::decreaseElapsedTime(uint32_t i)
{
    offset -= i;
    ticks -= i;
}

void trainprogram::onTapeStarted()
{
    started = true;
}

void trainprogram::restart()
{
    ticks = 0;
    offset = 0;
    currentStep = 0;
    started = true;
}

bool trainprogram::saveXML(QString filename, const QList<trainrow>& rows) {
    QFile output(filename);
    if (rows.size() && output.open(QIODevice::WriteOnly)) {
        QXmlStreamWriter stream(&output);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();
        stream.writeStartElement("rows");
        foreach (trainrow row, rows) {
            stream.writeStartElement("row");
            stream.writeAttribute("duration", row.duration.toString());
            if (row.speed>=0)
                stream.writeAttribute("speed", QString::number(row.speed));
            if (row.inclination>=-50)
                stream.writeAttribute("inclination", QString::number(row.inclination));
            if (row.resistance>=0)
                stream.writeAttribute("resistance", QString::number(row.resistance));
            if (row.requested_peloton_resistance>=0)
                stream.writeAttribute("requested_peloton_resistance", QString::number(row.requested_peloton_resistance));
            if (row.cadence>=0)
                stream.writeAttribute("cadence", QString::number(row.cadence));
            stream.writeAttribute("forcespeed", row.forcespeed?"1":"0");
            if (row.fanspeed>=0)
                stream.writeAttribute("fanspeed", QString::number(row.fanspeed));
            if (row.maxSpeed>=0)
                stream.writeAttribute("maxspeed", QString::number(row.maxSpeed));
            if (row.zoneHR>=0)
                stream.writeAttribute("zonehr", QString::number(row.zoneHR));
            if (row.loopTimeHR>=0)
                stream.writeAttribute("looptimehr", QString::number(row.loopTimeHR));
            stream.writeEndElement();
        }
        stream.writeEndElement();
        stream.writeEndDocument();
        return true;
    }
    else
        return false;
}

void trainprogram::save(QString filename)
{
    saveXML(filename, rows);
}

trainprogram* trainprogram::load(QString filename, bluetooth* b)
{
    return new trainprogram(loadXML(filename), b);
}

QList<trainrow> trainprogram::loadXML(QString filename)
{
    QList<trainrow> list;
    QFile input(filename);
    input.open(QIODevice::ReadOnly);
    QXmlStreamReader stream(&input);
    while(!stream.atEnd())
    {
        stream.readNext();
        trainrow row;
        QXmlStreamAttributes atts = stream.attributes();
        if(atts.length())
        {
            row.duration = QTime::fromString(atts.value("duration").toString(), "hh:mm:ss");
            if(atts.hasAttribute("speed"))
                row.speed = atts.value("speed").toDouble();
            if(atts.hasAttribute("fanspeed"))
                row.fanspeed = atts.value("fanspeed").toDouble();
            if(atts.hasAttribute("inclination"))
                row.inclination = atts.value("inclination").toDouble();
            if(atts.hasAttribute("resistance"))
                row.resistance = atts.value("resistance").toInt();
            if(atts.hasAttribute("requested_peloton_resistance"))
                row.requested_peloton_resistance = atts.value("requested_peloton_resistance").toInt();
            if(atts.hasAttribute("cadence"))
                row.cadence = atts.value("cadence").toInt();
            if(atts.hasAttribute("maxspeed"))
                row.maxSpeed = atts.value("maxspeed").toInt();
            if(atts.hasAttribute("zonehr"))
                row.zoneHR = atts.value("zonehr").toInt();
            if(atts.hasAttribute("looptimehr"))
                row.loopTimeHR = atts.value("looptimehr").toInt();
            if(atts.hasAttribute("forcespeed"))
                row.forcespeed = atts.value("forcespeed").toInt()?true:false ;
            list.append(row);
        }
    }
    return list;
}

QTime trainprogram::totalElapsedTime()
{
    return QTime(0,0,ticks);
}

trainrow trainprogram::currentRow()
{
    if(started && rows.length())
    {
        return rows.at(currentStep);
    }
    return trainrow();
}

QTime trainprogram::currentRowElapsedTime()
{
    uint32_t calculatedLine;
    uint32_t calculatedElapsedTime = 0;

    if(rows.length() == 0) return QTime(0,0,0);

    for(calculatedLine = 0; calculatedLine < rows.length(); calculatedLine++)
    {
        uint32_t currentLine = calculateTimeForRow(calculatedLine);
        calculatedElapsedTime += currentLine;

        if(calculatedElapsedTime > ticks)
            return QTime(0,0,calculatedElapsedTime - currentLine + ticks);
    }
    return QTime(0,0,0);
}

QTime trainprogram::duration()
{
    QTime total(0,0,0,0);
    foreach (trainrow row, rows) {
        total = total.addSecs((row.duration.hour() * 3600) + (row.duration.minute() * 60) + row.duration.second());
    }
    return total;
}

double trainprogram::totalDistance()
{
    double distance = 0;
    foreach (trainrow row, rows) {
        if(row.duration.hour() || row.duration.minute() || row.duration.second())
        {
            if(!row.forcespeed)
            {
                return -1;
            }
            distance += ((row.duration.hour() * 3600) + (row.duration.minute() * 60) + row.duration.second()) * (row.speed / 3600);
        }
    }
    return distance;
}
