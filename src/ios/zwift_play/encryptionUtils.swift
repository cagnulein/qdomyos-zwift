import Foundation
import CryptoSwift

struct EncryptionUtils {
    static let keyLength = 32
    static let hkdfLength = 36
    static let macLength = 4

    // Converti una chiave pubblica EC in array di byte
    static func publicKeyToByteArray(publicKey: P256.KeyAgreement.PublicKey) -> Data {
        // Assumendo che `publicKey` sia una rappresentazione simile di CryptoKit's P256,
        // la conversione diretta alla rappresentazione di byte è già disponibile
        return publicKey.rawRepresentation
    }

    // Genera una chiave pubblica EC dai byte ricevuti
    static func generatePublicKey(publicKeyBytes: Data) throws -> P256.KeyAgreement.PublicKey {
        // Assumendo l'uso di CryptoKit per la generazione della chiave pubblica
        return try P256.KeyAgreement.PublicKey(x963Representation: publicKeyBytes)
    }

    // Genera byte di segreto condiviso
    static func generateSharedSecretBytes(privateKey: P256.KeyAgreement.PrivateKey, publicKey: P256.KeyAgreement.PublicKey) -> Data {
        let sharedSecret = try! privateKey.sharedSecretFromKeyAgreement(with: publicKey)
        // Direttamente in formato Data
        return sharedSecret.withUnsafeBytes { Data($0) }
    }

    // Genera byte usando HKDF
    static func generateHKDFBytes(secretKey: Data, salt: Data) -> Data {
        // Utilizzo di HKDF in CryptoSwift
        let keyMaterial = secretKey.bytes
        let saltBytes = salt.bytes
        let hkdf = HKDF(key: keyMaterial, salt: saltBytes, variant: .sha2(.sha256))
        let derivedKey = try! hkdf.calculate(info: [], outputLength: hkdfLength)
        return Data(derivedKey)
    }
}
