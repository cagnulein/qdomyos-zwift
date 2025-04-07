#ifndef STAIRCLIMBER_H
#define STAIRCLIMBER_H

#include "devices/bluetoothdevice.h"
#include <QObject>

class stairclimber : public bluetoothdevice {
    Q_OBJECT

  public:
    stairclimber();
    bool connected() override;
    virtual uint16_t watts(double weight);
    bluetoothdevice::BLUETOOTH_TYPE deviceType() override;
    void clearStats() override;
    void setLap() override;
    void setPaused(bool p) override;

  public slots:

  signals:

  protected:
};

#endif // STAIRCLIMBER_H
