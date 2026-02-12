//
//  BLEPeripheralManager.swift
//  BLEVirtualHRM
//
//  Created by Manish Kumar on 2019-03-05.
//  Copyright © 2019 Manish Kumar. All rights reserved.
//

import CoreBluetooth

/*protocol BLEPeripheralManagerDelegate {
    func BLEPeripheralManagerDidSendValue(_ heartRateBPM: UInt8)
    func BLEPeripheralManagerCSCDidSendValue(_ flags: UInt8, crankRevolutions: UInt16, lastCrankEventTime: UInt16)
}*/
/// CBUUID will automatically convert this 16-bit number to 128-bit UUID
let heartRateServiceUUID = CBUUID(string: "0x180D")
let heartRateCharacteristicUUID = CBUUID(string: "0x2A37")

let CSCServiceUUID = CBUUID(string: "0x1816")
let CSCFeatureCharacteristicUUID = CBUUID(string: "0x2A5C")
let SensorLocationCharacteristicUUID = CBUUID(string: "0x2A5D")
let CSCMeasurementCharacteristicUUID = CBUUID(string: "0x2A5B")
let SCControlPointCharacteristicUUID = CBUUID(string: "0x2A55")

let PowerServiceUUID = CBUUID(string: "0x1818")
let PowerFeatureCharacteristicUUID = CBUUID(string: "0x2A65")
let PowerSensorLocationCharacteristicUUID = CBUUID(string: "0x2A5D")
let PowerMeasurementCharacteristicUUID = CBUUID(string: "0x2A63")

//WATTBIKE
let WattBikeServiceUUID = CBUUID(string: "b4cc1223-bc02-4cae-adb9-1217ad2860d1")
let WattBikeReadUUID = CBUUID(string: "b4cc1224-bc02-4cae-adb9-1217ad2860d1")
let WattBikeWriteUUID = CBUUID(string: "b4cc1225-bc02-4cae-adb9-1217ad2860d1")

// Zwift Play
let ZwiftPlayServiceUUID = CBUUID(string: "00000001-19ca-4651-86e5-fa29dcdd09d1")
let ZwiftPlayWriteUUID = CBUUID(string:    "00000003-19ca-4651-86e5-fa29dcdd09d1")
let ZwiftPlayReadUUID = CBUUID(string:   "00000002-19ca-4651-86e5-fa29dcdd09d1")
let ZwiftPlayIndicateUUID = CBUUID(string:"00000004-19ca-4651-86e5-fa29dcdd09d1")

@objc public class virtualbike_ios_swift: NSObject {
    private var peripheralManager: BLEPeripheralManager!
    
    @objc public override init() {
      super.init()
      peripheralManager = BLEPeripheralManager()
    }
    
    @objc public func updateHeartRate(HeartRate: UInt8)
    {
        peripheralManager.heartRate = HeartRate
    }
    
    @objc public func updateCadence(CrankRevolutions: UInt16, LastCrankEventTime: UInt16)
    {
        peripheralManager.lastCrankEventTime = LastCrankEventTime
        peripheralManager.crankRevolutions = CrankRevolutions
    }
}

class BLEPeripheralManager: NSObject, CBPeripheralManagerDelegate {
  private var peripheralManager: CBPeripheralManager!

  private var heartRateService: CBMutableService!
  private var heartRateCharacteristic: CBMutableCharacteristic!
    public var heartRate:UInt8! = 0

  private var CSCService: CBMutableService!
  private var CSCFeatureCharacteristic: CBMutableCharacteristic!
  private var SensorLocationCharacteristic: CBMutableCharacteristic!
  private var CSCMeasurementCharacteristic: CBMutableCharacteristic!
  private var SCControlPointCharacteristic: CBMutableCharacteristic!
    public var crankRevolutions: UInt16! = 0
    public var lastCrankEventTime: UInt16! = 0
    
    public var serviceToggle: Bool = false

  private var notificationTimer: Timer! = nil
  //var delegate: BLEPeripheralManagerDelegate?

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
																				 value: Data (bytes: [0x02, 0x00]),
																				 permissions: CSCFeaturePermissions)

      let SensorLocationProperties: CBCharacteristicProperties = [.read]
		let SensorLocationPermissions: CBAttributePermissions = [.readable]
		self.SensorLocationCharacteristic = CBMutableCharacteristic(type: SensorLocationCharacteristicUUID,
		                                                 properties: SensorLocationProperties,
																		 value: Data (bytes: [0x13]),
																		 permissions: SensorLocationPermissions)

        let CSCMeasurementProperties: CBCharacteristicProperties = [.notify, .read]
        let CSCMeasurementPermissions: CBAttributePermissions = [.readable]
		self.CSCMeasurementCharacteristic = CBMutableCharacteristic(type: CSCMeasurementCharacteristicUUID,
		                                           properties: CSCMeasurementProperties,
																 value: nil,
																 permissions: CSCMeasurementPermissions)

      let SCControlPointProperties: CBCharacteristicProperties = [.indicate, .write]
        let SCControlPointPermissions: CBAttributePermissions = [.writeable]
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
    if request.characteristic == self.CSCMeasurementCharacteristic {
        request.value = self.calculateCadence()
        self.peripheralManager.respond(to: request, withResult: .success)
        print("Responded successfully to a read request")
    }
  }
  
  func peripheralManager(_ peripheral: CBPeripheralManager, central: CBCentral, didSubscribeTo characteristic: CBCharacteristic) {
    print("Successfully subscribed")
    updateSubscribers();
    self.startSendingDataToSubscribers()
  }
  
  func peripheralManager(_ peripheral: CBPeripheralManager, central: CBCentral, didUnsubscribeFrom characteristic: CBCharacteristic) {
    //self.notificationTimer.invalidate()
    print("Successfully unsubscribed")
  }

  func startSendingDataToSubscribers() {
    if self.notificationTimer == nil {
	    self.notificationTimer = Timer.scheduledTimer(timeInterval: 1, target: self, selector: #selector(self.updateSubscribers), userInfo: nil, repeats: true)
		}
  }

  func peripheralManagerIsReady(toUpdateSubscribers peripheral: CBPeripheralManager) {
    print("Peripheral manager is ready to update subscribers")
    updateSubscribers();
    self.startSendingDataToSubscribers()
  }

  func calculateHeartRate() -> Data {
    //self.delegate?.BLEPeripheralManagerDidSendValue(self.heartRate)
    var heartRateBPM: [UInt8] = [0, self.heartRate, 0, 0, 0, 0, 0, 0]
    let heartRateData = Data(bytes: &heartRateBPM, count: heartRateBPM.count)
    return heartRateData
  }
    
    func calculateCadence() -> Data {
        let flags:UInt8 = 0x02
      //self.delegate?.BLEPeripheralManagerCSCDidSendValue(flags, crankRevolutions: self.crankRevolutions, lastCrankEventTime: self.lastCrankEventTime)
        var cadence: [UInt8] = [flags, (UInt8)(crankRevolutions & 0xFF), (UInt8)((crankRevolutions >> 8) & 0xFF),  (UInt8)(lastCrankEventTime & 0xFF), (UInt8)((lastCrankEventTime >> 8) & 0xFF)]
      let cadenceData = Data(bytes: &cadence, count: cadence.count)
      return cadenceData
    }
  
  @objc func updateSubscribers() {
    let heartRateData = self.calculateHeartRate()
    let cadenceData = self.calculateCadence()
    
    if(self.serviceToggle == true)
    {
        let ok = self.peripheralManager.updateValue(heartRateData, for: self.heartRateCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = !self.serviceToggle
        }
    }
    else
    {
        let ok = self.peripheralManager.updateValue(cadenceData, for: self.CSCMeasurementCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = !self.serviceToggle
        }
    }
  }
  
} /// class-end
