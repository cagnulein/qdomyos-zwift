//
//  PeerConnection.swift
//  MultiConnect
//
//  Created by michal on 29/11/2020.
//

import Foundation
import Network

var sharedConnection: Connection?

class Connection {

    let connection: NWConnection

    // outgoing connection
    init(endpoint: NWEndpoint) {
        log("PeerConnection outgoing endpoint: \(endpoint)")
        let tcpOptions = NWProtocolTCP.Options()
        tcpOptions.enableKeepalive = true
        tcpOptions.keepaliveIdle = 2

        let parameters = NWParameters(tls: nil, tcp: tcpOptions)
        parameters.includePeerToPeer = true
        connection = NWConnection(to: endpoint, using: parameters)
        start()
    }

    // incoming connection
    init(connection: NWConnection) {
        log("PeerConnection incoming connection: \(connection)")
        self.connection = connection
        start()
    }

    func start() {
        connection.stateUpdateHandler = { newState in
            log("connection.stateUpdateHandler \(newState)")
        }
        connection.start(queue: .main)
    }
}
