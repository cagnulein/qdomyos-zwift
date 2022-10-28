//
//  Dircon.swift
//  qdomyoszwift
//
//  Created by Roberto Viola on 28/10/22.
//

import Foundation
import Network
import UIKit

class Dircon {
    
    let listener: NWListener
    var connections: [Connection] = []
    
    init(name: String, port: UInt16, macAddress: String, serialNumber: String, bleServiceUuids: String) throws {
        let o = NWProtocolUDP.Options()
        let parameters = NWParameters(dtls: nil, udp: o)
        parameters.includePeerToPeer = true
        //parameters.requiredLocalEndpoint = NWEndpoint.hostPort(host: NWEndpoint.Host(name + "H"), port: NWEndpoint.Port(rawValue: port)!)
        parameters.allowFastOpen = true
        parameters.acceptLocalOnly = true
        listener = try NWListener(using: parameters, on: NWEndpoint.Port(rawValue: port)!)
        let records = ["mac-address": macAddress, "serial-number": serialNumber, "ble-service-uuids" : bleServiceUuids]
        if #available(iOS 13.0, *) {
            listener.service = NWListener.Service(name: name, type: "_wahoo-fitness-tnp._tcp", domain: "local", txtRecord: NWTXTRecord(records))
        } else {
            // Fallback on earlier versions
        }
    }
    
    func start() -> Bool {
        if(listener.service == nil) {
            return false
        }
        
        listener.stateUpdateHandler = { newState in
            print("listener.stateUpdateHandler \(newState)")
        }
        listener.newConnectionHandler = { [weak self] newConnection in
            print("listener.newConnectionHandler \(newConnection)")
            let connection = Connection(connection: newConnection)
            self?.connections += [connection]
        }
        listener.start(queue: .main)
        
        return true
    }
}
