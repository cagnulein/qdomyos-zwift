
#include "gtest/gtest.h"

#include "devices/trixterxdreambike/trixterxdreamclient.h"


// The fixture for testing class Foo.
class TrixterXDreamPacketInterpreterTests : public ::testing::Test {
protected:
    uint8_t* packet;
    int packetLength;

	// You can remove any or all of the following functions if their bodies would
	// be empty.

    TrixterXDreamPacketInterpreterTests() {
		// You can do set-up work for each test here.
	}

    ~TrixterXDreamPacketInterpreterTests() override {
		// You can do clean-up work that doesn't throw exceptions here.
	}

    static uint32_t getTime();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	void SetUp() override {
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	void TearDown() override {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}

	// Class members declared here can be used by all tests in the test suite
	// for Foo.

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

