#pragma once

#include <gtest/gtest.h>

#include "devices/sramAXSController/sramAXSCrypto.h"
#include "devices/sramAXSController/sramAXSController.h"

TEST(SramAxsCryptoTest, DerivesKnownSramBondValues) {
    const QByteArray privateKey = QByteArray::fromHex("6418b20cb4e1d4cf4af19b184aff1d2a");
    const QByteArray devicePublicKey = QByteArray::fromHex("9ac11ad0a4f6c2b99c5559e2d210c410");
    EXPECT_EQ(sramaxscrypto::computePublicKey(privateKey).toHex(), QByteArray("297ca1db5827261af813875fd09800b0"));
    const QByteArray shared = sramaxscrypto::computeSharedSecret(privateKey, devicePublicKey);
    EXPECT_EQ(shared.toHex(), QByteArray("55406a336a328156b81019d7ac3d5d24"));
    bool authenticated = false;
    const QByteArray key = sramaxscrypto::decryptTransportedKey(
        shared, QByteArray::fromHex("8d7a16ed42128ee445b9864f20324a0e0b9e1982be7a9ad3cf611f453696fa8d2f24f1878c44eb7f77ff5b4a3c616395"),
        &authenticated);
    EXPECT_TRUE(authenticated);
    EXPECT_EQ(key.toHex(), QByteArray("b0690781867fde13ac1b9d30bbb4004f"));
}

TEST(SramAxsCryptoTest, MatchesPublishedEaxVectorAndRejectsTampering) {
    const QByteArray key = QByteArray::fromHex("55406a336a328156b81019d7ac3d5d24");
    const QByteArray nonce = QByteArray::fromHex("8d7a16ed42128ee445b9864f20324a0e");
    const QByteArray sealed = sramaxscrypto::eaxEncrypt(key, nonce, QByteArray::fromHex("b0690781867fde13ac1b9d30bbb4004f"));
    EXPECT_EQ(sealed.toHex(), QByteArray("0b9e1982be7a9ad3cf611f453696fa8d2f24f1878c44eb7f77ff5b4a3c616395"));
    bool authenticated = false;
    EXPECT_TRUE(sramaxscrypto::eaxDecrypt(key, nonce, sealed, &authenticated).isEmpty());
    EXPECT_TRUE(authenticated);
    QByteArray tampered = sealed;
    tampered[tampered.size() - 1] = static_cast<char>(tampered.at(tampered.size() - 1) ^ 1);
    authenticated = true;
    EXPECT_TRUE(sramaxscrypto::eaxDecrypt(key, nonce, tampered, &authenticated).isEmpty());
    EXPECT_FALSE(authenticated);
}

TEST(SramAxsProtocolTest, DecodesRearGearAndDoesNotEmitOnBaseline) {
    EXPECT_EQ(sramaxscontroller::decodeRearGear(QByteArray::fromHex("a00101a80107b0010c")), 7);
    EXPECT_EQ(sramaxscontroller::decodeRearGear(QByteArray::fromHex("a80108")), 8);
    EXPECT_EQ(sramaxscontroller::decodeRearGear(QByteArray::fromHex("a80100")), -1);
    EXPECT_EQ(sramaxscontroller::virtualGearDirectionForRearDelta(-1), 1);
    EXPECT_EQ(sramaxscontroller::virtualGearDirectionForRearDelta(1), -1);
}
