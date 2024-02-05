import Foundation
import CryptoKit
import CommonCrypto

enum EncryptionUtils {
    static let keyLength = 32
    static let hkdfLength = 36
    static let macLength = 4

    static func publicKeyToByteArray(publicKey: P256.KeyAgreement.PublicKey) -> Data {
        // Concatenate X and Y coordinates
        let x = publicKey.rawRepresentation[..<32] // First 32 bytes for X
        let y = publicKey.rawRepresentation[32...] // Last 32 bytes for Y
        return Data(x) + Data(y)
    }

    static func generatePublicKey(publicKeyBytes: Data, curve: P256.Curve = .P256) throws -> P256.KeyAgreement.PublicKey {
        // Assuming publicKeyBytes contains the X and Y coordinates concatenated
        return try P256.KeyAgreement.PublicKey(x963Representation: publicKeyBytes)
    }

    static func generateSharedSecretBytes(privateKey: P256.KeyAgreement.PrivateKey, publicKey: P256.KeyAgreement.PublicKey) -> Data {
        let sharedSecret = try! privateKey.sharedSecretFromKeyAgreement(with: publicKey)
        return sharedSecret.withUnsafeBytes { Data($0) }
    }

    static func generateHKDFBytes(secretKey: Data, salt: Data) -> Data {
        let hkdf = HKDF<SHA256>(salt: salt, sharedSecret: secretKey, info: Data(), outputByteCount: hkdfLength)
        return hkdf.expandedKey
    }
}

// Extension to perform HKDF expansion using CryptoKit
extension HKDF where HashFunction == SHA256 {
    init(salt: Data, sharedSecret: Data, info: Data, outputByteCount: Int) {
        self.init(secret: SymmetricKey(data: sharedSecret), salt: SymmetricKey(data: salt), info: info, outputByteCount: outputByteCount)
    }

    var expandedKey: Data {
        var result = Data(repeating: 0, count: outputByteCount)
        result.withUnsafeMutableBytes { buffer in
            _ = self.expand(into: buffer.bindMemory(to: UInt8.self))
        }
        return result
    }
}
