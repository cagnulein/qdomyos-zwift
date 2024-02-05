import Foundation
import CryptoKit

class ZapCrypto {
    private let localKeyProvider: LocalKeyProvider
    private var encryptionKeyBytes: Data?
    private var ivBytes: Data?
    private var counter: Int = 0

    init(localKeyProvider: LocalKeyProvider) {
        self.localKeyProvider = localKeyProvider
    }

    func initialise(devicePublicKeyBytes: Data) {
        let hkdfBytes: Data = generateHmacKeyDerivationFunctionBytes(devicePublicKeyBytes: devicePublicKeyBytes)
        self.encryptionKeyBytes = hkdfBytes.subdata(in: 0..<EncryptionUtils.KEY_LENGTH)
        self.ivBytes = hkdfBytes.subdata(in: 32..<EncryptionUtils.HKDF_LENGTH)
    }

    func encrypt(data: Data) -> Data? {
        guard let encryptionKeyBytes = encryptionKeyBytes, let ivBytes = ivBytes else {
            assertionFailure("Not initialised")
            return nil
        }

        let counterValue = counter
        self.counter += 1

        let nonceBytes: Data = createNonce(iv: ivBytes, messageCounter: counterValue)
        return encryptDecrypt(encrypt: true, nonceBytes: nonceBytes, data: data)?.prependCounter(counterValue)
    }

    func decrypt(counterArray: Data, payload: Data) -> Data? {
        guard let ivBytes = ivBytes else {
            assertionFailure("Not initialised")
            return nil
        }

        let nonceBytes: Data = ivBytes + counterArray
        return encryptDecrypt(encrypt: false, nonceBytes: nonceBytes, data: payload)
    }

    private func encryptDecrypt(encrypt: Bool, nonceBytes: Data, data: Data) -> Data? {
        guard let encryptionKey = SymmetricKey(data: encryptionKeyBytes!) else {
            return nil
        }

        let sealedBox: AES.GCM.SealedBox
        do {
            if encrypt {
                sealedBox = try AES.GCM.seal(data, using: encryptionKey, nonce: AES.GCM.Nonce(data: nonceBytes))
            } else {
                sealedBox = try AES.GCM.SealedBox(nonce: AES.GCM.Nonce(data: nonceBytes), ciphertext: data, tag: Data())
                return try AES.GCM.open(sealedBox, using: encryptionKey)
            }
            return sealedBox.ciphertext + sealedBox.tag
        } catch {
            print(error)
            return nil
        }
    }

    private func generateHmacKeyDerivationFunctionBytes(devicePublicKeyBytes: Data) -> Data {
        let serverPublicKey = localKeyProvider.generatePublicKey(devicePublicKeyBytes: devicePublicKeyBytes)
        let sharedSecretBytes = localKeyProvider.generateSharedSecretBytes(serverPublicKey: serverPublicKey)
        let salt = serverPublicKey.toData() + localKeyProvider.getPublicKeyBytes()
        return EncryptionUtils.generateHKDFBytes(sharedSecretBytes: sharedSecretBytes, salt: salt)
    }

    private func createNonce(iv: Data, messageCounter: Int) -> Data {
        return iv + createCounterBytes(messageCounter: messageCounter)
    }

    private func createCounterBytes(messageCounter: Int) -> Data {
        var counterValue = messageCounter.bigEndian
        return Data(bytes: &counterValue, count: MemoryLayout.size(ofValue: counterValue))
    }
}

extension Data {
    func prependCounter(_ counter: Int) -> Data {
        var counterValue = counter.bigEndian
        var data = Data(bytes: &counterValue, count: MemoryLayout.size(ofValue: counterValue))
        data.append(contentsOf: self)
        return data
    }
}
