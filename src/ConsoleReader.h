#ifndef CONSOLEREADER_H
#define CONSOLEREADER_H

#include "bluetooth.h"
#include <QThread>

class ConsoleReader : public QThread {
    Q_OBJECT
  signals:
    void KeyPressed(char ch);

  public:
    ConsoleReader(bluetooth *bt);
    ~ConsoleReader();
    void run();

  private:
    bluetooth *bluetoothManager;
};

#endif /* CONSOLEREADER_H */
