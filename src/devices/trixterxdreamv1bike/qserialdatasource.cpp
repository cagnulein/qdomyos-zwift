#include "qserialdatasource.h"
#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>

QStringList qserialdatasource::get_availablePorts() {
    QStringList result;

    auto ports = QSerialPortInfo::availablePorts();
    for(const auto &port : ports) {
        QString portName = port.portName();

#if defined(Q_OS_LINUX)
        if(!portName.startsWith("ttyUSB"))
        {
            qDebug() << "Skipping port: " << portName << " because it doesn't start with ttyUSB";
            continue;
        }
#endif
        result.push_back(portName);
        qDebug() << "Found portName:" << portName
                 << "," << "description:" << port.description()
                 << "," << "vender identifier:" << port.vendorIdentifier()
                 << "," << "manufacturer:" << port.manufacturer()
                 << "," << "product identifier:" << port.productIdentifier()
                 << "," << "system location:" << port.systemLocation()
                 << "," << "isBusy:" << port.isBusy()
                 << "," << "isNull:" << port.isNull()
                 << "," << "serialNumber:" << port.serialNumber();
    }
    return result;
}

qserialdatasource::qserialdatasource(QObject *parent) : serialdatasource()
{
    this->serial = new QSerialPort(parent);
}

bool qserialdatasource::open(const QString& portName) {
    this->serial->setPortName(portName);
    this->serial->setBaudRate(QSerialPort::Baud115200);
    this->serial->setDataBits(QSerialPort::Data8);
    this->serial->setStopBits(QSerialPort::OneStop);
    this->serial->setFlowControl(QSerialPort::NoFlowControl);
    this->serial->setParity(QSerialPort::NoParity);
    this->serial->setReadBufferSize(4096);

    return this->serial->open(QIODevice::ReadWrite);
}

qint64 qserialdatasource::write(const QByteArray &data) {
    return this->serial->write(data);
}

void qserialdatasource::flush() {
    this->serial->flush();
}

bool qserialdatasource::waitForReadyRead() {
    return this->serial->waitForReadyRead(1);
}

QByteArray qserialdatasource::readAll() {
    return this->serial->readAll();
}

qint64 qserialdatasource::readBufferSize() {
    return this->serial->readBufferSize();
}

QString qserialdatasource::error() {
    return serial->parent()->tr("%1").arg(this->serial->error());
}

void qserialdatasource::close() {
    this->serial->close();
}

qserialdatasource::~qserialdatasource() {
    if(this->serial!=nullptr) {
        delete this->serial;
        this->serial = nullptr;
    }
}

#endif
