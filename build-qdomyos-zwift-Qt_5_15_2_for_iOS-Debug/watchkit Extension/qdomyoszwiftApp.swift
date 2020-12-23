//
//  qdomyoszwiftApp.swift
//  watchkit Extension
//
//  Created by Roberto Viola on 23/12/2020.
//

import SwiftUI

@main
struct qdomyoszwiftApp: App {
    @SceneBuilder var body: some Scene {
        WindowGroup {
            NavigationView {
                ContentView()
            }
        }

        WKNotificationScene(controller: NotificationController.self, category: "myCategory")
    }
}
