//
//  Client.swift
//  MultiConnect
//
//  Created by michal on 29/11/2020.
//

import Foundation
import Network
import UIKit

@available(iOS 13.0, *)
class Browser {

    let browser: NWBrowser

    init() {
        let parameters = NWParameters()
        parameters.includePeerToPeer = true

        if UIDevice.current.userInterfaceIdiom == .pad {
            browser = NWBrowser(for: .bonjour(type: "_qz_iphone._tcp", domain: nil), using: parameters)
        } else {
            browser = NWBrowser(for: .bonjour(type: "_qz_ipad._tcp", domain: nil), using: parameters)
        }
    }

    func start(handler: @escaping (NWBrowser.Result) -> Void) {
        browser.stateUpdateHandler = { newState in
            print("browser.stateUpdateHandler \(newState)")
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
