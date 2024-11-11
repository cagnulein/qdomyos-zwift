//
//  zwifthubbike.swift
//  qdomyoszwift
//
//  Created by Roberto Viola on 11/11/24.
//

import Foundation

@objc public class ZwiftHubBike : NSObject {
    @objc public static func inclinationCommand(inclination: Double) throws -> Data {
        var simulation = BLEReceiver_Zwift_SimulationParam()
        simulation.inclineX100 = Int32(inclination * 100.0)
        
        var command = BLEReceiver_Zwift_HubCommand()
        command.simulation = simulation
        
        let data = try command.serializedData()    
        var fullData = Data([0x04])
        fullData.append(data)
        
        return fullData
    }
    
    @objc public static func setGearCommand(gears: UInt32) throws -> Data {
        var physical = BLEReceiver_Zwift_PhysicalParam()
        physical.gearRatioX10000 = gears
        
        var command = BLEReceiver_Zwift_HubCommand()
        command.physical = physical
        
        let data = try command.serializedData()
        var fullData = Data([0x04])
        fullData.append(data)
        
        return fullData
    }
}
