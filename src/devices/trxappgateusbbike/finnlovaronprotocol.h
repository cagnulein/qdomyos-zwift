#ifndef FINNLOVARONPROTOCOL_H
#define FINNLOVARONPROTOCOL_H

#include <array>
#include <cstddef>
#include <cstdint>

// Experimental legacy FINNLO TC174 protocol, derived from four Kinomap sessions.
// No physical resistance or target-power commands are known.
class FinnloVaronProtocol {
public:
    enum class Command { None, IdentityAck, Initialize, Start, Stop };
    struct Telemetry { uint16_t first; uint8_t middle; uint16_t second; };

    static bool validFrame(const uint8_t *p, std::size_t n) {
        if (!p || n < 5 || n > 255 || p[0] != 0x5b || p[2] != n || p[n - 1] != 0x5d)
            return false;
        uint8_t checksum = 0;
        for (std::size_t i = 1; i < n - 2; ++i)
            checksum = static_cast<uint8_t>(checksum + p[i]);
        return checksum == p[n - 2];
    }

    static std::array<uint8_t, 6> encode(Command command) {
        uint8_t opcode = 0;
        uint8_t value = 0x31;
        switch (command) {
        case Command::IdentityAck: opcode = 0xc1; break;
        case Command::Initialize: opcode = 0x82; break;
        case Command::Start: opcode = 0x83; value = 0x30; break;
        case Command::Stop: opcode = 0x83; break;
        case Command::None: return {};
        }
        return {{0x5b, opcode, 0x06, value, static_cast<uint8_t>(opcode + 6 + value), 0x5d}};
    }

    static bool telemetry(const uint8_t *p, std::size_t n, Telemetry &out) {
        if (!validFrame(p, n) || n != 10 || p[1] != 0x85)
            return false;
        out.first = static_cast<uint16_t>(p[3] | (p[4] << 8));
        out.middle = p[5];
        out.second = static_cast<uint16_t>(p[6] | (p[7] << 8));
        return true;
    }

    void reset() { state = State::WaitingIdentity; due = 0; retries = 0; }
    bool streaming() const { return state == State::Streaming; }

    Command receive(const uint8_t *p, std::size_t n, int64_t now) {
        if (!validFrame(p, n))
            return Command::None;
        if (n == 12 && p[1] == 0x81) {
            if (state == State::WaitingIdentity) {
                state = State::InitDelay;
                due = now + 200;
                retries = 0;
            }
            return Command::IdentityAck;
        }
        if (n == 6 && p[3] == 0x31) {
            if (p[1] == 0xc2 && state == State::AwaitInitAck) {
                state = State::StartDelay;
                due = now + 900;
                retries = 0;
            } else if (p[1] == 0xc3 && state == State::AwaitStartAck) {
                state = State::Streaming;
            }
        }
        return Command::None;
    }

    Command tick(int64_t now) {
        if (now < due)
            return Command::None;
        if (state == State::InitDelay || state == State::AwaitInitAck) {
            if (retries >= 3) { reset(); return Command::None; }
            state = State::AwaitInitAck;
            ++retries;
            due = now + 3000;
            return Command::Initialize;
        }
        if (state == State::StartDelay || state == State::AwaitStartAck) {
            if (retries >= 3) { reset(); return Command::None; }
            state = State::AwaitStartAck;
            ++retries;
            due = now + 3000;
            return Command::Start;
        }
        return Command::None;
    }

    Command stop() {
        if (state != State::Streaming && state != State::AwaitStartAck)
            return Command::None;
        state = State::Paused;
        return Command::Stop;
    }
    Command resume(int64_t now) {
        if (state != State::Paused)
            return Command::None;
        state = State::AwaitStartAck;
        retries = 1;
        due = now + 3000;
        return Command::Start;
    }

private:
    enum class State { WaitingIdentity, InitDelay, AwaitInitAck, StartDelay, AwaitStartAck, Streaming, Paused };
    State state = State::WaitingIdentity;
    int64_t due = 0;
    unsigned retries = 0;
};

#endif
