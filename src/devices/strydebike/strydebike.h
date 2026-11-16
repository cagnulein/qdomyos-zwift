#ifndef STRYDEBIKE_H
#define STRYDEBIKE_H

#include "devices/bike.h"
#include <QElapsedTimer>
#include <QTimer>

class strydebike : public bike {
    Q_OBJECT
public:
    strydebike(bool noHeartService, int8_t resistanceOffset, double resistanceGain);
    ~strydebike() override;
    bool connected() override { return serialOpen && !stale; }
    uint16_t watts() override { return static_cast<uint16_t>(m_watt.value()); }
    resistance_t maxResistance() override { return 100; }

signals:
    void debug(QString message);

private slots:
    void update();

private:
    QTimer refresh;
    QElapsedTimer lastFrame;
    QElapsedTimer crankTimer;
    double crankRemainder = 0;
    QByteArray buffer;
    bool serialOpen = false;
    bool announced = false;
    bool stale = false;
};
#endif
