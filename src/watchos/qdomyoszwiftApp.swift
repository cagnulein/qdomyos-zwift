//
//  qdomyoszwiftApp.swift
//  watchkit Extension
//
//  Created by Roberto Viola on 24/12/2020.
//  Updated by Claude Code for SwiftUI Redesign
//

import SwiftUI

@main
struct qdomyoszwiftApp: App {
    @SceneBuilder var body: some Scene {
        WindowGroup {
            NavigationView {
                WorkoutContentView()
            }
        }

        WKNotificationScene(controller: NotificationController.self, category: "myCategory")
    }
}
