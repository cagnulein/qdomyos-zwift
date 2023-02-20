#include "TrixterXDreamV1PacketInterpreterTests.h"

uint32_t TrixterXDreamV1PacketInterpreterTests::getTime()
{
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    return static_cast<uint32_t>(ms.count());
}

void TrixterXDreamV1PacketInterpreterTests::TestInput(std::string input, uint8_t expectedHR, uint8_t expectedSteering)
{
    trixterxdreamv1client tx1;

    tx1.set_GetTime(getTime);

    for (char value : input)
        tx1.ReceiveChar(value);

    trixterxdreamv1client::state state = tx1.getLastState();

    EXPECT_EQ(state.HeartRate, expectedHR);
    EXPECT_EQ(state.Steering, expectedSteering);


}

void TrixterXDreamV1PacketInterpreterTests::TestResistance(trixterxdreamv1client *tx1, uint8_t resistanceLevel)
{

    this->packet = nullptr;
    this->packetLength = -1;
    tx1->SendResistance(resistanceLevel);

    auto p = this->packet;

    if(resistanceLevel==0)
    {
        // no packet sent = request for no resistance
        EXPECT_EQ(-1, this->packetLength);
        EXPECT_TRUE(p == nullptr);

        return;
    }

    // make sure the resistance is clipped
    if (resistanceLevel > trixterxdreamv1client::MaxResistance)
        resistanceLevel = trixterxdreamv1client::MaxResistance;

    EXPECT_EQ(6, this->packetLength);
    EXPECT_TRUE(p != nullptr);
    EXPECT_EQ(p[0], 0x6a);
    EXPECT_EQ(p[1], resistanceLevel);
    EXPECT_EQ(p[2], (resistanceLevel+60)%255);
    EXPECT_EQ(p[3], (resistanceLevel+90)%255);
    EXPECT_EQ(p[4], (resistanceLevel+120)%255);
    EXPECT_EQ(p[5], p[0]^p[1]^p[2]^p[3]^p[4]);

}


