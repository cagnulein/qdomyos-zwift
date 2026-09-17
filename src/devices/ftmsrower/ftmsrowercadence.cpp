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

    const sample first = m_samples.first();
    const quint16 totalStrokeDelta = static_cast<quint16>(strokeCount - first.strokeCount);
    const qint64 totalElapsedMs = timestampMs - first.timestampMs;
    if (totalStrokeDelta > 0 && totalElapsedMs > 0) {
        m_cadence = (static_cast<double>(totalStrokeDelta) * 60000.0) /
                    static_cast<double>(totalElapsedMs);
        m_recentIntervalMs = elapsedMs / strokeDelta;
        m_lastStrokeTimestampMs = timestampMs;
    }

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
    m_cadence = 0;
    m_hasStroke = false;
    m_lastStrokeTimestampMs = -1;
    m_recentIntervalMs = 0;
}
