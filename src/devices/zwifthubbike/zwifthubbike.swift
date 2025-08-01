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
    
    public static func ridingData(power: UInt32, cadence: UInt32, speed: Double, HR: UInt32, unkown1: UInt32, unkown2: UInt32) throws -> Data {
        var physical = BLEReceiver_Zwift_HubRidingData()
        physical.cadence = cadence
        physical.power = power
        physical.speedX100 = UInt32(speed * 100.0)
        physical.hr = HR
        physical.unknown1 = unkown1
        physical.unknown2 = unkown2
        
        let data = try physical.serializedData()
        var fullData = Data([0x03])
        fullData.append(data)
        
        return fullData
    }
    
    @objc public static func getPowerFromBuffer(buffer: Data) -> UInt32 {
        var physical = BLEReceiver_Zwift_HubRidingData()
        do {
            try physical.merge(serializedData: buffer)
            return physical.power
        } catch {
            var SwiftDebug = swiftDebug()
            SwiftDebug.qtDebug("Error in getPowerFromBuffer: \(error)")
            return 0
        }
    }

    @objc public static func getCadenceFromBuffer(buffer: Data) -> UInt32 {
        var physical = BLEReceiver_Zwift_HubRidingData()
        do {
            try physical.merge(serializedData: buffer)
            return physical.cadence
        } catch {
            var SwiftDebug = swiftDebug()
            SwiftDebug.qtDebug("Error in getCadenceFromBuffer: \(error)")
            return 0
        }
    }
}
