#ifndef NETSERIAL_H
#define NETSERIAL_H

#include "serialhandler.h"
#include <QString>
#include <QTcpSocket>

class NetSerial : public SerialHandler {
  public:
    NetSerial(QString deviceFilename);
    ~NetSerial() override;

    int openPort() override;
    int closePort() override;
    int dataAvailable() override;
    int rawWrite(uint8_t *bytes, int size) override;
    int rawRead(uint8_t bytes[], int size, bool line = false) override;

    bool isOpen() const override;
    void setTimeout(int timeout) override;
    void setEndChar(uint8_t endChar) override;
    void setDevice(const QString &devname) override;

  private:
    QString deviceFilename;
    QString serverAddress;
    quint16 serverPort;
    QTcpSocket *socket;
    int _timeout = 1000; // Timeout in milliseconds
    uint8_t endChar = '\n';
    bool parseDeviceFilename(const QString &filename);
};

#endif // NETSERIAL_H