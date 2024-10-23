#ifndef QSERIALDATASOURCE_H
#define QSERIALDATASOURCE_H
#include <qglobal.h>

// Not compatible with Android and iOS.
#if !defined(Q_OS_IOS) && !defined(Q_OS_ANDROID)
#include <QObject>
#include "serialdatasource.h"

/**
 * @brief An implementation of serialdatasource that uses Qt's QSerialPort class.
 */
class qserialdatasource : public serialdatasource {
  private:
    class QSerialPort * serial;
  public:
    qserialdatasource(QObject *parent);

    QStringList get_availablePorts() override;

    bool open(const QString& portName) override;
    qint64 write(const QByteArray& data) override;
    void flush() override;
    bool waitForReadyRead() override;
    QByteArray readAll() override;
    qint64 readBufferSize() override;
    QString error() override;
    void close() override;

    ~qserialdatasource() override;
};

#endif
#endif // QSERIALDATASOURCE_H
