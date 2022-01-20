#ifndef ZWIFTWORKOUT_H
#define ZWIFTWORKOUT_H
#include "trainprogram.h"

class zwiftworkout {

  public:
    static QList<trainrow> load(const QString &filename);
    static QList<trainrow> load(const QByteArray &input);

  private:
    static bool durationAsDistance(QString sportType, QString durationType);
};

#endif // ZWIFTWORKOUT_H
