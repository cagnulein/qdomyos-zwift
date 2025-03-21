import CoreBluetooth

let FitnessMachineServiceUuid = CBUUID(string: "0x1826");
let FitnessMachineFeatureCharacteristicUuid = CBUUID(string: "0x2ACC");
let supported_resistance_level_rangeCharacteristicUuid = CBUUID(string: "0x2AD6");
let FitnessMachineControlPointUuid = CBUUID(string: "0x2AD9");
let indoorbikeUuid = CBUUID(string: "0x2AD2");
let FitnessMachinestatusUuid = CBUUID(string: "0x2ADA");
let TrainingStatusUuid = CBUUID(string: "0x2AD3");

@objc public class virtualbike_zwift: NSObject {
    private var peripheralManager: BLEPeripheralManagerZwift!
    
    @objc public init(disable_hr: Bool, garmin_bluetooth_compatibility: Bool, zwift_play_emulator: Bool, watt_bike_emulator: Bool) {
      super.init()
      peripheralManager = BLEPeripheralManagerZwift(disable_hr: disable_hr, garmin_bluetooth_compatibility: garmin_bluetooth_compatibility, zwift_play_emulator: zwift_play_emulator, watt_bike_emulator: watt_bike_emulator)
    }
    
    @objc public func updateHeartRate(HeartRate: UInt8)
    {
        peripheralManager.heartRate = HeartRate
    }
    
    @objc public func readCurrentSlope() -> Double
    {
        return peripheralManager.CurrentSlope;
    }
    
    @objc public func readCurrentCRR() -> Double
    {
        return peripheralManager.CurrentCRR;
    }
    
    @objc public func readCurrentCW() -> Double
    {
        return peripheralManager.CurrentCW;
    }

    @objc public func readPowerRequested() -> Double
    {
        return peripheralManager.PowerRequested;
    }
    
    @objc public func updateFTMS(normalizeSpeed: UInt16, currentCadence: UInt16, currentResistance: UInt8, currentWatt: UInt16, CrankRevolutions: UInt16, LastCrankEventTime: UInt16, Gears: Int16) -> Bool
    {
        peripheralManager.NormalizeSpeed = normalizeSpeed
        peripheralManager.CurrentCadence = currentCadence
        peripheralManager.CurrentResistance = currentResistance
        peripheralManager.CurrentWatt = currentWatt
        peripheralManager.lastCrankEventTime = LastCrankEventTime
        peripheralManager.crankRevolutions = CrankRevolutions
        peripheralManager.CurrentGears = Gears

        return peripheralManager.connected;
    }
    
    @objc public func getLastFTMSMessage() -> Data? {
        peripheralManager.LastFTMSMessageReceivedAndPassed = peripheralManager.LastFTMSMessageReceived
        peripheralManager.LastFTMSMessageReceived?.removeAll()
        return peripheralManager.LastFTMSMessageReceivedAndPassed
    }
}

class BLEPeripheralManagerZwift: NSObject, CBPeripheralManagerDelegate {
  private var garmin_bluetooth_compatibility: Bool = false
  private var zwift_play_emulator: Bool = false
  private var watt_bike_emulator: Bool = false
    private var disable_hr: Bool = false
  private var peripheralManager: CBPeripheralManager!

  private var heartRateService: CBMutableService!
  private var heartRateCharacteristic: CBMutableCharacteristic!
  public var heartRate:UInt8! = 0

  private var FitnessMachineService: CBMutableService!
  private var FitnessMachineFeatureCharacteristic: CBMutableCharacteristic!
  private var supported_resistance_level_rangeCharacteristic: CBMutableCharacteristic!
  private var FitnessMachineControlPointCharacteristic: CBMutableCharacteristic!
  private var indoorbikeCharacteristic: CBMutableCharacteristic!
  private var FitnessMachinestatusCharacteristic: CBMutableCharacteristic!
  private var TrainingStatusCharacteristic: CBMutableCharacteristic!
    public var CurrentSlope: Double! = 0
	 public var CurrentCRR: Double! = 0
	 public var CurrentCW: Double! = 0
    public var PowerRequested: Double! = 0
    public var NormalizeSpeed: UInt16! = 0
    public var CurrentCadence: UInt16! = 0
    public var CurrentResistance: UInt8! = 0
    public var CurrentWatt: UInt16! = 0
    public var CurrentZwiftGear: UInt8! = 8
    
  private var CSCService: CBMutableService!
  private var CSCFeatureCharacteristic: CBMutableCharacteristic!
  private var SensorLocationCharacteristic: CBMutableCharacteristic!
  private var CSCMeasurementCharacteristic: CBMutableCharacteristic!
  private var SCControlPointCharacteristic: CBMutableCharacteristic!
    public var crankRevolutions: UInt16! = 0
    public var lastCrankEventTime: UInt16! = 0

  private var PowerService: CBMutableService!
    private var PowerFeatureCharacteristic: CBMutableCharacteristic!
    private var PowerSensorLocationCharacteristic: CBMutableCharacteristic!
    private var PowerMeasurementCharacteristic: CBMutableCharacteristic!

  private var WattBikeService: CBMutableService!
    private var WattBikeReadCharacteristic: CBMutableCharacteristic!
    private var WattBikeWriteCharacteristic: CBMutableCharacteristic!

  private var ZwiftPlayService: CBMutableService!
    private var ZwiftPlayReadCharacteristic: CBMutableCharacteristic!
    private var ZwiftPlayWriteCharacteristic: CBMutableCharacteristic!
    private var ZwiftPlayIndicateCharacteristic: CBMutableCharacteristic!

    
    public var LastFTMSMessageReceived: Data?
    public var LastFTMSMessageReceivedAndPassed: Data?
    
    public var serviceToggle: UInt8 = 0
    
    public var CurrentGears: Int16 = 0

  public var connected: Bool = false

  private var notificationTimer: Timer! = nil
    
  var updateQueue: [(characteristic: CBMutableCharacteristic, data: Data)] = []
    
  let SwiftDebug = swiftDebug()
  //var delegate: BLEPeripheralManagerDelegate?

  init(disable_hr: Bool, garmin_bluetooth_compatibility: Bool, zwift_play_emulator: Bool, watt_bike_emulator: Bool) {
    super.init()
    self.disable_hr = disable_hr
    self.garmin_bluetooth_compatibility = garmin_bluetooth_compatibility
    self.zwift_play_emulator = zwift_play_emulator
    self.watt_bike_emulator = watt_bike_emulator

    peripheralManager = CBPeripheralManager(delegate: self, queue: nil)
  }
  
  func peripheralManagerDidUpdateState(_ peripheral: CBPeripheralManager) {
    switch peripheral.state {
    case .poweredOn:
      print("Peripheral manager is up and running")
      
        if(!self.garmin_bluetooth_compatibility) {
            self.heartRateService = CBMutableService(type: heartRateServiceUUID, primary: true)
            let characteristicProperties: CBCharacteristicProperties = [.notify, .read, .write]
            let characteristicPermissions: CBAttributePermissions = [.readable]
            self.heartRateCharacteristic = CBMutableCharacteristic(type:          heartRateCharacteristicUUID,
                                                                   properties: characteristicProperties,
                                                                   value: nil,
                                                                   permissions: characteristicPermissions)
            
            heartRateService.characteristics = [heartRateCharacteristic]
            self.peripheralManager.add(heartRateService)
            
            self.FitnessMachineService = CBMutableService(type: FitnessMachineServiceUuid, primary: true)
            
            let FitnessMachineFeatureProperties: CBCharacteristicProperties = [.read]
            let FitnessMachineFeaturePermissions: CBAttributePermissions = [.readable]
            self.FitnessMachineFeatureCharacteristic = CBMutableCharacteristic(type: FitnessMachineFeatureCharacteristicUuid,
                                                                               properties: FitnessMachineFeatureProperties,
                                                                               value: Data (bytes: [0x83, 0x14, 0x00, 0x00, 0x0c, 0xe0, 0x00, 0x00]),
                                                                               permissions: FitnessMachineFeaturePermissions)
            
            let supported_resistance_level_rangeProperties: CBCharacteristicProperties = [.read]
            let supported_resistance_level_rangePermissions: CBAttributePermissions = [.readable]
            self.supported_resistance_level_rangeCharacteristic = CBMutableCharacteristic(type: supported_resistance_level_rangeCharacteristicUuid,
                                                                                          properties: supported_resistance_level_rangeProperties,
                                                                                          value: Data (bytes: [0x0A, 0x00, 0x96, 0x00, 0x0A, 0x00]),
                                                                                          permissions: supported_resistance_level_rangePermissions)
            
            let FitnessMachineControlPointProperties: CBCharacteristicProperties = [.indicate, .notify, .write]
            let FitnessMachineControlPointPermissions: CBAttributePermissions = [.writeable]
            self.FitnessMachineControlPointCharacteristic = CBMutableCharacteristic(type: FitnessMachineControlPointUuid,
                                                                                    properties: FitnessMachineControlPointProperties,
                                                                                    value: nil,
                                                                                    permissions: FitnessMachineControlPointPermissions)
            
            let indoorbikeProperties: CBCharacteristicProperties = [.notify, .read]
            let indoorbikePermissions: CBAttributePermissions = [.readable]
            self.indoorbikeCharacteristic = CBMutableCharacteristic(type: indoorbikeUuid,
                                                                    properties: indoorbikeProperties,
                                                                    value: nil,
                                                                    permissions: indoorbikePermissions)
            
            let FitnessMachinestatusProperties: CBCharacteristicProperties = [.notify]
            let FitnessMachinestatusPermissions: CBAttributePermissions = [.readable]
            self.FitnessMachinestatusCharacteristic = CBMutableCharacteristic(type: FitnessMachinestatusUuid,
                                                                              properties: FitnessMachinestatusProperties,
                                                                              value: nil,
                                                                              permissions: FitnessMachinestatusPermissions)
            
            let TrainingStatusProperties: CBCharacteristicProperties = [.read]
            let TrainingStatusPermissions: CBAttributePermissions = [.readable]
            self.TrainingStatusCharacteristic = CBMutableCharacteristic(type: TrainingStatusUuid,
                                                                        properties: TrainingStatusProperties,
                                                                        value: Data (bytes: [0x00, 0x01]),
                                                                        permissions: TrainingStatusPermissions)
            
            FitnessMachineService.characteristics = [FitnessMachineFeatureCharacteristic,
                                                     supported_resistance_level_rangeCharacteristic,
                                                     FitnessMachineControlPointCharacteristic,
                                                     indoorbikeCharacteristic,
                                                     FitnessMachinestatusCharacteristic,
                                                     TrainingStatusCharacteristic ]
            
            self.peripheralManager.add(FitnessMachineService)
            
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
                                                                        value: Data (bytes: [0x0D]),
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
        }
        self.PowerService = CBMutableService(type: PowerServiceUUID, primary: true)
        
        let PowerFeatureProperties: CBCharacteristicProperties = [.read]
          let PowerFeaturePermissions: CBAttributePermissions = [.readable]
          self.PowerFeatureCharacteristic = CBMutableCharacteristic(type: PowerFeatureCharacteristicUUID,
                                                                 properties: PowerFeatureProperties,
                                                                                   value: Data (bytes: [0x00, 0x00, 0x00, 0x08]),
                                                                                   permissions: PowerFeaturePermissions)

        let PowerSensorLocationProperties: CBCharacteristicProperties = [.read]
          let PowerSensorLocationPermissions: CBAttributePermissions = [.readable]
          self.PowerSensorLocationCharacteristic = CBMutableCharacteristic(type: PowerSensorLocationCharacteristicUUID,
                                                           properties: PowerSensorLocationProperties,
                                                                           value: Data (bytes: [0x0D]),
                                                                           permissions: PowerSensorLocationPermissions)

          let PowerMeasurementProperties: CBCharacteristicProperties = [.notify, .read]
          let PowerMeasurementPermissions: CBAttributePermissions = [.readable]
          self.PowerMeasurementCharacteristic = CBMutableCharacteristic(type: PowerMeasurementCharacteristicUUID,
                                                     properties: PowerMeasurementProperties,
                                                                   value: nil,
                                                                   permissions: PowerMeasurementPermissions)


        PowerService.characteristics = [PowerFeatureCharacteristic,
                                        PowerSensorLocationCharacteristic,
                                        PowerMeasurementCharacteristic]
          self.peripheralManager.add(PowerService)

       
        // ZwiftPlay
        if(self.zwift_play_emulator) {
          self.ZwiftPlayService = CBMutableService(type: ZwiftPlayServiceUUID, primary: true)
          
          let ZwiftPlayReadProperties: CBCharacteristicProperties = [.notify, .read]
            let ZwiftPlayReadPermissions: CBAttributePermissions = [.readable]
            self.ZwiftPlayReadCharacteristic = CBMutableCharacteristic(type: ZwiftPlayReadUUID,
                                                            properties: ZwiftPlayReadProperties,
                                                                      value: nil,
                                                                            permissions: ZwiftPlayReadPermissions)

          let ZwiftPlayWriteProperties: CBCharacteristicProperties = [.write]
            let ZwiftPlayWritePermissions: CBAttributePermissions = [.writeable]
            self.ZwiftPlayWriteCharacteristic = CBMutableCharacteristic(type: ZwiftPlayWriteUUID,
                                                      properties: ZwiftPlayWriteProperties,
                                                                    value: nil,
                                                                    permissions: ZwiftPlayWritePermissions)

          let ZwiftPlayIndicateProperties: CBCharacteristicProperties = [.indicate]
          let ZwiftPlayIndicatePermissions: CBAttributePermissions = [.readable]
            self.ZwiftPlayIndicateCharacteristic = CBMutableCharacteristic(type: ZwiftPlayIndicateUUID,
                                                      properties: ZwiftPlayIndicateProperties,
                                                                    value: nil,
                                                                    permissions: ZwiftPlayIndicatePermissions)

          ZwiftPlayService.characteristics = [ZwiftPlayReadCharacteristic,
                                            ZwiftPlayWriteCharacteristic,
                                              ZwiftPlayIndicateCharacteristic]
            self.peripheralManager.add(ZwiftPlayService)
        } else if(watt_bike_emulator) {
          // WATT BIKE
          self.WattBikeService = CBMutableService(type: WattBikeServiceUUID, primary: true)
          
          let WattBikeReadProperties: CBCharacteristicProperties = [.notify, .read]
            let WattBikeReadPermissions: CBAttributePermissions = [.readable]
            self.WattBikeReadCharacteristic = CBMutableCharacteristic(type: WattBikeReadUUID,
                                                            properties: WattBikeReadProperties,
                                                                      value: nil,
                                                                            permissions: WattBikeReadPermissions)

          let WattBikeWriteProperties: CBCharacteristicProperties = [.write]
            let WattBikeWritePermissions: CBAttributePermissions = [.writeable]
            self.WattBikeWriteCharacteristic = CBMutableCharacteristic(type: WattBikeWriteUUID,
                                                      properties: WattBikeWriteProperties,
                                                                    value: nil,
                                                                    permissions: WattBikeWritePermissions)


          WattBikeService.characteristics = [WattBikeReadCharacteristic,
                                            WattBikeWriteCharacteristic]
            self.peripheralManager.add(WattBikeService)          
        }
        
    default:
      print("Peripheral manager is down")
    }
  }
    
    func sendUpdates() -> Bool {
        guard !updateQueue.isEmpty else { return false }
        
        let update = updateQueue.removeFirst()
        let hexString = update.data.map { String(format: "%02x", $0) }.joined(separator: " ")
        let debugMessage = "virtualbike_zwift send: " + String(describing: update.characteristic) + " " + hexString
        SwiftDebug.qtDebug(debugMessage)
        peripheralManager.updateValue(update.data, for: update.characteristic, onSubscribedCentrals: nil)
        return true
    }
    
  func peripheralManager(_ peripheral: CBPeripheralManager, didAdd service: CBService, error: Error?) {
    if let uwError = error {
      print("Failed to add service with error: \(uwError.localizedDescription)")
      return
    }
            
      
      if(garmin_bluetooth_compatibility) {
          let advertisementData = [CBAdvertisementDataLocalNameKey: "QZ",
                                CBAdvertisementDataServiceUUIDsKey: [PowerServiceUUID]] as [String : Any]
          peripheralManager.startAdvertising(advertisementData)
      } else if(disable_hr) {
          // useful in order to hide HR from Garmin devices
          let advertisementData = [CBAdvertisementDataLocalNameKey: "QZ",
                                    CBAdvertisementDataServiceUUIDsKey: [FitnessMachineServiceUuid, CSCServiceUUID, PowerServiceUUID]] as [String : Any]
          peripheralManager.startAdvertising(advertisementData)
      } else {
          let advertisementData = [CBAdvertisementDataLocalNameKey: "QZ",
                                  CBAdvertisementDataServiceUUIDsKey: [heartRateServiceUUID, FitnessMachineServiceUuid, CSCServiceUUID, PowerServiceUUID]] as [String : Any]
          peripheralManager.startAdvertising(advertisementData)
      }
    
    print("Successfully added service")
  }
  
  
  func peripheralManagerDidStartAdvertising(_ peripheral: CBPeripheralManager, error: Error?) {
    if let uwError = error {
      print("Failed to advertise with error: \(uwError.localizedDescription)")
      return
    }
    
    print("Started advertising")
    
  }
  
    func calculateUnknown1(power: UInt16) -> UInt32 {
       var lastUnknown1: UInt32 = 836
       let baseValue: UInt32 = 19000
       
       var increment = 400 + (UInt32(power) * 2)
       increment = min(increment, 800)
       
       if power > 0 {
           lastUnknown1 += increment
       } else {
           lastUnknown1 += 600
       }
       
       if lastUnknown1 > 24000 {
           lastUnknown1 = baseValue
       }
       
       return lastUnknown1
    }
    
    func peripheralManager(_ peripheral: CBPeripheralManager, didReceiveWrite requests: [CBATTRequest]) {
    if let value = requests.first?.value {
          let hexString = value.map { String(format: "%02x", $0) }.joined(separator: " ")
        let debugMessage = "virtualbike_zwift didReceiveWrite: " + String(describing: requests.first!.characteristic) + " " + hexString + " " + ((requests.first!.characteristic == self.FitnessMachineControlPointCharacteristic) ? "FTMS" : "NOFTMS") + " " + String(describing: self.FitnessMachineControlPointCharacteristic) + " " + self.FitnessMachineControlPointCharacteristic.uuid.uuidString + " " + requests.first!.characteristic.uuid.uuidString
          SwiftDebug.qtDebug(debugMessage)
    }      

    if requests.first!.characteristic == self.FitnessMachineControlPointCharacteristic {        
        if(LastFTMSMessageReceived == nil || LastFTMSMessageReceived?.count == 0) {
            LastFTMSMessageReceived = requests.first!.value
        }
        if(requests.first!.value?.first == 0x11)
        {
               var high : Int16 = ((Int16)(requests.first!.value![4])) << 8;
                 self.CurrentSlope = (Double)((Int16)(requests.first!.value![3]) + high);
					  self.CurrentCRR = (Double)((Int16)(requests.first!.value![5]));
					  self.CurrentCW = (Double)((Int16)(requests.first!.value![6]));
        }
        else if(requests.first!.value?.first == 0x05)
        {
            var high : UInt16 = (((UInt16)(requests.first!.value![2])) << 8);
            self.PowerRequested = (Double)((UInt16)(requests.first!.value![1]) + high);
        }
        self.connected = true;
        self.peripheralManager.respond(to: requests.first!, withResult: .success)
        SwiftDebug.qtDebug("Responded successfully to a write request")

        let funcCode: UInt8 = requests.first!.value![0]
        var response: [UInt8] = [0x80, funcCode , 0x01]
        let responseData = Data(bytes: &response, count: 3)
          
        self.peripheralManager.updateValue(responseData, for: self.FitnessMachineControlPointCharacteristic, onSubscribedCentrals: nil)
    } else if requests.first!.characteristic == ZwiftPlayWriteCharacteristic && zwift_play_emulator {
        let receivedData = requests.first!.value ?? Data()
      let expectedHexArray: [UInt8] = [0x52, 0x69, 0x64, 0x65, 0x4F, 0x6E, 0x02]
      let expectedHexArray2: [UInt8] = [0x41, 0x08, 0x05]
      let expectedHexArray3: [UInt8] = [0x00, 0x08, 0x88, 0x04]
      let expectedHexArray4: [UInt8] = [0x04, 0x2a, 0x0a, 0x10, 0xc0, 0xbb, 0x01, 0x20]
      let expectedHexArray5: [UInt8] = [0x04, 0x22]
      let expectedHexArray6: [UInt8] = [0x04, 0x2a, 0x04, 0x10]
      let expectedHexArray7: [UInt8] = [0x04, 0x2a, 0x03, 0x10]
      let expectedHexArray8: [UInt8] = [0x04, 0x18]

      let receivedBytes = [UInt8](receivedData.prefix(expectedHexArray.count))
      
      if receivedBytes == expectedHexArray {
        SwiftDebug.qtDebug("Zwift Play Ask 1")
        peripheral.respond(to: requests.first!, withResult: .success)
        
        var response: [UInt8] = [0x2a, 0x08, 0x03, 0x12, 0x11, 0x22, 0x0f, 0x41, 0x54, 0x58, 0x20, 0x30, 0x34, 0x2c, 0x20, 0x53, 0x54, 0x58, 0x20, 0x30, 0x34, 0x00]
        var responseData = Data(bytes: &response, count: 22)

        updateQueue.append((ZwiftPlayReadCharacteristic, responseData))

        response = [0x2a, 0x08, 0x03, 0x12, 0x0d, 0x22, 0x0b, 0x52, 0x49, 0x44, 0x45, 0x5f, 0x4f, 0x4e, 0x28, 0x32, 0x29, 0x00]
        responseData = Data(bytes: &response, count: 18)

        updateQueue.append((ZwiftPlayReadCharacteristic, responseData))

        response = [0x52, 0x69, 0x64, 0x65, 0x4f, 0x6e, 0x02, 0x00]
        responseData = Data(bytes: &response, count: 8)

        updateQueue.append((ZwiftPlayIndicateCharacteristic, responseData))
      }
      let receivedBytes2 = [UInt8](receivedData.prefix(expectedHexArray2.count))
      
      if receivedBytes2 == expectedHexArray2 {
        SwiftDebug.qtDebug("Zwift Play Ask 2")
        peripheral.respond(to: requests.first!, withResult: .success)
        
        
        var response: [UInt8] = [0x3c, 0x08, 0x00, 0x12, 0x32, 0x0a, 0x30, 0x08, 0x80, 0x04, 0x12, 0x04, 0x05, 0x00, 0x05, 0x01, 0x1a, 0x0b, 0x4b, 0x49, 0x43, 0x4b, 0x52, 0x20, 0x43, 0x4f, 0x52, 0x45, 0x00, 0x32, 0x0f, 0x34, 0x30, 0x32, 0x34, 0x31, 0x38, 0x30, 0x30, 0x39, 0x38, 0x34, 0x00, 0x00, 0x00, 0x00, 0x3a, 0x01, 0x31, 0x42, 0x04, 0x08, 0x01, 0x10, 0x14 ]
        var responseData = Data(bytes: &response, count: 55)

        updateQueue.append((ZwiftPlayIndicateCharacteristic, responseData))
      }
      let receivedBytes3 = [UInt8](receivedData.prefix(expectedHexArray3.count))
      
      if receivedBytes3 == expectedHexArray3 {
        SwiftDebug.qtDebug("Zwift Play Ask 3")
        peripheral.respond(to: requests.first!, withResult: .success)
        
        
        var response: [UInt8] = [0x3c, 0x08, 0x88, 0x04, 0x12, 0x06, 0x0a, 0x04, 0x40, 0xc0, 0xbb, 0x01 ]
        var responseData = Data(bytes: &response, count: 12)

        updateQueue.append((ZwiftPlayIndicateCharacteristic, responseData))
      }
      let receivedBytes4 = [UInt8](receivedData.prefix(expectedHexArray4.count))
      
      if receivedBytes4 == expectedHexArray4 {
        SwiftDebug.qtDebug("Zwift Play Ask 4")
        peripheral.respond(to: requests.first!, withResult: .success)
        
        
        var response: [UInt8] = [ 0x03, 0x08, 0x00, 0x10, 0x00, 0x18, 0x59, 0x20, 0x00, 0x28, 0x00, 0x30, 0x9b, 0xed, 0x01]
        var responseData = Data(bytes: &response, count: 15)

          updateQueue.append((ZwiftPlayReadCharacteristic, responseData))
          
        response = [ 0x2a, 0x08, 0x03, 0x12, 0x27, 0x22, 0x25, 0x67, 0x61, 0x70, 0x5f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x73, 0x5f, 0x63, 0x68, 0x61, 0x6e, 0x67, 0x65, 0x28, 0x32, 0x29, 0x3a, 0x20, 0x37, 0x32, 0x2c, 0x20, 0x37, 0x32, 0x2c, 0x20, 0x30, 0x2c, 0x20, 0x36, 0x30, 0x30, 0x00 ]
        responseData = Data(bytes: &response, count: 44)

        updateQueue.append((ZwiftPlayReadCharacteristic, responseData))
      }
      let receivedBytes5 = [UInt8](receivedData.prefix(expectedHexArray5.count))
      
      if receivedBytes5 == expectedHexArray5 {
        SwiftDebug.qtDebug("Zwift Play Ask 5")
        peripheral.respond(to: requests.first!, withResult: .success)
        
        // 04 22 02 10 1a TODO
          var r = receivedBytes
          r.remove(at: 0)
          let slopefloat = decodeSInt(r)
          print("slopefloat \(slopefloat)")
          var slope: [UInt8] = [ 0x00, 0x00 ]
          self.CurrentSlope = Double(slopefloat)
          slope[0] = UInt8(Int16(self.CurrentSlope) & 0xFF)
          slope[1] = UInt8((Int16(self.CurrentSlope) >> 8) & 0x00FF)
          LastFTMSMessageReceived = Data([0x11, 0x69, 0x01, slope[0], slope[1], 0x32, 0x28])
          
          do {
              let response = try ZwiftHubBike.ridingData(power: UInt32(self.CurrentWatt), cadence: UInt32(self.CurrentCadence / 2), speed: 0, HR: UInt32(self.heartRate), unkown1: self.calculateUnknown1(power: self.CurrentWatt), unkown2: 0)
              
              updateQueue.append((ZwiftPlayIndicateCharacteristic, response))
          } catch {
              
          }
      }
      let receivedBytes6 = [UInt8](receivedData.prefix(expectedHexArray6.count))
      
      if receivedBytes6 == expectedHexArray6 {
        SwiftDebug.qtDebug("Zwift Play Ask 6")
        peripheral.respond(to: requests.first!, withResult: .success)
        
        
        var response: [UInt8] = [ 0x3c, 0x08, 0x88, 0x04, 0x12, 0x06, 0x0a, 0x04, 0x40, 0xc0, 0xbb, 0x01 ]
        response[9] = receivedData[4]
        response[10] = receivedData[5]
        response[11] = receivedData[6]
        handleZwiftGear(receivedData[4...])
        var responseData = Data(bytes: &response, count: 12)

          updateQueue.append((ZwiftPlayIndicateCharacteristic, responseData))

        response = [0x03, 0x08, 0x00, 0x10, 0x00, 0x18, 0xe7, 0x02, 0x20, 0x00, 0x28, 0x96, 0x14, 0x30, 0x9b, 0xed, 0x01]
        responseData = Data(bytes: &response, count: 17)
        updateQueue.append((ZwiftPlayReadCharacteristic, responseData))
      }
    let receivedBytes7 = [UInt8](receivedData.prefix(expectedHexArray7.count))

    if receivedBytes7 == expectedHexArray7 {
      SwiftDebug.qtDebug("Zwift Play Ask 7")
      peripheral.respond(to: requests.first!, withResult: .success)
      
      var response: [UInt8]  = [0x03, 0x08, 0x00, 0x10, 0x00, 0x18, 0xe7, 0x02, 0x20, 0x00, 0x28, 0x00, 0x30, 0x9b, 0xed, 0x01]
      var responseData = Data(bytes: &response, count: 17)
      updateQueue.append((ZwiftPlayReadCharacteristic, responseData))
        
        response = [ 0x3c, 0x08, 0x88, 0x04, 0x12, 0x05, 0x0a, 0x03, 0x40, 0x8c, 0x60 ]
        response[9] = receivedData[4]
        response[10] = receivedData[5]
        responseData = Data(bytes: &response, count: 11)
        handleZwiftGear(receivedData[4...])

          updateQueue.append((ZwiftPlayIndicateCharacteristic, responseData))

    }

      let receivedBytes8 = [UInt8](receivedData.prefix(expectedHexArray8.count))
      
      if receivedBytes8 == expectedHexArray8 {
        SwiftDebug.qtDebug("Zwift Play Ask 8")
        peripheral.respond(to: requests.first!, withResult: .success)
        
        var protobuf: [UInt8] = [receivedData[2], 0x00]
          if(receivedData.count == 4) {
              protobuf[1] = receivedData[3]
          }
                
        let (power, _)  = decodeVarint(protobuf, startIndex: 0)
        self.PowerRequested = (Double)(power);
        LastFTMSMessageReceived = Data([0x05, UInt8(UInt16(power) & 0xff), UInt8(((UInt16(power) & 0xff00) >> 8) & 0x00ff)])
          
          do {
              let response = try ZwiftHubBike.ridingData(power: UInt32(power), cadence: UInt32(self.CurrentCadence / 2), speed: Double(self.NormalizeSpeed) / 100.0, HR: UInt32(self.heartRate), unkown1: self.calculateUnknown1(power: self.CurrentWatt), unkown2: 0)
              
              updateQueue.append((ZwiftPlayIndicateCharacteristic, response))
          } catch {
              
          }
      }

    }
    }
    
    func handleZwiftGear(_ array: Data) {
        var g : UInt8 = 0;
        if(array[4] == 0xCC && array[5] == 0x3a) {
            g = 1;
        } else if(array[4] == 0xFC && array[5] == 0x43) {
            g = 2;
        } else if(array[4] == 0xac && array[5] == 0x4d) {
            g = 3;
        } else if(array[4] == 0xdc && array[5] == 0x56) {
            g = 4;
        } else if(array[4] == 0x8c && array[5] == 0x60) {
            g = 5;
        } else if(array[4] == 0xe8 && array[5] == 0x6b) {
            g = 6;
        } else if(array[4] == 0xc4 && array[5] == 0x77) {
            g = 7;
        } else if(array[4] == 0xa0 && array[5] == 0x83 && array[6] == 0x01) {
            g = 8;
        } else if(array[4] == 0xa8 && array[5] == 0x91 && array[6] == 0x01) {
            g = 9;
        } else if(array[4] == 0xb0 && array[5] == 0x9f && array[6] == 0x01) {
            g = 10;
        } else if(array[4] == 0xb8 && array[5] == 0xad && array[6] == 0x01) {
            g = 11;
        } else if(array[4] == 0xc0 && array[5] == 0xbb && array[6] == 0x01) {
            g = 12;
        } else if(array[4] == 0xf3 && array[5] == 0xcb && array[6] == 0x01) {
            g = 13;
        } else if(array[4] == 0xa8 && array[5] == 0xdc && array[6] == 0x01) {
            g = 14;
        } else if(array[4] == 0xdc && array[5] == 0xec && array[6] == 0x01) {
            g = 15;
        } else if(array[4] == 0x90 && array[5] == 0xfd && array[6] == 0x01) {
            g = 16;
        } else if(array[4] == 0xd4 && array[5] == 0x90 && array[6] == 0x02) {
            g = 17;
        } else if(array[4] == 0x98 && array[5] == 0xa4 && array[6] == 0x02) {
            g = 18;
        } else if(array[4] == 0xdc && array[5] == 0xb7 && array[6] == 0x02) {
            g = 19;
        } else if(array[4] == 0x9f && array[5] == 0xcb && array[6] == 0x02) {
            g = 20;
        } else if(array[4] == 0xd8 && array[5] == 0xe2 && array[6] == 0x02) {
            g = 21;
        } else if(array[4] == 0x90 && array[5] == 0xfa && array[6] == 0x02) {
            g = 22;
        } else if(array[4] == 0xc8 && array[5] == 0x91 && array[6] == 0x03) {
            g = 23;
        } else if(array[4] == 0xf3 && array[5] == 0xac && array[6] == 0x03) {
            g = 24;
        }

        
        if (g < self.CurrentZwiftGear) {
            for _ in 0..<(self.CurrentZwiftGear - g) {
                SwiftDebug.gearDown()
            }
        } else if (g > self.CurrentZwiftGear) {
            for _ in 0..<(g - self.CurrentZwiftGear) {
                SwiftDebug.gearUp()
            }
        }
        self.CurrentZwiftGear = g
    }
    
    func decodeVarint(_ bytes: [UInt8], startIndex: Int) -> (value: Int, bytesRead: Int) {
        var result = 0
        var shift = 0
        var bytesRead = 0
        
        for i in startIndex..<bytes.count {
            let byte = bytes[i]
            result |= Int(byte & 0x7F) << shift
            bytesRead += 1
            
            if (byte & 0x80) == 0 {
                break
            }
            shift += 7
        }
        
        return (result, bytesRead)
    }
    
    func decodeSInt(_ bytes: [UInt8]) -> Int {
        // Verifica header del campo (0x22)
        guard bytes[0] == 0x22 else {
            fatalError("Invalid field header")
        }
        
        // Lunghezza del contenuto
        let length = Int(bytes[1])
        
        // Verifica header interno (0x10)
        guard bytes[2] == 0x10 else {
            fatalError("Invalid inner header")
        }
        
        // Decodifica il varint
        let (value, _) = decodeVarint(bytes, startIndex: 3)
        
        // Applica la decodifica ZigZag
        let decoded = (value >> 1) ^ -(value & 1)
        
        return decoded
    }
    
  func peripheralManager(_ peripheral: CBPeripheralManager, didReceiveRead request: CBATTRequest) {
    SwiftDebug.qtDebug("virtualbike_zwift didReceiveRead: " + String(describing: request.characteristic))
    if request.characteristic == self.heartRateCharacteristic {
      request.value = self.calculateHeartRate()
      self.peripheralManager.respond(to: request, withResult: .success)
      print("Responded successfully to a read request")
    }
    else if request.characteristic == self.CSCMeasurementCharacteristic {
      request.value = self.calculateCadence()
      self.peripheralManager.respond(to: request, withResult: .success)
      print("Responded successfully to a read request")
    }
    else if request.characteristic == self.indoorbikeCharacteristic {
        request.value = self.calculateIndoorBike()
        self.peripheralManager.respond(to: request, withResult: .success)
        print("Responded successfully to a read request")
    } else if request.characteristic == self.PowerMeasurementCharacteristic {
        request.value = self.calculatePower()
        self.peripheralManager.respond(to: request, withResult: .success)
        print("Responded successfully to a read request")
    }
  }
  
  func peripheralManager(_ peripheral: CBPeripheralManager, central: CBCentral, didSubscribeTo characteristic: CBCharacteristic) {
    print("Successfully subscribed")
	 self.connected = true
    updateSubscribers();
    self.startSendingDataToSubscribers()
  }
  
  func peripheralManager(_ peripheral: CBPeripheralManager, central: CBCentral, didUnsubscribeFrom characteristic: CBCharacteristic) {
    //self.notificationTimer.invalidate()
	 self.connected = false
    print("Successfully unsubscribed")
  }

  func startSendingDataToSubscribers() {
    if self.notificationTimer == nil {
        var t : Double = 0.2
        if(zwift_play_emulator) {
            t = 0.01
        }
        self.notificationTimer = Timer.scheduledTimer(timeInterval: t, target: self, selector: #selector(self.updateSubscribers), userInfo: nil, repeats: true)
    }
  }

  func peripheralManagerIsReady(toUpdateSubscribers peripheral: CBPeripheralManager) {
    print("Peripheral manager is ready to update subscribers")
    updateSubscribers();
    self.startSendingDataToSubscribers()
  }

    func calculateCadence() -> Data {
        let flags:UInt8 = 0x02
      //self.delegate?.BLEPeripheralManagerCSCDidSendValue(flags, crankRevolutions: self.crankRevolutions, lastCrankEventTime: self.lastCrankEventTime)
        var cadence: [UInt8] = [flags, (UInt8)(crankRevolutions & 0xFF), (UInt8)((crankRevolutions >> 8) & 0xFF),  (UInt8)(lastCrankEventTime & 0xFF), (UInt8)((lastCrankEventTime >> 8) & 0xFF)]
      let cadenceData = Data(bytes: &cadence, count: MemoryLayout.size(ofValue: cadence))
      return cadenceData
    }
    
    
    private var revolutions: UInt16 = 0
    private var timestamp: UInt16 = 0
    private var lastRevolution: UInt64 = UInt64(Date().timeIntervalSince1970 * 1000)
    
    func calculatePower() -> Data {
        if(garmin_bluetooth_compatibility) {
            /*
             // convert RPM to timestamp
             if (cadenceInstantaneous != 0 && (millis()) >= (lastRevolution + (60000 / cadenceInstantaneous)))
             {
             revolutions++;                                  // One crank revolution should have passed, add one revolution
             timestamp = (unsigned short)(((millis() * 1024) / 1000) % 65536); // create timestamp and format
             lastRevolution = millis();
             }
             */
            
            let millis : UInt64 = UInt64(Date().timeIntervalSince1970 * 1000)
            if CurrentCadence != 0 && (millis >= lastRevolution + (60000 / UInt64(CurrentCadence / 2))) {
                revolutions = revolutions + 1
                var newT: UInt64 = ((60000 / (UInt64(CurrentCadence / 2)) * 1024) / 1000)
                newT = newT + UInt64(timestamp)
                newT = newT  % 65536
                timestamp = UInt16(newT)
                lastRevolution = millis
            }
            
            let flags:UInt8 = 0x20
            //self.delegate?.BLEPeripheralManagerCSCDidSendValue(flags, crankRevolutions: self.crankRevolutions, lastCrankEventTime: self.lastCrankEventTime)
            var power: [UInt8] = [flags, 0x00, (UInt8)(self.CurrentWatt & 0xFF), (UInt8)((self.CurrentWatt >> 8) & 0xFF), (UInt8)(revolutions & 0xFF), (UInt8)((revolutions >> 8) & 0xFF),  (UInt8)(timestamp & 0xFF), (UInt8)((timestamp >> 8) & 0xFF)]
            let powerData = Data(bytes: &power, count: MemoryLayout.size(ofValue: power))
            return powerData
        } else {
            let flags:UInt8 = 0x30

                    /*
                     // set measurement
                     measurement[2] = power & 0xFF;
                     measurement[3] = (power >> 8) & 0xFF;
                     measurement[4] = wheelrev & 0xFF;
                     measurement[5] = (wheelrev >> 8) & 0xFF;
                     measurement[6] = (wheelrev >> 16) & 0xFF;
                     measurement[7] = (wheelrev >> 24) & 0xFF;
                     measurement[8] = lastwheel & 0xFF;
                     measurement[9] = (lastwheel >> 8) & 0xFF;
                     measurement[10] = crankrev & 0xFF;
                     measurement[11] = (crankrev >> 8) & 0xFF;
                     measurement[12] = lastcrank & 0xFF;
                     measurement[13] = (lastcrank >> 8) & 0xFF;
                     
                     // speed & distance
                     // NOTE : based on Apple Watch default wheel dimension 700c x 2.5mm
                     // NOTE : 3 is theoretical crank:wheel gear ratio
                     // NOTE : 2.13 is circumference of 700c in meters

                     wheelCount = crankCount * 3;
                          speed = cadence * 3 * 2.13 * 60 / 1000;
                       distance = wheelCount * 2.13 / 1000;
                     #if defined(USEPOWER)
                       lastWheelK = lastCrankK * 2;  // 1/2048 s granularity
                     #else
                       lastWheelK = lastCrankK * 1;  // 1/1024 s granularity
                     #endif

                     */

                  //self.delegate?.BLEPeripheralManagerCSCDidSendValue(flags, crankRevolutions: self.crankRevolutions, lastCrankEventTime: self.lastCrankEventTime)
                    let wheelrev: UInt32 = ((UInt32)(crankRevolutions)) * 3;
                    let lastWheel: UInt16 = (UInt16)((((UInt32)(lastCrankEventTime)) * 2) & 0xFFFF);
                    var power: [UInt8] = [flags, 0x00, (UInt8)(self.CurrentWatt & 0xFF), (UInt8)((self.CurrentWatt >> 8) & 0xFF),
                                          (UInt8)(wheelrev & 0xFF), (UInt8)((wheelrev >> 8) & 0xFF), (UInt8)((wheelrev >> 16) & 0xFF), (UInt8)((wheelrev >> 24) & 0xFF),
                                          (UInt8)(lastWheel & 0xFF), (UInt8)((lastWheel >> 8) & 0xFF),
                                          (UInt8)(crankRevolutions & 0xFF), (UInt8)((crankRevolutions >> 8) & 0xFF),
                                          (UInt8)(lastCrankEventTime & 0xFF), (UInt8)((lastCrankEventTime >> 8) & 0xFF)]
                  let powerData = Data(bytes: &power, count: MemoryLayout.size(ofValue: power))
                  return powerData
                }
        }
    
  func calculateHeartRate() -> Data {
    //self.delegate?.BLEPeripheralManagerDidSendValue(self.heartRate)
    var heartRateBPM: [UInt8] = [0, self.heartRate, 0, 0, 0, 0, 0, 0]
    let heartRateData = Data(bytes: &heartRateBPM, count: MemoryLayout.size(ofValue: heartRateBPM))
    return heartRateData
  }
    
    func calculateIndoorBike() -> Data {
        let flags0:UInt8 = 0x64
        let flags1:UInt8 = 0x02
      //self.delegate?.BLEPeripheralManagerCSCDidSendValue(flags, crankRevolutions: self.crankRevolutions, lastCrankEventTime: self.lastCrankEventTime)
        var indoorBike: [UInt8] = [flags0, flags1, (UInt8)(self.NormalizeSpeed & 0xFF), (UInt8)((self.NormalizeSpeed >> 8) & 0xFF),  (UInt8)(self.CurrentCadence & 0xFF), (UInt8)((self.CurrentCadence >> 8) & 0xFF), self.CurrentResistance, 0x00, (UInt8)(self.CurrentWatt & 0xFF), (UInt8)((self.CurrentWatt >> 8) & 0xFF),
                                   self.heartRate, 0x00]
      let indoorBikeData = Data(bytes: &indoorBike, count: 12)
      return indoorBikeData
    }
  
    var WattBikeSequence: UInt8 = 0
    
  @objc func updateSubscribers() {
      if(self.serviceToggle == 4 || garmin_bluetooth_compatibility || (self.serviceToggle == 3 && !zwift_play_emulator && !watt_bike_emulator))
      {
          let powerData = self.calculatePower()
          let ok = self.peripheralManager.updateValue(powerData, for: self.PowerMeasurementCharacteristic, onSubscribedCentrals: nil)
          if(ok) {
              self.serviceToggle = 0
          }
      } else if(self.serviceToggle == 3) {
          if(watt_bike_emulator) {
            WattBikeSequence = (WattBikeSequence + 1) % 255
              let gearValue = max(0, min(self.CurrentGears, 255))
              let WattBikeArray: [UInt8] = [ WattBikeSequence, 0x03, 0xB6, UInt8(gearValue) ]
            let WattBikeData = Data(bytes: WattBikeArray, count: 4)
            let ok = self.peripheralManager.updateValue(WattBikeData, for: self.WattBikeReadCharacteristic, onSubscribedCentrals: nil)
            if(ok) {
                self.serviceToggle = self.serviceToggle + 1
            }
          } else if(zwift_play_emulator) {
            if(!sendUpdates()) {
                do {
                    let response = try ZwiftHubBike.ridingData(power: UInt32(self.CurrentWatt), cadence: UInt32(self.CurrentCadence / 2), speed: 0, HR: UInt32(self.heartRate), unkown1: self.calculateUnknown1(power: self.CurrentWatt), unkown2: 0)
                    
                    let ok = self.peripheralManager.updateValue(response, for: self.ZwiftPlayReadCharacteristic, onSubscribedCentrals: nil)
                    if(ok) {
                        self.serviceToggle = self.serviceToggle + 1
                    }

                } catch {
                    self.serviceToggle = self.serviceToggle + 1
                }
            } else {
                self.serviceToggle = self.serviceToggle + 1
            }
          }
    } else if(self.serviceToggle == 2) {
      let cadenceData = self.calculateCadence()
      let ok = self.peripheralManager.updateValue(cadenceData, for: self.CSCMeasurementCharacteristic, onSubscribedCentrals: nil)
      if(ok) {
          self.serviceToggle = self.serviceToggle + 1
      }
    } else if(self.serviceToggle == 1) {
        let heartRateData = self.calculateHeartRate()
        let ok = self.peripheralManager.updateValue(heartRateData, for: self.heartRateCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = self.serviceToggle + 1
        }
    } else {
        let indoorBikeData = self.calculateIndoorBike()
        let ok = self.peripheralManager.updateValue(indoorBikeData, for: self.indoorbikeCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = self.serviceToggle + 1
        }
    }
  }
  
} /// class-end
