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
#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QUdpSocket>

#include "devices/bike.h"
#include "pelotonpowermodel.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class pelotonbike : public bike {
    Q_OBJECT
  public:
    pelotonbike(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    bool inclinationAvailableByHardware() override;
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    void changePower(int32_t power) override;
    void changeResistance(resistance_t resistance) override;
    void changeInclination(double grade, double percentage) override;
    void stop(bool pause) override;
    void setPaused(bool paused) override;
    void workoutEventStateChanged(bluetoothdevice::WORKOUT_EVENT_STATE state) override;

  private:
    bool forceResistance(double resistance);
    void updateErg(float power, float cadence, float resistance);
    void resetErg();
    uint16_t watts() override;
    double getDouble(QString v);
    
    // Peloton sensor integration methods (similar to nordictrackadbbike GRPC)
    void initializePelotonSensorService();
    void startPelotonSensorUpdates();
    void stopPelotonSensorUpdates();
    float getPelotonPower();
    float getPelotonCadence();
    float getPelotonResistance();
    float getPelotonSpeed();
    bool isPelotonSensorConnected();

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
    bool pelotonSensorInitialized = false;

    // Grupetto Bike+ model and ERG motor controller (native Affernet telemetry).
    PelotonPowerModel m_powerModel;
    QElapsedTimer m_ergClock;
    bool m_ergActive = false;
    bool m_ergPowerCommand = false;
    bool m_ergFeedForwardPending = false;
    bool m_ergFeedForwardRamp = false;
    int m_ergTargetWatts = 0;
    int m_ergGoalResistance = -1;
    int m_ergExpectedResistance = -1;
    int m_ergLastObservedResistance = -1;
    int m_ergStableResistance = -1;
    int m_ergUnexpectedResistance = -1;
    int m_ergUnexpectedSamples = 0;
    qint64 m_ergCadenceSince = -1;
    qint64 m_ergPedalingSince = -1;
    qint64 m_ergStableSince = 0;
    qint64 m_ergWriteSince = 0;
    qint64 m_ergLastWrite = -1000;
    qint64 m_ergSettleUntil = 0;
    qint64 m_ergLastPowerSample = 0;
    float m_ergFilteredPower = 0.0f;
    float m_ergCorrectedResistance = 0.0f;

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
