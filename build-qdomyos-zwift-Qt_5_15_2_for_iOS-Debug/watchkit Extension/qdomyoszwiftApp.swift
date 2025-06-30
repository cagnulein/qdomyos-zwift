//
//  qdomyoszwiftApp.swift
//  watchkit Extension
//
//  Created by Roberto Viola on 24/12/2020.
//  Updated for SwiftUI by Claude on 2025-06-30
//

import SwiftUI

@main
struct qdomyoszwiftApp: App {
    @SceneBuilder var body: some Scene {
        WindowGroup {
            ContentView()
        }

        WKNotificationScene(controller: NotificationController.self, category: "myCategory")
    }
}
