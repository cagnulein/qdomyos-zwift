//
//  Client.swift
//  MultiConnect
//
//  Created by michal on 30/11/2020.
//

import Foundation

@available(iOS 13.0, *)
class Client {

    static let client = Client()
    let browser = Browser()

    var connection: Connection?

    func start() {
        browser.start { [weak self] result in
            /*guard let self = self,
                  self.connection == nil else { return }*/
            print("client.handler result: \(result)")
            self?.connection = Connection(endpoint: result.endpoint)
        }
    }

}
