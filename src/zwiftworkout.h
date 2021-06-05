#ifndef ZWIFTWORKOUT_H
#define ZWIFTWORKOUT_H
#include "trainprogram.h"

class zwiftworkout {
  public:
    static QList<trainrow> load(const QString &filename);
};

#endif // ZWIFTWORKOUT_H
