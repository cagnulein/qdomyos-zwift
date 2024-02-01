package com.che.zap.crypto

import java.security.KeyPair
import java.security.KeyPairGenerator
import java.security.interfaces.ECPrivateKey
import java.security.interfaces.ECPublicKey
import java.security.spec.ECGenParameterSpec

class LocalKeyProvider {

    private lateinit var pair: KeyPair

    fun getPublicKeyBytes(): ByteArray {
        return EncryptionUtils.publicKeyToByteArray(pair.public as ECPublicKey)
    }

    fun getPublicKey(): ECPublicKey {
        return pair.public as ECPublicKey
    }

    fun getPrivateKey(): ECPrivateKey {
        return pair.private as ECPrivateKey
    }

    private fun generateKeyPair() {
        val keyPairGenerator = KeyPairGenerator.getInstance("EC")
        keyPairGenerator.initialize(ECGenParameterSpec("secp256r1"))
        pair = keyPairGenerator.genKeyPair()
    }

    init {
        generateKeyPair()
    }
}