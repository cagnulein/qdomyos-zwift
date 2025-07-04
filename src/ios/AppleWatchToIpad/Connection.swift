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
    let SwiftDebug = swiftDebug()

    // outgoing connection
    init(endpoint: NWEndpoint) {
        SwiftDebug.qtDebug("PeerConnection outgoing endpoint: \(endpoint)")
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
        SwiftDebug.qtDebug("PeerConnection incoming connection: \(connection)")
        self.connection = connection
        start()
    }

    func start() {
        connection.stateUpdateHandler = { newState in
            self.SwiftDebug.qtDebug("connection.stateUpdateHandler \(newState)")
            switch newState {
            case .ready:
                self.receiveMessage()
            case .failed(let error):
                self.connection.stateUpdateHandler = nil
                self.connection.cancel()
                self.SwiftDebug.qtDebug("Server error\(error)")
            case .setup:
                self.SwiftDebug.qtDebug("Server setup.")
            case .waiting(_):
                self.SwiftDebug.qtDebug("Server waiting.")
            case .preparing:
                self.SwiftDebug.qtDebug("Server preparing.")
            case .cancelled:
                self.SwiftDebug.qtDebug("Server cancelled.")
            @unknown default:
                self.SwiftDebug.qtDebug("Server DEFAULT.")
            }
        }
        connection.start(queue: .main)
    }

    func send(_ message: String) {
        self.SwiftDebug.qtDebug("sending \(message)")
        connection.send(content: message.data(using: .utf8), contentContext: .defaultMessage, isComplete: true, completion: .contentProcessed({ error in
            if error != nil {
                self.SwiftDebug.qtDebug("Connection send error: \(String(describing: error))")
            }
        }))
    }

    func receiveMessage() {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 100) { data, _, _, _ in
            if let data = data,
               let message = String(data: data, encoding: .utf8) {
                self.SwiftDebug.qtDebug("Connection receiveMessage message: \(message)")
				if message.contains("SENDER=") {
					let sender = message.slice(from: "SENDER=", to: "#")
                    if sender?.contains("PHONE") ?? false && message.contains("HR=") {
                        let hr : String = message.slice(from: "HR=", to: "#") ?? ""
                        WatchKitConnection.currentHeartRate = (Int(hr) ?? WatchKitConnection.currentHeartRate)
					}
                    if sender?.contains("PHONE") ?? false && message.contains("CAD=") {
                        let cad : String = message.slice(from: "CAD=", to: "#") ?? ""
                        WatchKitConnection.stepCadence = (Int(cad) ?? WatchKitConnection.stepCadence)
                    }
                    if sender?.contains("PAD") ?? false && message.contains("KCAL=") {
                        let kcal : String = message.slice(from: "KCAL=", to: "#") ?? ""
                        WatchKitConnection.kcal = (Double(kcal) ?? WatchKitConnection.kcal)
                    }
                    if sender?.contains("PAD") ?? false && message.contains("ODO=") {
                        let odo : String = message.slice(from: "ODO=", to: "#") ?? ""
                        WatchKitConnection.distance = (Double(odo) ?? WatchKitConnection.distance)
                    }
                    if sender?.contains("PAD") ?? false && message.contains("BCAD=") {
                        let cad : String = message.slice(from: "BCAD=", to: "#") ?? ""
                        WatchKitConnection.cadence = (Double(cad) ?? WatchKitConnection.cadence)
                    }
                    if sender?.contains("PAD") ?? false && message.contains("SPD=") {
                        let spd : String = message.slice(from: "SPD=", to: "#") ?? ""
                        WatchKitConnection.speed = (Double(spd) ?? WatchKitConnection.speed)
                    }
                    if sender?.contains("PAD") ?? false && message.contains("PWR=") {
                        let pwr : String = message.slice(from: "PWR=", to: "#") ?? ""
                        WatchKitConnection.power = (Double(pwr) ?? WatchKitConnection.power)
                    }
                    if sender?.contains("PAD") ?? false && message.contains("INCL=") {
                        let incl : String = message.slice(from: "INCL=", to: "#") ?? ""
                        WatchKitConnection.inclination = (Double(incl) ?? WatchKitConnection.inclination)
                    }
				}
            }
            if(self.connection.state == .ready) {
                self.receiveMessage()
            }
        }
    }
}
