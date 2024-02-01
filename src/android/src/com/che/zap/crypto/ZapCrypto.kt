package com.che.zap.crypto

import com.che.zap.crypto.EncryptionUtils.HKDF_LENGTH
import com.che.zap.crypto.EncryptionUtils.KEY_LENGTH
import com.che.zap.crypto.EncryptionUtils.MAC_LENGTH
import com.che.zap.crypto.EncryptionUtils.generateHKDFBytes
import com.che.zap.crypto.EncryptionUtils.generatePublicKey
import com.che.zap.crypto.EncryptionUtils.generateSharedSecretBytes
import com.che.zap.crypto.EncryptionUtils.publicKeyToByteArray
import org.bouncycastle.crypto.engines.AESEngine
import org.bouncycastle.crypto.modes.CCMBlockCipher
import org.bouncycastle.crypto.params.AEADParameters
import org.bouncycastle.crypto.params.KeyParameter
import java.nio.ByteBuffer
import java.security.interfaces.ECPublicKey

class ZapCrypto(private val localKeyProvider: LocalKeyProvider) {

    private val aesEngine = AESEngine()

    private var encryptionKeyBytes: ByteArray? = null
    private var ivBytes: ByteArray? = null
    private var counter: Int = 0

    fun initialise(devicePublicKeyBytes: ByteArray) {
        val hkdfBytes: ByteArray = generateHmacKeyDerivationFunctionBytes(devicePublicKeyBytes)
        this.encryptionKeyBytes = hkdfBytes.copyOfRange(0, KEY_LENGTH)
        this.ivBytes = hkdfBytes.copyOfRange(32, HKDF_LENGTH)
    }

    fun encrypt(data: ByteArray): ByteArray {

        assert(encryptionKeyBytes != null && ivBytes != null) { "Not initialised" }

        val counterValue = counter
        this.counter++

        val nonceBytes: ByteArray = createNonce(ivBytes!!, counterValue)
        val encrypted = encryptDecrypt(true, nonceBytes, data)
        return createCounterBytes(counterValue).plus(encrypted)
    }

    fun decrypt(counterArray: ByteArray, payload: ByteArray): ByteArray {

        assert(encryptionKeyBytes != null && ivBytes != null) { "Not initialised" }

        val nonceBytes: ByteArray = ivBytes!!.plus(counterArray)
        return encryptDecrypt(false, nonceBytes, payload)
    }

    private fun encryptDecrypt(encrypt: Boolean, nonceBytes: ByteArray, data: ByteArray): ByteArray {
        val aeadParameters = AEADParameters(KeyParameter(encryptionKeyBytes), MAC_LENGTH * 8, nonceBytes)
        val ccmBlockCipher = CCMBlockCipher(aesEngine)
        ccmBlockCipher.init(encrypt, aeadParameters)
        val processed = ByteArray(ccmBlockCipher.getOutputSize(data.size))
        ccmBlockCipher.processBytes(data, 0, data.size, processed, 0)
        ccmBlockCipher.doFinal(processed, 0)
        return processed
    }

    private fun generateHmacKeyDerivationFunctionBytes(devicePublicKeyBytes: ByteArray): ByteArray {
        val serverPublicKey: ECPublicKey = generatePublicKey(devicePublicKeyBytes, localKeyProvider.getPublicKey().params)
        val sharedSecretBytes: ByteArray = generateSharedSecretBytes(localKeyProvider.getPrivateKey(), serverPublicKey)
        val salt = publicKeyToByteArray(serverPublicKey).plus(localKeyProvider.getPublicKeyBytes())
        return generateHKDFBytes(sharedSecretBytes, salt)
    }

    private fun createNonce(iv: ByteArray, messageCounter: Int): ByteArray {
        return iv.plus(createCounterBytes(messageCounter))
    }

    private fun createCounterBytes(messageCounter: Int): ByteArray {
        return ByteBuffer.allocate(Int.SIZE_BYTES)
            .putInt(messageCounter)
            .array()
    }

}