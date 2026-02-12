//
//  zwift_protobuf_layer.swift
//  qdomyoszwift
//
//  Created by Roberto Viola on 30/12/23.
//

import Foundation
import SwiftProtobuf

@objc public class zwift_protobuf_layer : NSObject {
    var player: PlayerState!
    let SwiftDebug = swiftDebug()
    
    @objc public func getPlayerState(value: Data)  {
        do {
            //let hexString = value.map { String(format: "%02x", $0) }.joined()
            //SwiftDebug.qtDebug("HEX \(hexString)")
            let decodedInfo = try PlayerState(serializedData: value)
            player = decodedInfo
        } catch {
            SwiftDebug.qtDebug("An error occurred: \(error)")
        }
    }
    
    @objc public func getAltitude() -> Float  {
        return player.altitude
    }
    
    @objc public func getDistance() -> Int32  {
        return player.distance
    }
    
    @objc public func getLatitude() -> Float  {
        return player.x
    }
    
    @objc public func getLongitude() -> Float  {
        return player.y
    }
    
   /*
    @objc public init() {
      super.init()
    }*/

}
