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
let heartRateCharacteristicUUID = CBUUID(string: "0x2A37")

let CSCServiceUUID = CBUUID(string: "0x1816")
let CSCFeatureCharacteristicUUID = CBUUID(string: "0x2A5C")
let SensorLocationCharacteristicUUID = CBUUID(string: "0x2A5D")
let CSCMeasurementCharacteristicUUID = CBUUID(string: "0x2A5B")
let SCControlPointCharacteristicUUID = CBUUID(string: "0x2A55")

class BLEPeripheralManager: NSObject, CBPeripheralManagerDelegate {
  private var peripheralManager: CBPeripheralManager!

  private var heartRateService: CBMutableService!
  private var heartRateCharacteristic: CBMutableCharacteristic!
  private var heartRate!

  private var CSCService: CBMutableService!
  private var CSCFeatureCharacteristic: CBMutableCharacteristic!
  private var SensorLocationCharacteristic: CBMutableCharacteristic!
  private var CSCMeasurementCharacteristic: CBMutableCharacteristic!
  private var SCControlPointCharacteristic: CBMutableCharacteristic!
  private var crankRevolutions!
  private var lastCrankEventTime!

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
      self.heartRateCharacteristic = CBMutableCharacteristic(type:          heartRateCharacteristicUUID,
                                                            properties: characteristicProperties,
                                                            value: nil,
                                                            permissions: characteristicPermissions)
      
      heartRateService.characteristics = [heartRateCharacteristic]
      self.peripheralManager.add(heartRateService)

      self.CSCService = CBMutableService(type: CSCServiceUUID, primary: true)

      let CSCFeatureProperties: CBCharacteristicProperties = [.read]
		let CSCFeaturePermissions: CBAttributePermissions = [.readable]
		self.CSCFeatureCharacteristic = CBMutableCharacteristic(type: CSCFeatureCharacteristicUUID,
		                                                       properties: CSCFeatureProperties,
																				 value: [0x02, 0x00],
																				 permissions: CSCFeaturePermissions)

      let SensorLocationProperties: CBCharacteristicProperties = [.read]
		let SensorLocationPermissions: CBAttributePermissions = [.readable]
		self.SensorLocationCharacteristic = CBMutableCharacteristic(type: SensorLocationCharacteristicUUID,
		                                                 properties: SensorLocationProperties,
																		 value: [13],
																		 permissions: SensorLocationPermissions)

      let CSCMeasurementProperties: CBCharacteristicProperties = [.notify, .read]
		let CSCMeasurementPermissions: CBAttributePermissions = [.readable]
		self.CSCMeasurementCharacteristic = CBMutableCharacteristic(type: CSCMeasurementCharacteristicUUID,
		                                           properties: CSCMeasurementProperties,
																 value: nil,
																 permissions: CSCMeasurementPermissions)

      let SCControlPointProperties: CBCharacteristicProperties = [.indicate, .write]
		let SCControlPointPermissions: CBAttributePermissions = [.writable]
		self.SCControlPointCharacteristic = CBMutableCharacteristic(type: SCControlPointCharacteristicUUID,
		                                     properties: SCControlPointProperties,
														 value: nil,
														 permissions: SCControlPointPermissions)

      CSCService.characteristics = [CSCFeatureCharacteristic,
		                              SensorLocationCharacteristic,
												CSCMeasurementCharacteristic,
												SCControlPointCharacteristic]
		self.peripheralManager.add(CSCService)

    default:
      print("Peripheral manager is down")
    }
  }

  func peripheralManager(_ peripheral: CBPeripheralManager, didAdd service: CBService, error: Error?) {
    if let uwError = error {
      print("Failed to add service with error: \(uwError.localizedDescription)")
      return
    }
    
    let advertisementData = [CBAdvertisementDataLocalNameKey: "qDomyos-zwift",
	                          CBAdvertisementDataServiceUUIDsKey: [heartRateServiceUUID, CSCServiceUUID]] as [String : Any]
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

  func calculateHeartRate() -> Data {
    self.delegate?.BLEPeripheralManagerDidSendValue(heartRate)
	 var heartRateBPM: [UInt8] = [0, heartRate, 0, 0, 0, 0, 0, 0]
    let heartRateData = Data(bytes: &heartRateBPM, count: MemoryLayout.size(ofValue: heartRateBPM))
    return heartRateData
  }
  
  @objc func updateSubscribers() {
    let heartRateData = self.calculateHeartRate()
    self.peripheralManager.updateValue(heartRateData, for: self.heartRateCharacteristic, onSubscribedCentrals: nil)
  }
  
} /// class-end
