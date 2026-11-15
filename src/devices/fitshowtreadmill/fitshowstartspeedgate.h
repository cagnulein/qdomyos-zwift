#ifndef FITSHOW_START_SPEED_GATE_H
#define FITSHOW_START_SPEED_GATE_H

// FitShow may acknowledge a target-speed command during its start countdown
// without actually applying it. Retain the latest queued workout target until
// the treadmill's status packet confirms RUNNING.
class FitShowStartSpeedGate {
public:
    void startSent() { m_waitingForRunning = true; }
    void runningConfirmed() { m_waitingForRunning = false; }
    void cancel() { m_waitingForRunning = false; }

    bool waitingForRunning() const { return m_waitingForRunning; }

    // The FitShow target command includes both speed and incline. Neither may
    // be sent before the start request is dispatched and RUNNING is confirmed.
    bool canSendTarget(bool startRequested, bool stopRequested) const {
        return !m_waitingForRunning && !startRequested && !stopRequested;
    }

private:
    bool m_waitingForRunning = false;
};

#endif // FITSHOW_START_SPEED_GATE_H
