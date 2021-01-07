//
//  BLEPeripheralManager.swift
//  BLEVirtualHRM
//
//  Created by Manish Kumar on 2019-03-05.
//  Copyright © 2019 Manish Kumar. All rights reserved.
//

import CoreBluetooth

protocol BLEPeripheralManagerDelegate {
  func BLEPeripheralManagerDidSendValue(_ heartRateBPM: UInt8)
}

/// CBUUID will automatically convert this 16-bit number to 128-bit UUID
let heartRateServiceUUID = CBUUID(string: "0x180D")
let heartRateCharacteristicUUID = CBUUID(string: "2A37")

class BLEPeripheralManager: NSObject, CBPeripheralManagerDelegate {
  private var peripheralManager: CBPeripheralManager!
  private var heartRateService: CBMutableService!
  private var heartRateCharacteristic: CBMutableCharacteristic!
  private var notificationTimer: Timer!
  var delegate: BLEPeripheralManagerDelegate?

  override init() {
    super.init()
    peripheralManager = CBPeripheralManager(delegate: self, queue: nil)
  }
  
  func peripheralManagerDidUpdateState(_ peripheral: CBPeripheralManager) {
    switch peripheral.state {
    case .poweredOn:
      print("Peripheral manager is up and running")
      
      
      self.heartRateService = CBMutableService(type: heartRateServiceUUID, primary: true)
      let characteristicProperties: CBCharacteristicProperties = [.notify, .read, .write]
      let characteristicPermissions: CBAttributePermissions = [.readable]
      self.heartRateCharacteristic = CBMutableCharacteristic(type: heartRateServiceUUID,
                                                            properties: characteristicProperties,
                                                            value: nil,
                                                            permissions: characteristicPermissions)
      
      heartRateService.characteristics = [heartRateCharacteristic]
      self.peripheralManager.add(heartRateService)
      
    default:
      print("Peripheral manager is down")
    }
  }

  func peripheralManager(_ peripheral: CBPeripheralManager, didAdd service: CBService, error: Error?) {
    if let uwError = error {
      print("Failed to add service with error: \(uwError.localizedDescription)")
      return
    }
    
    let advertisementData = [CBAdvertisementDataLocalNameKey: "Virtual HRM",
                             CBAdvertisementDataServiceUUIDsKey: [heartRateServiceUUID]] as [String : Any]
    peripheralManager.startAdvertising(advertisementData)
    print("Successfully added service")
  }
  
  
  func peripheralManagerDidStartAdvertising(_ peripheral: CBPeripheralManager, error: Error?) {
    if let uwError = error {
      print("Failed to advertise with error: \(uwError.localizedDescription)")
      return
    }
    
    print("Started advertising")
    
  }
  
  func peripheralManager(_ peripheral: CBPeripheralManager, didReceiveRead request: CBATTRequest) {
    if request.characteristic == self.heartRateCharacteristic {
      request.value = self.calculateHeartRate()
      self.peripheralManager.respond(to: request, withResult: .success)
      print("Responded successfully to a read request")
    }
  }
  
  func peripheralManager(_ peripheral: CBPeripheralManager, central: CBCentral, didSubscribeTo characteristic: CBCharacteristic) {
    print("Successfully subscribed")
    self.startSendingDataToSubscribers()
  }
  
  func peripheralManager(_ peripheral: CBPeripheralManager, central: CBCentral, didUnsubscribeFrom characteristic: CBCharacteristic) {
    self.notificationTimer.invalidate()
    print("Successfully unsubscribed")
  }

  func startSendingDataToSubscribers() {
    self.notificationTimer = Timer.scheduledTimer(timeInterval: 2, target: self, selector: #selector(self.updateSubscribers), userInfo: nil, repeats: true)
  }

  func peripheralManagerIsReady(toUpdateSubscribers peripheral: CBPeripheralManager) {
    print("Peripheral manager is ready to update subscribers")
    self.startSendingDataToSubscribers()
  }
  /// The value of a characteristic is returned as an array of 8-bit integers. If the value is small enough to fit in
  /// one byte (between 0 and 255), it is returned in two bytes where the first bit of the first byte is 0 and the second byte contains the
  /// actual value. In this case, the formula is:
  /// value = secondByte
  /// If the value is larger than 255, the first bit of the first byte is 1. In this case, the formula to get the value is:
  /// value = (firstByte * 256^0) + (secondByte * 256^1) + (thirdByte * 256^2)... and so on.
  func calculateHeartRate() -> Data {
    let randomHeartRate = UInt8.random(in: 22...222)
    self.delegate?.BLEPeripheralManagerDidSendValue(randomHeartRate)
    var heartRateBPM: [UInt8] = [0, randomHeartRate, 0, 0, 0, 0, 0, 0]
    let heartRateData = Data(bytes: &heartRateBPM, count: MemoryLayout.size(ofValue: heartRateBPM))
    return heartRateData
  }
  
  @objc func updateSubscribers() {
    let heartRateData = self.calculateHeartRate()
    self.peripheralManager.updateValue(heartRateData, for: self.heartRateCharacteristic, onSubscribedCentrals: nil)
  }
  
} /// class-end
