#ifndef INSPIREIC15DSERIALBIKE_H
#define INSPIREIC15DSERIALBIKE_H

#include "devices/bike.h"
#include "inspireic15dserialreader.h"

#include <QTimer>

class virtualbike;

class inspireic15dserialbike : public bike {
    Q_OBJECT
  public:
    inspireic15dserialbike(const QString &serialPort, bool noHeartService, int8_t bikeResistanceOffset,
                           double bikeResistanceGain);
    ~inspireic15dserialbike() override;

    bool connected() override;
    uint16_t watts() override;
    resistance_t maxResistance() override { return 100; }
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;

    // Stock console conversions, exposed as pure static functions for unit testing.
    static int stockResistance(int controllerResistance);
    static int stockPower(int cadence, int resistance);

  signals:
    void disconnected();
    void debug(QString string);

  private slots:
    void update();

  private:
    QTimer *refresh = nullptr;
    inspireic15dserialreader *reader = nullptr;
    bool connectionAnnounced = false;
    bool lastPortOpen = false;
    qint64 lastReportedBytes = -1;
    qint64 lastValidFrames = -1;
    QString lastError;
};

#endif // INSPIREIC15DSERIALBIKE_H
