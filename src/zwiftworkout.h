#ifndef ZWIFTWORKOUT_H
#define ZWIFTWORKOUT_H
#include "trainprogram.h"

class zwiftworkout {

  public:
    static QList<trainrow> load(const QString &filename);
    static QList<trainrow> load(const QByteArray &input);
    static QList<trainrow> loadJSON(const QString &input);

  private:
    static bool durationAsDistance(QString sportType, QString durationType);
    static double speedFromPace(int Pace);
    static void convertTag(double thresholdSecPerKm, const QString &sportType, const QString &durationType,
                           QList<trainrow> &list, const char *tag, ...);
};

#endif // ZWIFTWORKOUT_H
