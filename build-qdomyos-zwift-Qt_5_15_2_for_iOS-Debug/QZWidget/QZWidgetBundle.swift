//
//  QZWidgetBundle.swift
//  QZWidget
//
//  Created by Roberto Viola on 04/10/25.
//

import WidgetKit
import SwiftUI

@main
struct QZWidgetBundle: WidgetBundle {
    var body: some Widget {
        QZWidget()
        QZWidgetControl()
        QZWidgetLiveActivity()
    }
}
