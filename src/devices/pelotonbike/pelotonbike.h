#ifndef PELOTONBIKE_H
#define PELOTONBIKE_H

#include <QtCore/qbytearray.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QUdpSocket>

#include "devices/bike.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class pelotonbike : public bike {
    Q_OBJECT
  public:
    pelotonbike(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    bool inclinationAvailableByHardware() override;

  private:
    void forceResistance(double resistance);
    uint16_t watts() override;
    double getDouble(QString v);

    QTimer *refresh;

    uint8_t sec1Update = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastInclinationChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    QUdpSocket* pelotonOCRsocket = nullptr;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);

private slots:
    void pelotonOCRprocessPendingDatagrams();

    void changeInclinationRequested(double grade, double percentage);

    void update();
};

#endif // PELOTONBIKE_H
