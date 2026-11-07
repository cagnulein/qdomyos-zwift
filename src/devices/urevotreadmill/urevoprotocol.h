#ifndef UREVO_PROTOCOL_H
#define UREVO_PROTOCOL_H

#include <QByteArray>
#include <QVector>
#include <QtGlobal>
#include <QtMath>

// URTM047 FFF0/FFF1/FFF2 + FEE0/FEE1/FEE2 protocol, reconstructed from
// Taylor Morelli's 2026-11-06 iOS HCI trace. Pure state machine: no BLE writes
// or timers here, so replay tests exercise the actual production decisions.
class UrevoProtocol {
public:
    enum class Channel { Main, Auxiliary };
    struct Command {
        Channel channel;
        QByteArray payload;
    };
    enum class State {
        Disconnected, Hello, Info, Auxiliary, Ready,
        FirstStartDelay, QueryDelay, QueryResponse, SecondStartDelay,
        StartAcknowledgement, Starting, Running, Stopping, Failed
    };

    static QByteArray hello() { return QByteArray::fromHex("02510b03"); }
    static QByteArray info() { return QByteArray::fromHex("0250030903"); }
    static QByteArray auxiliaryInit() { return QByteArray::fromHex("5aa501030001100000cc76"); }
    static QByteArray startFrame() { return QByteArray::fromHex("02530100000000000000000e03"); }
    static QByteArray startQuery() { return QByteArray::fromHex("0240021803"); }
    static QByteArray startInfo() { return QByteArray::fromHex("0250020803"); }
    static QByteArray stopFrame(bool pause) {
        return QByteArray::fromHex(pause ? "02530a0703" : "0253030c03");
    }
    static QByteArray idleFrame() { return QByteArray::fromHex("025100020903"); }
    static QByteArray infoReply() { return QByteArray::fromHex("0250035a005b5203"); }
    static QByteArray startReply() { return QByteArray::fromHex("025301030d03"); }

    // The status checksum in the captured URTM047 packets excludes STX 0x02.
    static bool decodeFull(const QByteArray &packet, double &speedKmh, double &inclination) {
        if (packet.size() != 25 || quint8(packet[0]) != 2 || quint8(packet[1]) != 0x51 ||
            quint8(packet[24]) != 3)
            return false;
        const quint8 kind = quint8(packet[2]);
        if (kind != 1 && kind != 3 && kind != 4 && kind != 10)
            return false;
        unsigned sum = 0;
        for (int i = 1; i < 23; ++i)
            sum += quint8(packet[i]);
        if (quint8(packet[23]) != quint8((sum & 0xff) ^ 0x5a))
            return false;
        speedKmh = quint8(packet[3]) * 0.1609344; // 0.1 mph -> km/h
        inclination = quint8(packet[4]) / 10.0;
        return speedKmh <= 22.0 && inclination <= 15.0;
    }

    static QByteArray targetFrame(double speedKmh, double inclination) {
        if (!qIsFinite(speedKmh) || !qIsFinite(inclination) ||
            speedKmh < 0 || speedKmh > 22 || inclination < 0 || inclination > 15)
            return {};
        const int speed10Mph = qRound(speedKmh / 0.1609344);
        const int incline10 = qRound(inclination * 10);
        if (speed10Mph > 255 || incline10 > 255)
            return {};
        QByteArray packet = QByteArray::fromHex("02530200000003");
        packet[3] = char(speed10Mph);
        packet[4] = char(incline10);
        packet[5] = char(((0x53 + 0x02 + speed10Mph + incline10) & 255) ^ 0x5a);
        return packet;
    }

    void reset() {
        m_state = State::Disconnected;
        m_outbox.clear();
        m_phaseAt = m_statusAt = m_stopAt = 0;
        m_speed = m_inclination = 0;
        m_fullStatus = m_newFullStatus = m_startAfterHandshake = false;
    }

    void subscribed(qint64 now) {
        if (m_state != State::Disconnected)
            return;
        beginHandshake(false, now);
    }

    bool requestStart(qint64 now) {
        // An explicit Start is required. A speed or incline request must never
        // implicitly start the motor; an idle report alone is not a Start ACK.
        if (m_state != State::Ready || !statusFresh(now) || m_speed > 0)
            return false;
        beginHandshake(true, now);
        return true;
    }

    void requestStop(bool pause, qint64 now) {
        // Stop overrides all queued handshakes, starts and target writes.
        m_outbox.clear();
        m_startAfterHandshake = false;
        m_newFullStatus = false;
        m_stopAt = 0; // Set only when BLE confirms the Stop write.
        setState(State::Stopping, now);
        enqueue(Channel::Main, stopFrame(pause));
    }

    void stopWriteConfirmed(qint64 now) {
        // ATT Write Response is only delivery confirmation, not a physical stop.
        if (m_state == State::Stopping)
            m_stopAt = now;
    }

    void receiveMain(const QByteArray &value, qint64 now) {
        if (value == idleFrame()) {
            m_statusAt = now;
            if (m_state == State::Hello) {
                setState(State::Info, now);
                enqueue(Channel::Main, info());
            } else if (m_state == State::Stopping && m_stopAt > 0 && now > m_stopAt + 1000) {
                // Local stop gate only; an idle response is not proof of physical stop.
                setState(State::Ready, now);
            }
            return;
        }
        if (value == infoReply() && m_state == State::Info) {
            setState(State::Auxiliary, now);
            enqueue(Channel::Auxiliary, auxiliaryInit());
            return;
        }
        if (m_state == State::QueryResponse && value.startsWith(QByteArray::fromHex("024002"))) {
            setState(State::SecondStartDelay, now);
            return;
        }
        if (m_state == State::StartAcknowledgement && value == startReply()) {
            setState(State::Starting, now);
            enqueue(Channel::Main, startInfo());
            return;
        }

        double speed, incline;
        if (!decodeFull(value, speed, incline))
            return;
        m_speed = speed;
        m_inclination = incline;
        m_statusAt = now;
        m_fullStatus = true;
        m_newFullStatus = true;
        if (m_state == State::Starting && speed > 0) {
            setState(State::Running, now);
        } else if (m_state == State::Stopping && speed == 0 && m_stopAt > 0 && now >= m_stopAt) {
            setState(State::Ready, now);
        } else if (m_state == State::Running && speed == 0) {
            setState(State::Ready, now);
        }
    }

    void receiveAuxiliary(const QByteArray &value, qint64 now) {
        // Observed FEE1 response to 5A A5 01 03 ... CC 76.
        if (m_state != State::Auxiliary ||
            !value.startsWith(QByteArray::fromHex("5aa503010001100c00")))
            return;
        if (m_startAfterHandshake) {
            setState(State::FirstStartDelay, now);
        } else {
            setState(State::Ready, now);
        }
    }

    void tick(qint64 now) {
        if (m_state == State::FirstStartDelay && now >= m_phaseAt + 800) {
            enqueue(Channel::Main, startFrame());
            setState(State::QueryDelay, now);
        } else if (m_state == State::QueryDelay && now >= m_phaseAt + 180) {
            enqueue(Channel::Main, startQuery());
            setState(State::QueryResponse, now);
        } else if (m_state == State::SecondStartDelay && now >= m_phaseAt + 450) {
            enqueue(Channel::Main, startFrame());
            setState(State::StartAcknowledgement, now);
        }

        // No endless retries or speculative commands after a missing response.
        const bool handshake = m_state == State::Hello || m_state == State::Info ||
                               m_state == State::Auxiliary || m_state == State::QueryResponse ||
                               m_state == State::StartAcknowledgement;
        if ((handshake && now > m_phaseAt + 4000) ||
            (m_state == State::Starting && now > m_phaseAt + 8000))
            fail(now);
    }

    bool requestTarget(double speedKmh, double inclination, qint64 now) {
        if (m_state != State::Running || !m_fullStatus || !m_newFullStatus || !statusFresh(now))
            return false;
        const auto packet = targetFrame(speedKmh, inclination);
        if (packet.isEmpty())
            return false;
        enqueue(Channel::Main, packet);
        m_newFullStatus = false; // Wait for the next physical status before another target.
        return true;
    }

    QVector<Command> takeCommands() {
        QVector<Command> result = m_outbox;
        m_outbox.clear();
        return result;
    }
    State state() const { return m_state; }
    bool ready() const { return m_state == State::Ready || m_state == State::Running; }
    bool running() const { return m_state == State::Running; }
    bool hasFullStatus() const { return m_fullStatus; }
    double speedKmh() const { return m_speed; }
    double inclination() const { return m_inclination; }
    bool statusFresh(qint64 now) const {
        return m_statusAt > 0 && now >= m_statusAt && now - m_statusAt < 3000;
    }
    void fail(qint64 now) {
        m_outbox.clear();
        m_startAfterHandshake = false;
        setState(State::Failed, now);
    }

private:
    void beginHandshake(bool forStart, qint64 now) {
        m_startAfterHandshake = forStart;
        setState(State::Hello, now);
        enqueue(Channel::Main, hello());
    }
    void setState(State state, qint64 now) { m_state = state; m_phaseAt = now; }
    void enqueue(Channel channel, const QByteArray &payload) {
        m_outbox.append({channel, payload});
    }
    State m_state = State::Disconnected;
    QVector<Command> m_outbox;
    bool m_startAfterHandshake = false;
    bool m_fullStatus = false;
    bool m_newFullStatus = false;
    qint64 m_phaseAt = 0;
    qint64 m_statusAt = 0;
    qint64 m_stopAt = 0;
    double m_speed = 0;
    double m_inclination = 0;
};

#endif // UREVO_PROTOCOL_H
