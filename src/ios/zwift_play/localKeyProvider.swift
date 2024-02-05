import Foundation
import CryptoKit

class LocalKeyProvider {
    private var keyPair: P256.KeyAgreement.PrivateKey?

    init() {
        generateKeyPair()
    }

    func getPublicKeyBytes() -> Data {
        guard let publicKey = keyPair?.publicKey else {
            fatalError("Key pair not initialized correctly.")
        }
        return publicKey.rawRepresentation
    }

    func getPublicKey() -> P256.KeyAgreement.PublicKey {
        guard let publicKey = keyPair?.publicKey else {
            fatalError("Key pair not initialized correctly.")
        }
        return publicKey
    }

    func getPrivateKey() -> P256.KeyAgreement.PrivateKey {
        guard let keyPair = keyPair else {
            fatalError("Key pair not initialized correctly.")
        }
        return keyPair
    }

    private func generateKeyPair() {
        keyPair = P256.KeyAgreement.PrivateKey()
    }
}
