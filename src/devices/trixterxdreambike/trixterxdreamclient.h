#pragma once
#include <vector>
#include <cstdint>
#include <functional>
#include <mutex>

/**
 * @brief Basic functionality to interpret character data read from a Trixter X-Dream bike via a serial port.
 * Intended to be free from any non-standard C++ library code.
 * Requires a time source callback (set_getTime) to timestamp packets and optionally
 * a callback to write resistance packets to the serial port.
 */
class trixterxdreamclient {
public:

    /**
     * @brief Values for the button state.
     */
    enum buttons : uint16_t
    {
        LeftArrow = 4096,
        RightArrow = 16384,
        UpArrow = 256,
        DownArrow = 1024,

        Blue = 8192,
        Red = 512,
        Green = 2048,

        Seated = 8,

        FrontGearUp = 32768,
        FrontGearDown = 128,

        BackGearUp = 32,
        BackGearDown = 64
    };

    /**
     * @brief Device state data: CSCS, heartrate, steering, buttons.
     */
    struct state {
        /**
         * @brief Buttons The state of the buttons.
         */
        buttons Buttons;

        /**
         * @brief Steering Steering value, from 0 (left) to 250 (right)
         */
        uint8_t Steering;

        /**
         * @brief HeartRate Heart rate in beats per minute.
         */
        uint8_t HeartRate;

        /**
         * @brief CumulativeWheelRevolutions The number of flywheel revolutions since the last reset event.
         */
        uint32_t CumulativeWheelRevolutions;

        /**
         * @brief CumulativeCrankRevolutions The number of crank revolutions since the last reset event.
         */
        uint16_t CumulativeCrankRevolutions;

        /**
         * @brief LastEventTime The time of the last event. Unit:  milliseconds
         */
        uint32_t LastEventTime;

        /**
         * @brief FlywheelRPM Flywheel speed. Units: revolutions per minute
         */
        uint16_t FlywheelRPM;

        /**
         * @brief CrankRPM Crank speed. Units: revolutions per minute
         */
        uint16_t CrankRPM;

        /**
         * @brief CrankPosition Position of the crank. Range: 1 to 60.
         */
        uint8_t CrankPosition;

        /**
         * @brief Brake 1. Position of brake 1. Range: 135 (on) to 250 (off)
         */
        uint8_t Brake1;


        /**
         * @brief Brake 2. Position of brake 1. Range: 135 (on) to 250 (off)
         */
        uint8_t Brake2;
    };

private:
    uint8_t** resistanceMessages{};

    enum PacketState { None, Incomplete, Invalid, Complete };

    /**
     * @brief Raw data selected from the incoming packet.
     */
    struct Packet {
        uint8_t Steering, Brake1, Brake2, HeartRate, CrankPosition;
        uint16_t Flywheel, Crank, Buttons;
    };

    std::function<uint32_t()> get_time_ms=nullptr;
    std::function<void(uint8_t*, int)> write_bytes=nullptr;
    std::mutex stateMutex, writeMutex;
    uint32_t lastT = 0;
    double flywheelRevolutions{}, crankRevolutions{};
    Packet lastPacket{};
    std::vector<uint8_t> inputBuffer;
    std::vector<uint8_t> byteBuffer;
    state lastState;

    /**
     * @brief Clear the input buffer.
     */
    void ResetBuffer();

    /**
     * @brief Add the character to the input buffer and process to eventually read the next packet.
     * @param c A text character '0'..'9' or 'a'..'f'
     */
    PacketState ProcessChar(char c);

    void ConfigureResistanceMessages();

public:
    /**
     * @brief GearRatio The physical gear ratio between the flywheel:crank.
     */
    constexpr static uint8_t GearRatio = 5;

    /**
     * @brief MaxResistance The maximum resistance value supported by the device.
     */
    constexpr static uint8_t MaxResistance = 250;

    /**
     * @brief MaxSteering The maximum steering value supported by the device.
     */
    constexpr static uint8_t MaxSteering = 255;


    /**
     * @brief MaxBrake The maximum brake value, which indicates fully off.
     */
    constexpr static uint8_t MaxBrake = 250;

    /**
     * @brief MinBrake The minimum brake value, which indicates fully on.
     */
    constexpr static uint8_t MinBrake = 135;

    /**
     * @brief MinCrankPosition The minimum CrankPosition value.
     */
    constexpr static uint8_t MinCrankPosition = 1;

    /**
    * @brief MinCrankPosition The maximum CrankPosition value.
    */
    constexpr static uint8_t MaxCrankPosition = 60;

    /**
     * @brief The time interval between sending resistance requests to the device.
     * This is intended to achieve 60 resistance packets sent per second.
     */
    constexpr static uint8_t ResistancePulseIntervalMilliseconds = 16;

    trixterxdreamclient();

    /**
     * @brief Receives and processes a character of input from the device.
     * @param c Should be '0' to '9' or 'a' to 'f' (lower case)
     * @return true if a packet was completed and the state updated, otherwise false.
     */
    bool ReceiveChar(char c);

    /**
     * @brief set_WriteBytes Sets the function used to write bytes to the serial port.
     * @param write_bytes The function that writes bytes to the serial port.
     */
    void set_WriteBytes(std::function<void(uint8_t*, int)> write_bytes) { this->write_bytes = write_bytes; }

    /**
     * @brief set_GetTime Sets the function to get the time in milliseconds since
     * a starting point understood by the client.
     * @param get_time_ms A function to get the time.
     */
    void set_GetTime(std::function<uint32_t()> get_time_ms);

    /**
     * @brief Gets the state of the device as it was last read. This consists of CSCS data, steering and heartbeat.
     */
    state getLastState();

    /**
     * @brief Reset the Cycle Speed and Cadence information.
     */
    void Reset();

    /**
     * @brief Sends 1 packet indicating a specific resistance level to the device. Needs to be sent at the rate specified by ResistancePulseIntervalMilliseconds.
     * @param level 0 to 250.
     */
    void SendResistance(uint8_t level);
};
