//
//  Server.swift
//  MultiConnect
//
//  Created by michal on 29/11/2020.
//

import Foundation
import Network
import UIKit

class Server {

    static let server = try? Server()
    let listener: NWListener
    let SwiftDebug = swiftDebug()

    var connections: [Connection] = []

    init() throws {
        let tcpOptions = NWProtocolTCP.Options()
        tcpOptions.enableKeepalive = true
        tcpOptions.keepaliveIdle = 2
        tcpOptions.connectionTimeout = 5

        let parameters = NWParameters(tls: nil, tcp: tcpOptions)
        parameters.includePeerToPeer = true
        listener = try NWListener(using: parameters)
        
        if UIDevice.current.userInterfaceIdiom == .pad {
            listener.service = NWListener.Service(name: "server", type: "_qz_ipad._tcp")
        } else {
            listener.service = NWListener.Service(name: "server", type: "_qz_iphone._tcp")
        }
    }

    func start() {
        listener.stateUpdateHandler = { newState in
            SwiftDebug.qtDebug("listener.stateUpdateHandler \(newState)")
        }
        listener.newConnectionHandler = { [weak self] newConnection in
            SwiftDebug.qtDebug("listener.newConnectionHandler \(newConnection)")
            let connection = Connection(connection: newConnection)
            self?.connections += [connection]
        }
        listener.start(queue: .main)
    }

    func send(_ message: String) {
        SwiftDebug.qtDebug("Server.send \(message) \(connections)")
        connections.forEach {
            if($0.connection.state == .ready) {
                $0.send(message)
            }
        }
    }
}
