#include "devices/ftmsrower/ftmsrowercadence.h"

#include <QtGlobal>

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

    // Average individual stroke intervals before deriving cadence. This avoids
    // the quantized 20/29 SPM jumps caused by selecting one interval as the
    // representative value for this JOROTO model.
    m_recentIntervalsMs.append(m_recentIntervalMs);
    while (m_recentIntervalsMs.size() > 7) {
        m_recentIntervalsMs.removeFirst();
    }

    // Wait for four complete stroke intervals before publishing a value. The
    // JOROTO occasionally reports a short/long pair for one real interval.
    if (m_recentIntervalsMs.size() < 4) {
        return m_cadence;
    }

    qint64 totalIntervalMs = 0;
    for (const qint64 intervalMs : m_recentIntervalsMs) {
        totalIntervalMs += intervalMs;
    }
    const double averageIntervalMs = static_cast<double>(totalIntervalMs) /
                                     static_cast<double>(m_recentIntervalsMs.size());
    m_cadence = 60000.0 / averageIntervalMs;

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
