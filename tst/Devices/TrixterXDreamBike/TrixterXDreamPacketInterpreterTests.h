
#include "gtest/gtest.h"

#include "devices/trixterxdreambike/trixterxdreamclient.h"


class TrixterXDreamPacketInterpreterTests : public ::testing::Test {
protected:
    uint8_t* packet;
    int packetLength;

    static uint32_t getTime();

    void TestInput(std::string input, uint8_t expectedHR, uint8_t expectedSteering);

    void TestResistance(trixterxdreamclient * tx1, uint8_t resistanceLevel);

};

//int main(int argc, char** argv) {
//	::testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
//}

TEST_F(TrixterXDreamPacketInterpreterTests, ValidPacket) {

	TestInput("56b6a00000000000000000000000000016b6a7f45000000000000000000000050006a", 0x50, 0x7f);
}

TEST_F(TrixterXDreamPacketInterpreterTests, SendResistance) {

    trixterxdreamclient tx1;

	tx1.set_GetTime(getTime);
	auto device = this;
	tx1.set_WriteBytes([device](uint8_t* bytes, int length)->void { device->packet = bytes; device->packetLength = length; });

	for (int i = 0; i <= 255; i++)
		TestResistance(&tx1, static_cast<uint8_t>(i));	
}

