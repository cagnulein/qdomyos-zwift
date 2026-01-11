//#include "pch.h"
#include "trixterxdreamclient.h"

#include <string>
#include <cmath>

using namespace std;

trixterxdreamclient::trixterxdreamclient() {
    this->ConfigureResistanceMessages();
}

void trixterxdreamclient::ResetBuffer() {
    // for the case of an invalid packet, if this was smart, it would store all the input
    // and backtrack to the first header bytes after the beginning.

    this->inputBuffer.clear();
    this->byteBuffer.clear();
}

void trixterxdreamclient::set_GetTime(std::function<uint32_t()> get_time_ms) {
    this->get_time_ms = get_time_ms;
}

trixterxdreamclient::PacketState trixterxdreamclient::ProcessChar(char c) {
    /* Packet content
     *                            6A 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
     * (00) Header ---------------+  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
     * (01) Steering ----------------+  |  |  |  |  |  |  |  |  |  |  |  |  |  |
     * (02) Unknown --------------------+  |  |  |  |  |  |  |  |  |  |  |  |  |
     * (03) Crank position ----------------+  |  |  |  |  |  |  |  |  |  |  |  |
     * (04) Right brake ----------------------+  |  |  |  |  |  |  |  |  |  |  |
     * (05) Left brake --------------------------+  |  |  |  |  |  |  |  |  |  |
     * (06) Unknown --------------------------------+  |  |  |  |  |  |  |  |  |
     * (07) Unknown -----------------------------------+  |  |  |  |  |  |  |  |
     * (08) Button flags ---------------------------------+  |  |  |  |  |  |  |
     * (09) Button flags ------------------------------------+  |  |  |  |  |  |
     * (0A) Crank revolution time (high byte) ------------------+  |  |  |  |  |
     * (0B) Crank revolution time (low byte) ----------------------+  |  |  |  |
     * (0C) Flywheel Revolution Time (high byte) ---------------------+  |  |  |
     * (0D) Flywheel Revolution Time (low byte) -------------------------+  |  |
     * (0E) Heart rate (BPM) -----------------------------------------------+  |
     * (0F) XOR of 00..0E------------------------------------------------------+
     */

    constexpr int headerLength = 2;
    constexpr int packetLength = 16;
    constexpr uint8_t header[] = { 0x6, 0xA };

    uint8_t b;

    if (isdigit(c)) {
        b = c - '0';
    }
    else if (c >= 'a' && c <= 'f') {
        b = c - 'a' + '\xA';
    }
    else {
        this->ResetBuffer();
        return Invalid;
    }

    // make sure the first 2 bytes are the header '6','a'
    if (this->byteBuffer.empty() && this->inputBuffer.size() < headerLength && b != header[this->inputBuffer.size()]) {
        this->inputBuffer.clear();
        return None;
    }

    if (this->inputBuffer.size() == 1) {
        this->byteBuffer.push_back((this->inputBuffer.back() << 4) + b);
        this->inputBuffer.clear();
    }
    else
        this->inputBuffer.push_back(b);

    if (this->byteBuffer.size() == packetLength) {
        // Validate the packet - the last byte should the XOR of the 1st 15.
        b = 0;
        for (int i = 0, limit = packetLength - 1; i < limit; i++)
            b ^= this->byteBuffer[i];

        if (b != this->byteBuffer.back()) {
            // invalid checksum
            this->ResetBuffer();
            return Invalid;
        }

        return Complete;
    }

    return Incomplete;
}

void trixterxdreamclient::ConfigureResistanceMessages() {
    resistanceMessages = new uint8_t * [251];

    for (uint8_t level = 0; level <= 250; level++) {
        uint8_t* message = new uint8_t[6];
        resistanceMessages[level] = message;

        message[5] = message[0] = 0x6a;
        message[5] ^= message[1] = level;
        message[5] ^= message[2] = (level + 60) % 255;
        message[5] ^= message[3] = (level + 90) % 255;
        message[5] ^= message[4] = (level + 120) % 255;
    }
}

bool trixterxdreamclient::ReceiveChar(char c) {
    if (this->ProcessChar(c) != Complete)
        return false;

    lastPacket.Buttons = (static_cast<uint16_t>(this->byteBuffer[0x8]) << 8) + this->byteBuffer[0x9];
    lastPacket.CrankPosition = this->byteBuffer[0x3];
    lastPacket.Brake1 = this->byteBuffer[0x4];
    lastPacket.Brake2 = this->byteBuffer[0x5];
    lastPacket.Steering = this->byteBuffer[0x1];
    lastPacket.Flywheel = (static_cast<uint16_t>(this->byteBuffer[0xC]) << 8) + this->byteBuffer[0xD];
    lastPacket.Crank = (static_cast<uint16_t>(this->byteBuffer[0xA]) << 8) + this->byteBuffer[0xB];
    lastPacket.HeartRate = byteBuffer[0xE];

    // got the data, now clear the buffer
    this->ResetBuffer();

    constexpr double flywheelToRevolutionsPerMinute = 576000.0;
    constexpr double crankToRevolutionsPerMinute = 1.0 / 6e-6;
    constexpr double minutesToMilliseconds = 60.0 * 1000.0;

    double flywheelRevsPerMinute = 0, crankRevsPerMinute = 0;

    if (lastPacket.Flywheel < 65534) {
        flywheelRevsPerMinute = flywheelToRevolutionsPerMinute / max(static_cast<uint16_t>(1), lastPacket.Flywheel);
    }

    if (lastPacket.Crank > 0 && lastPacket.Crank < 65534) {
        crankRevsPerMinute = crankToRevolutionsPerMinute / max(static_cast<uint16_t>(1), lastPacket.Crank);
    }

    const uint32_t t = this->get_time_ms();
    const uint32_t lt = this->lastT ? this->lastT : t;

    this->lastT = t;

    if(t<lt)
    {
        // TODO: error logging - this could be indicative of a problem
        // It should usually be about 12ms

        this->Reset();
        return false;
    }

    const uint32_t dt = t - lt;

    if (dt > 0)
    {
        // update the internal, precise state
        double dt_minutes = dt / minutesToMilliseconds;

        this->flywheelRevolutions += dt_minutes * flywheelRevsPerMinute;
        this->crankRevolutions += dt_minutes * crankRevsPerMinute;
    }

    state newState{};
    newState.LastEventTime = t;
    newState.Steering = lastPacket.Steering;
    newState.HeartRate = lastPacket.HeartRate;
    newState.CumulativeCrankRevolutions = static_cast<uint16_t>(round(crankRevolutions));
    newState.CumulativeWheelRevolutions = static_cast<uint32_t>(round(flywheelRevolutions));
    newState.CrankRPM = static_cast<uint16_t>(crankRevsPerMinute);
    newState.FlywheelRPM = static_cast<uint16_t>(flywheelRevsPerMinute);
    newState.Buttons = (buttons)(0xFFFF-lastPacket.Buttons);
    newState.Brake1 = lastPacket.Brake1;
    newState.Brake2 = lastPacket.Brake2;

    this->stateMutex.lock();
    this->lastState = newState;
    this->stateMutex.unlock();

    return true;
}

trixterxdreamclient::state trixterxdreamclient::getLastState() {
    this->stateMutex.lock();
    const state result = this->lastState;
    this->stateMutex.unlock();
    return result;
}

void trixterxdreamclient::SendResistance(uint8_t level) {

    // to maintain the resistance, this needs to be resent about every 10ms
    if (level != 0 && this->write_bytes)
    {
        this->writeMutex.lock();
        try { this->write_bytes(this->resistanceMessages[min(MaxResistance, level)], 6); }
        catch (...)
        {
            this->writeMutex.unlock();
            throw;
        }
        this->writeMutex.unlock();
    }
}

void trixterxdreamclient::Reset() {
    this->lastT = this->get_time_ms();
    this->flywheelRevolutions = 0.0;
    this->crankRevolutions = 0.0;
}
