#ifndef NORDICTRACKIFITADBBIKE_H
#define NORDICTRACKIFITADBBIKE_H

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

#include "bike.h"
#include "virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class nordictrackifitadbbike : public bike {
    Q_OBJECT
  public:
    nordictrackifitadbbike(bool noWriteResistance, bool noHeartService);
    bool connected();
    bool inclinationAvailableByHardware();

    void *VirtualTreadmill();
    void *VirtualDevice();

  private:
    void forceResistance(double resistance);
    uint16_t watts();

    QTimer *refresh;
    virtualbike *virtualBike = nullptr;

    uint8_t sec1Update = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    QUdpSocket *socket = nullptr;
    QHostAddress lastSender;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);

  private slots:

    void processPendingDatagrams();
    void changeInclinationRequested(double grade, double percentage);

    void update();
};

#endif // NORDICTRACKIFITADBBIKE_H
