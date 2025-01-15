#ifndef JUMPROPE_H
#define JUMPROPE_H

#include "devices/bluetoothdevice.h"
#include <QObject>

class jumprope : public bluetoothdevice {
    Q_OBJECT

  public:
    jumprope();
    bool connected() override;
    virtual uint16_t watts(double weight);
    bluetoothdevice::BLUETOOTH_TYPE deviceType() override;
    void clearStats() override;
    void setLap() override;
    void setPaused(bool p) override;
    metric JumpsSequence;
    metric JumpsCount;

  public slots:

  signals:

  protected:
};

#endif // JUMPROPE_H
