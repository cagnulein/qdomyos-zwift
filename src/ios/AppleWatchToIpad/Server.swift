//
//  Server.swift
//  MultiConnect
//
//  Created by michal on 29/11/2020.
//

import Foundation
import Network
import UIKit

let server = try? Server()

class Server {

    let listener: NWListener

    var connections: [Connection] = []

    init() throws {
        let tcpOptions = NWProtocolTCP.Options()
        tcpOptions.enableKeepalive = true
        tcpOptions.keepaliveIdle = 2

        let parameters = NWParameters(tls: nil, tcp: tcpOptions)
        parameters.includePeerToPeer = true
        listener = try NWListener(using: parameters)
        
        listener.service = NWListener.Service(name: "server", type: "_superapp._tcp")
    }

    func start() {
        listener.stateUpdateHandler = { newState in
            log("listener.stateUpdateHandler \(newState)")
        }
        listener.newConnectionHandler = { [weak self] newConnection in
            log("listener.newConnectionHandler \(newConnection)")
            let connection = Connection(connection: newConnection)
            self?.connections += [connection]
        }
        listener.start(queue: .main)
    }

    func send() {
        connections.forEach {
            $0.send("super message from the server! \(Int(Date().timeIntervalSince1970))")
        }
    }
}
