package com.che.zap.crypto

import org.bouncycastle.crypto.digests.SHA256Digest
import org.bouncycastle.crypto.generators.HKDFBytesGenerator
import org.bouncycastle.crypto.params.HKDFParameters
import org.bouncycastle.util.BigIntegers
import java.math.BigInteger
import java.nio.ByteBuffer
import java.security.KeyFactory
import java.security.PrivateKey
import java.security.PublicKey
import java.security.interfaces.ECPublicKey
import java.security.spec.ECParameterSpec
import java.security.spec.ECPoint
import java.security.spec.ECPublicKeySpec
import javax.crypto.KeyAgreement

object EncryptionUtils {

    const val KEY_LENGTH = 32
    const val HKDF_LENGTH = 36
    const val MAC_LENGTH = 4

    fun publicKeyToByteArray(eCPublicKey: ECPublicKey): ByteArray {
        val affineX: BigInteger = eCPublicKey.w.affineX
        val affineXUnsigned: ByteArray = BigIntegers.asUnsignedByteArray((eCPublicKey.w.affineX.bitLength() + 7) / 8, affineX)
        val affineY: BigInteger = eCPublicKey.w.affineY
        val affineYUnsigned: ByteArray = BigIntegers.asUnsignedByteArray( (eCPublicKey.w.affineY.bitLength() + 7) / 8, affineY)
        return ByteBuffer.allocate(affineXUnsigned.size + affineYUnsigned.size)
            .put(affineXUnsigned)
            .put(affineYUnsigned)
            .array()
    }

    // used to generate a complete ECPublicKey from the 64 bytes received in the handshake
    fun generatePublicKey(publicKeyBytes: ByteArray, paramSpec: ECParameterSpec): ECPublicKey {
        val bitLength: Int = paramSpec.order.bitLength() / 8
        val i = bitLength + 0
        val byteRange = publicKeyBytes.copyOfRange(0, i)
        val bigInteger = BigInteger(1, byteRange)
        val byteRange2 = publicKeyBytes.copyOfRange(i, bitLength + i)
        val generatePublic: PublicKey = KeyFactory.getInstance("EC")
            .generatePublic(ECPublicKeySpec(ECPoint(bigInteger, BigInteger(1, byteRange2)), paramSpec))
        return generatePublic as ECPublicKey
    }

    fun generateSharedSecretBytes(privateKey: PrivateKey, serverPublicKey: PublicKey): ByteArray {
        val keyAgreement = KeyAgreement.getInstance("ECDH")
        keyAgreement.init(privateKey)
        keyAgreement.doPhase(serverPublicKey, true)
        return keyAgreement.generateSecret("ECDH").encoded
    }

    fun generateHKDFBytes(secretKey: ByteArray, salt: ByteArray): ByteArray {
        val digest = SHA256Digest()
        val hKDFParameters = HKDFParameters(secretKey, salt, ByteArray(0))
        val hKDFBytesGenerator = HKDFBytesGenerator(digest)
        hKDFBytesGenerator.init(hKDFParameters)

        val result = ByteArray(HKDF_LENGTH)
        hKDFBytesGenerator.generateBytes(result, 0, HKDF_LENGTH)
        return result
    }
}