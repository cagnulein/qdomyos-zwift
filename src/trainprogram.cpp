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
    elapsed = ticks;
    ticksCurrentRow++;
    elapsedCurrentRow =  ticksCurrentRow;

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

    uint32_t currentRowLen = rows[currentStep].duration.second() +
            (rows[currentStep].duration.minute() * 60) +
            (rows[currentStep].duration.hour() * 3600);

    uint32_t nextRowLen = 0;

    if(rows.count() > currentStep + 1)
        nextRowLen = rows[currentStep + 1].duration.second() +
            (rows[currentStep + 1].duration.minute() * 60) +
            (rows[currentStep + 1].duration.hour() * 3600);

    qDebug() << "trainprogram elapsed current row" + QString::number(elapsedCurrentRow) + "current row len" + QString::number(currentRowLen);

    if(elapsedCurrentRow >= currentRowLen && currentRowLen)
    {
        if(nextRowLen)
        {
            currentStep++;
            ticksCurrentRow = 0;
            elapsedCurrentRow = 0;
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

void trainprogram::onTapeStarted()
{
    started = true;
}

void trainprogram::restart()
{
    ticks = 0;
    ticksCurrentRow = 0;
    elapsed = 0;
    elapsedCurrentRow = 0;
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
            stream.writeAttribute("speed", QString::number(row.speed));
            stream.writeAttribute("inclination", QString::number(row.inclination));
            stream.writeAttribute("resistance", QString::number(row.resistance));
            stream.writeAttribute("requested_peloton_resistance", QString::number(row.requested_peloton_resistance));
            stream.writeAttribute("cadence", QString::number(row.cadence));
            stream.writeAttribute("forcespeed", row.forcespeed?"1":"0");
            stream.writeAttribute("fanspeed", QString::number(row.fanspeed));
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
            row.speed = atts.value("speed").toDouble();
            if(atts.hasAttribute("fanspeed"))
                row.fanspeed = atts.value("fanspeed").toDouble();
            else
                row.fanspeed = -1;
            row.inclination = atts.value("inclination").toDouble();
            row.resistance = atts.value("resistance").toInt();
            row.requested_peloton_resistance = atts.value("requested_peloton_resistance").toInt();
            row.cadence = atts.value("cadence").toInt();
            row.forcespeed = atts.value("forcespeed").toInt()?true:false ;
            list.append(row);
        }
    }
    return list;
}

QTime trainprogram::totalElapsedTime()
{
    return QTime(0,0,elapsed);
}

QTime trainprogram::currentRowElapsedTime()
{
    return QTime(0,0,elapsedCurrentRow);
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
