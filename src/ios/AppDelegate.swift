//
//  AppDelegate.swift
//  ElecDemo
//
//  Created by NhatHM on 8/9/19.
//  Copyright © 2019 GST.PID. All rights reserved.
//

import UIKit
import CoreMotion

var pedometer = CMPedometer()

@objc public class virtualdevice_ios:NSObject {
    let v = BLEPeripheralManager()
}

@objc public class healthkit:NSObject {
    let w = watchAppStart()
    let SwiftDebug = swiftDebug()
    var liveActivityManager: LiveActivityManager?

    @objc public func request()
    {
        SwiftDebug.qtDebug("swift debug test")
        if #available(iOS 13.0, *) {
            Client.client.start()
        } else {
            // Fallback on earlier versions
        }
        if UIDevice.current.userInterfaceIdiom == .phone {
            Timer.scheduledTimer(timeInterval: 1, target: self, selector: #selector(self.updateHeartRate), userInfo: nil, repeats: true)
        }
        Server.server?.start()

        LocalNotificationHelper.requestPermission()
        WatchKitConnection.shared.startSession()

        if CMPedometer.isStepCountingAvailable() {
            pedometer.startUpdates(from: Date()) { pedometerData, error in
                guard let pedometerData = pedometerData, error == nil else { return }
                    print("\(pedometerData.numberOfSteps.intValue) STEP CAD.")
            }
        }

        // Initialize Live Activity manager
        if #available(iOS 16.1, *) {
            liveActivityManager = LiveActivityManager()
            print("Live Activity manager initialized")
        }

        //w.startWatchApp()
    }
    
    @objc public func heartRate() -> Int
    {
        return WatchKitConnection.currentHeartRate;
    }
    
    @objc public func stepCadence() -> Int
    {
        return WatchKitConnection.stepCadence;
    }

    @objc public func setSteps(steps: Int) -> Void
    {
        var sender: String
        if UIDevice.current.userInterfaceIdiom == .pad {
            sender = "PAD"
        } else {
            sender = "PHONE"
        }
        WatchKitConnection.steps = steps;
        Server.server?.send(createString(sender: sender))
    }
    
    @objc public func setDistance(distance: Double) -> Void
    {
		var sender: String
		if UIDevice.current.userInterfaceIdiom == .pad {
			sender = "PAD"
		} else {
			sender = "PHONE"
		}
        WatchKitConnection.distance = distance;
        Server.server?.send(createString(sender: sender))
    }
    
    @objc public func setKcal(kcal: Double) -> Void
    {
		var sender: String
		if UIDevice.current.userInterfaceIdiom == .pad {
			sender = "PAD"
		} else {
			sender = "PHONE"
		}
        WatchKitConnection.kcal = kcal;
        Server.server?.send(createString(sender: sender))
    }
    
    @objc public func setTotalKcal(totalKcal: Double) -> Void
    {
		var sender: String
		if UIDevice.current.userInterfaceIdiom == .pad {
			sender = "PAD"
		} else {
			sender = "PHONE"
		}
        WatchKitConnection.totalKcal = totalKcal;
        Server.server?.send(createString(sender: sender))
    }
    
    @objc public func setCadence(cadence: Double) -> Void
    {
        var sender: String
        if UIDevice.current.userInterfaceIdiom == .pad {
            sender = "PAD"
        } else {
            sender = "PHONE"
        }
        WatchKitConnection.cadence = cadence;
        Server.server?.send(createString(sender: sender))
    }
    
    @objc public func setSpeed(speed: Double) -> Void
    {
        var sender: String
        if UIDevice.current.userInterfaceIdiom == .pad {
            sender = "PAD"
        } else {
            sender = "PHONE"
        }
        WatchKitConnection.speed = speed;
        Server.server?.send(createString(sender: sender))
    }
    
    @objc public func setPower(power: Double) -> Void
    {
        var sender: String
        if UIDevice.current.userInterfaceIdiom == .pad {
            sender = "PAD"
        } else {
            sender = "PHONE"
        }
        WatchKitConnection.power = power;
        Server.server?.send(createString(sender: sender))
    }
    
    func createString(sender: String) -> String {
        return "SENDER=\(sender)#HR=\(WatchKitConnection.currentHeartRate)#KCAL=\(WatchKitConnection.kcal)#TOTALKCAL=\(WatchKitConnection.totalKcal)#BCAD=\(WatchKitConnection.cadence)#SPD=\(WatchKitConnection.speed)#PWR=\(WatchKitConnection.power)#CAD=\(WatchKitConnection.stepCadence)#ODO=\(WatchKitConnection.distance)#";
    }
    
    @objc func updateHeartRate() {
        var sender: String
        if UIDevice.current.userInterfaceIdiom == .pad {
            sender = "PAD"
        } else {
            sender = "PHONE"
        }
        Server.server?.send(createString(sender: sender))
    }
}
/*
@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    var window: UIWindow?
    var backgroundTask: UIBackgroundTaskIdentifier = .invalid

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        // Override point for customization after application launch.
        
        LocalNotificationHelper.requestPermission()
        
        WatchKitConnection.shared.startSession()
        return true
    }

    func applicationWillResignActive(_ application: UIApplication) {
        // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    }

    func applicationDidEnterBackground(_ application: UIApplication) {
        // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
        // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
        backgroundTask = UIApplication.shared.beginBackgroundTask { [unowned self] in
            //            UIApplication.shared.endBackgroundTask(backgroundTask)
            //            self?.backgroundTask = .invalid
            UIApplication.shared.endBackgroundTask(self.backgroundTask)
            self.backgroundTask = UIBackgroundTaskIdentifier.invalid
        }
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
        // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
    }

    func applicationDidBecomeActive(_ application: UIApplication) {
        // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    }

    func applicationWillTerminate(_ application: UIApplication) {
        // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    }


}

*/
