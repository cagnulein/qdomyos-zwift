//
//  PeerConnection.swift
//  MultiConnect
//
//  Created by michal on 29/11/2020.
//

import Foundation
import Network

extension String {
    func slice(from: String, to: String) -> String? {
        return (from.isEmpty ? startIndex..<startIndex : range(of: from)).flatMap { fromRange in
            (to.isEmpty ? endIndex..<endIndex : range(of: to, range: fromRange.upperBound..<endIndex)).map({ toRange in
                String(self[fromRange.upperBound..<toRange.lowerBound])
            })
        }
    }
}

class Connection {

    let connection: NWConnection

    // outgoing connection
    init(endpoint: NWEndpoint) {
        print("PeerConnection outgoing endpoint: \(endpoint)")
        let tcpOptions = NWProtocolTCP.Options()
        tcpOptions.enableKeepalive = true
        tcpOptions.keepaliveIdle = 2
        tcpOptions.connectionTimeout = 5

        let parameters = NWParameters(tls: nil, tcp: tcpOptions)
        parameters.includePeerToPeer = true
        connection = NWConnection(to: endpoint, using: parameters)
        start()
    }

    // incoming connection
    init(connection: NWConnection) {
        print("PeerConnection incoming connection: \(connection)")
        self.connection = connection
        start()
    }

    func start() {
        connection.stateUpdateHandler = { newState in
            print("connection.stateUpdateHandler \(newState)")
            switch newState {
            case .ready:
                self.receiveMessage()
            case .failed(let error):
                self.connection.stateUpdateHandler = nil
                self.connection.cancel()
                print("Server error\(error)")
            case .setup:
                print("Server setup.")
            case .waiting(_):
                print("Server waiting.")
            case .preparing:
                print("Server preparing.")
            case .cancelled:
                print("Server cancelled.")
            @unknown default:
                print("Server DEFAULT.")
            }
        }
        connection.start(queue: .main)
    }

    func send(_ message: String) {
        print("sending \(message)")
        connection.send(content: message.data(using: .utf8), contentContext: .defaultMessage, isComplete: true, completion: .contentProcessed({ error in
            print("Connection send error: \(String(describing: error))")
        }))
    }

    func receiveMessage() {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 100) { data, _, _, _ in
            if let data = data,
               let message = String(data: data, encoding: .utf8) {
                print("Connection receiveMessage message: \(message)")
				if message.contains("SENDER=") {
					let sender = message.slice(from: "SENDER=", to: "#")
                    if sender?.contains("PHONE") ?? false && message.contains("HR=") {
                        let hr : String = message.slice(from: "HR=", to: "#") ?? ""
                        WatchKitConnection.currentHeartRate = (Int(hr) ?? 0)
					}
                    if sender?.contains("PAD") ?? false && message.contains("KCAL=") {
                        let kcal : String = message.slice(from: "KCAL=", to: "#") ?? ""
                        WatchKitConnection.kcal = (Double(kcal) ?? 0)
                    }
                    if sender?.contains("PAD") ?? false && message.contains("ODO=") {
                        let odo : String = message.slice(from: "ODO=", to: "#") ?? ""
                        WatchKitConnection.distance = (Double(odo) ?? 0)
                    }
				}
            }
            if(self.connection.state == .ready) {
                self.receiveMessage()
            }
        }
    }
}
