//
//  ViewController.swift
//  ElecDemo
//
//  Created by NhatHM on 8/9/19.
//  Copyright © 2019 GST.PID. All rights reserved.
//

import UIKit

class ViewController: UIViewController {

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
    }

    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        WorkoutTracking.shared.authorizeHealthKit()
        WorkoutTracking.shared.observerHeartRateSamples()
        WatchKitConnection.shared.delegate = self
    }
}

extension ViewController: WatchKitConnectionDelegate {
    func didFinishedActiveSession() {
        WatchKitConnection.shared.sendMessage(message: ["username" : "nhathm" as AnyObject])
    }
}
