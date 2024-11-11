//
//  zwifthubbike.swift
//  qdomyoszwift
//
//  Created by Roberto Viola on 11/11/24.
//

import Foundation

@objc public class ZwiftHubBike : NSObject {
    @objc public static func inclinationCommand(inclination: Double) throws -> Data {
        // Creiamo i parametri di simulazione con solo pendenza
        var simulation = BLEReceiver_Zwift_SimulationParam()
        simulation.inclineX100 = Int32(inclination * 100.0)
        
        // Creiamo il comando principale solo con simulation
        var command = BLEReceiver_Zwift_HubCommand()
        command.simulation = simulation
        
        // Serializziamo in dati binari
        let data = try command.serializedData()
        
        // Se vuoi vedere l'hex string:
        let hexString = data.map { String(format: "%02x", $0) }.joined()
        print("Hex data: \(hexString)")
        
        // Aggiungi il command code 0x04
        var fullData = Data([0x04])
        fullData.append(data)
        
        return fullData
    }
}
