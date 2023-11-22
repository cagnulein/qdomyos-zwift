#ifndef IOSELITEARIAFAN_H
#define IOSELITEARIAFAN_H

class ios_eliteariafan {
  public:
    ios_eliteariafan() {
        m_control = [[ios_interal_eliteariafan alloc] init];
    }

    void fanSpeedRequest(unsigned char speed) {
        if (m_control != nullptr) {
            m_control->setFanSpeed(speed);
        }
    }

   private:
    ios_interal_eliteariafan *m_control = nullptr;
};

#endif // IOSELITEARIAFAN_H
