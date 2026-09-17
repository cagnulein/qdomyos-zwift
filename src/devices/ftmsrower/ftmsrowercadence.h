#ifndef FTMSROWER_CADENCE_H
#define FTMSROWER_CADENCE_H

#include <QList>
#include <QtGlobal>

class ftmsrowerCadenceCalculator {
  public:
    double update(quint16 strokeCount, qint64 timestampMs, double reportedCadence);
    double cadence() const { return m_cadence; }
    bool isStale(qint64 timestampMs) const;
    void reset();

  private:
    struct sample {
        quint16 strokeCount;
        qint64 timestampMs;
    };

    QList<sample> m_samples;
    double m_cadence = 0;
    qint64 m_lastStrokeTimestampMs = -1;
    qint64 m_recentIntervalMs = 0;
};

#endif // FTMSROWER_CADENCE_H
