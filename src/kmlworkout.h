#ifndef KMLWORKOUT_H
#define KMLWORKOUT_H
#include "trainprogram.h"

class kmlworkout {
  public:
    static QList<trainrow> load(const QString &filename);
    static QList<trainrow> load(const QByteArray &input);
};

#endif // KMLWORKOUT_H
