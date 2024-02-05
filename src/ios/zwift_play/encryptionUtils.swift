import CryptoKit
import Foundation

enum EncryptionUtils {
    static let keyLength = 32
    static let hkdfLength = 36
    static let macLength = 4

    // Converti una chiave pubblica P-256 in Data concatenando le coordinate X e Y
    static func publicKeyToByteArray(publicKey: P256.KeyAgreement.PublicKey) -> Data {
        return publicKey.rawRepresentation
    }

    // Genera una chiave pubblica P-256 da Data
    static func generatePublicKey(publicKeyBytes: Data) throws -> P256.KeyAgreement.PublicKey {
        return try P256.KeyAgreement.PublicKey(x963Representation: publicKeyBytes)
    }

    // Genera byte del segreto condiviso utilizzando ECDH
    static func generateSharedSecretBytes(privateKey: P256.KeyAgreement.PrivateKey, publicKey: P256.KeyAgreement.PublicKey) -> Data {
        let sharedSecret = try! privateKey.sharedSecretFromKeyAgreement(with: publicKey)
        return sharedSecret.withUnsafeBytes { Data($0) }
    }

    // Utilizza HKDF per derivare chiavi da un segreto condiviso
    static func generateHKDFBytes(secretKey: Data, salt: Data) -> Data {
        let symmetricKey = SymmetricKey(data: secretKey)
        let hkdfDerivedBytes = HKDF<SHA256>.deriveKey(inputKeyMaterial: symmetricKey, salt: salt, info: Data(), outputByteCount: hkdfLength)
        return Data(hkdfDerivedBytes)
    }
}
