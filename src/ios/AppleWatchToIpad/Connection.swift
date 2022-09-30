//
//  PeerConnection.swift
//  MultiConnect
//
//  Created by michal on 29/11/2020.
//

import Foundation
import Network

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
            if case .ready = newState {
                self.receiveMessage()
            }
        }
        connection.start(queue: .main)
    }

    func send(_ message: String) {
        connection.send(content: message.data(using: .utf8), contentContext: .defaultMessage, isComplete: true, completion: .contentProcessed({ error in
            log("Connection send error: \(String(describing: error))")
        }))
    }

	extension String {
		func slice(from: String, to: String) -> String? {
			return (from.isEmpty ? startIndex..<startIndex : range(of: from)).flatMap { fromRange in
				(to.isEmpty ? endIndex..<endIndex : range(of: to, range: fromRange.upperBound..<endIndex)).map({ toRange in
					String(self[fromRange.upperBound..<toRange.lowerBound])
				})
			}
		}
	}

    func receiveMessage() {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 100) { data, _, _, _ in
            if let data = data,
               let message = String(data: data, encoding: .utf8) {
                log("Connection receiveMessage message: \(message)")
				if message.contains("SENDER=") {
					let sender = message.slice(from: "SENDER=", to: "#")
					if sender.contains("PAD") and message.contains("HR=") {
						let hr = message.slice(from: "HR=", to: "#")
						WatchKitConnection.currentHeartRate = hr
					}
				}
            }
            self.receiveMessage()
        }
    }
}
