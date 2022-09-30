//
//  Client.swift
//  MultiConnect
//
//  Created by michal on 29/11/2020.
//

import Foundation
import Network

class Browser {

    let browser: NWBrowser

    init() {
        let parameters = NWParameters()
        parameters.includePeerToPeer = true

        browser = NWBrowser(for: .bonjour(type: "_qz._tcp", domain: nil), using: parameters)
    }

    func start(handler: @escaping (NWBrowser.Result) -> Void) {
        browser.stateUpdateHandler = { newState in
            log("browser.stateUpdateHandler \(newState)")
        }
        browser.browseResultsChangedHandler = { results, changes in
            for result in results {
                if case NWEndpoint.service = result.endpoint {
                    handler(result)
                }
            }
        }
        browser.start(queue: .main)
    }
}
