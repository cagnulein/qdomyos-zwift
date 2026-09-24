#include "devices/ftmsrower/ftmsrowercadence.h"

#include <QtGlobal>
#include <algorithm>

double ftmsrowerCadenceCalculator::update(quint16 strokeCount, qint64 timestampMs, double reportedCadence) {
    Q_UNUSED(reportedCadence);

    if (m_samples.isEmpty()) {
        m_samples.append({strokeCount, timestampMs});
        m_cadence = 0;
        return m_cadence;
    }

    // A pause must not become part of the first interval after resuming.
    if (isStale(timestampMs)) {
        reset();
        m_samples.append({strokeCount, timestampMs});
        return m_cadence;
    }

    const sample previous = m_samples.last();
    const quint16 strokeDelta = static_cast<quint16>(strokeCount - previous.strokeCount);
    const qint64 elapsedMs = timestampMs - previous.timestampMs;

    if (strokeDelta == 0) {
        return m_cadence;
    }

    // A large backwards jump is a device reset, not a real stroke burst.
    if (strokeDelta > 1000 || elapsedMs <= 0) {
        reset();
        m_samples.append({strokeCount, timestampMs});
        return m_cadence;
    }

    // The first increment after startup/resume establishes a new real-stroke baseline.
    if (!m_hasStroke) {
        m_samples.clear();
        m_samples.append({strokeCount, timestampMs});
        m_hasStroke = true;
        m_lastStrokeTimestampMs = timestampMs;
        m_cadence = 0;
        return m_cadence;
    }

    m_samples.append({strokeCount, timestampMs});
    while (m_samples.size() > 5) {
        m_samples.removeFirst();
    }

    // Keep stale detection anchored to every real stroke, including while the
    // four-interval warm-up window is still being collected.
    m_recentIntervalMs = elapsedMs / strokeDelta;
    m_lastStrokeTimestampMs = timestampMs;

    // Filter individual stroke intervals before deriving cadence. This avoids
    // letting one short JOROTO interval contaminate overlapping windows.
    m_recentIntervalsMs.append(m_recentIntervalMs);
    while (m_recentIntervalsMs.size() > 7) {
        m_recentIntervalsMs.removeFirst();
    }

    // Wait for four complete stroke intervals before publishing a value. The
    // JOROTO occasionally reports a short/long pair for one real interval.
    if (m_recentIntervalsMs.size() < 4) {
        return m_cadence;
    }

    QList<qint64> sortedIntervals = m_recentIntervalsMs;
    std::sort(sortedIntervals.begin(), sortedIntervals.end());
    const int middle = sortedIntervals.size() / 2;
    const double medianIntervalMs = sortedIntervals.size() % 2 == 0
        ? (static_cast<double>(sortedIntervals.at(middle - 1)) + sortedIntervals.at(middle)) / 2.0
        : static_cast<double>(sortedIntervals.at(middle));
    m_cadence = 60000.0 / medianIntervalMs;

    return m_cadence;
}

bool ftmsrowerCadenceCalculator::isStale(qint64 timestampMs) const {
    if (!m_hasStroke || m_lastStrokeTimestampMs < 0 || timestampMs <= m_lastStrokeTimestampMs) {
        return false;
    }

    const qint64 timeoutMs = qMax<qint64>(5000, m_recentIntervalMs > 0 ? m_recentIntervalMs * 2 : 5000);
    return timestampMs - m_lastStrokeTimestampMs > timeoutMs;
}

void ftmsrowerCadenceCalculator::reset() {
    m_samples.clear();
    m_recentIntervalsMs.clear();
    m_cadence = 0;
    m_hasStroke = false;
    m_lastStrokeTimestampMs = -1;
    m_recentIntervalMs = 0;
}
