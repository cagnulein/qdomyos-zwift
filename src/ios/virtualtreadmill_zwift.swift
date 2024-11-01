import CoreBluetooth

let RSCServiceUuid = CBUUID(string: "0x1814");
let RSCFeatureUuid = CBUUID(string: "0x2a54");
let RSCSensorLocationUuid = CBUUID(string: "0x2a5d");
let RSCMeasurementUuid = CBUUID(string: "0x2a53");
let RSCControlPointUuid = CBUUID(string: "0x2a55");

let treadmilldataUuid = CBUUID(string: "0x2ACD");

let WahooRunServiceUuid = CBUUID(string: "A026EE0E-0A7D-4AB3-97FA-F1500F9FEB8B")
let WahooRunNotifyUuid = CBUUID(string: "A026E03D-0A7D-4AB3-97FA-F1500F9FEB8B")
let WahooRunWriteUuid = CBUUID(string: "A026E03E-0A7D-4AB3-97FA-F1500F9FEB8B")

@objc public class virtualtreadmill_zwift: NSObject {
    private var peripheralManager: BLEPeripheralManagerTreadmillZwift!
    
    @objc public override init() {
      super.init()
      peripheralManager = BLEPeripheralManagerTreadmillZwift()
    }
    
    @objc public func updateHeartRate(HeartRate: UInt8)
    {
        peripheralManager.heartRate = HeartRate
    }
    
    @objc public func readCurrentSlope() -> Double
    {
        return peripheralManager.CurrentSlope;
    }

    @objc public func lastChangeCurrentSlope() -> UInt64
    {
        return peripheralManager.lastCurrentSlope;
    }
    
    @objc public func readPowerRequested() -> Double
    {
        return peripheralManager.PowerRequested;
    }
    
    @objc public func updateFTMS(normalizeSpeed: UInt16, currentCadence: UInt16, currentResistance: UInt8, currentWatt: UInt16, currentInclination: UInt16, currentDistance: UInt64) -> Bool
    {
        peripheralManager.NormalizeSpeed = normalizeSpeed
        peripheralManager.CurrentCadence = currentCadence
        peripheralManager.CurrentResistance = currentResistance
        peripheralManager.CurrentWatt = currentWatt
        peripheralManager.CurrentInclination = currentInclination
        peripheralManager.CurrentDistance = currentDistance

        return peripheralManager.connected;
    }
}

class BLEPeripheralManagerTreadmillZwift: NSObject, CBPeripheralManagerDelegate {
  private var peripheralManager: CBPeripheralManager!

  private var heartRateService: CBMutableService!
  private var heartRateCharacteristic: CBMutableCharacteristic!
  public var heartRate:UInt8! = 0

  private var FitnessMachineService: CBMutableService!
  private var FitnessMachineFeatureCharacteristic: CBMutableCharacteristic!
  private var supported_resistance_level_rangeCharacteristic: CBMutableCharacteristic!
  private var FitnessMachineControlPointCharacteristic: CBMutableCharacteristic!
  private var indoorbikeCharacteristic: CBMutableCharacteristic!
  private var treadmilldataCharacteristic: CBMutableCharacteristic!
  private var FitnessMachinestatusCharacteristic: CBMutableCharacteristic!
  private var TrainingStatusCharacteristic: CBMutableCharacteristic!
    public var CurrentSlope: Double! = 0
    public var PowerRequested: Double! = 0
    public var NormalizeSpeed: UInt16! = 0
    public var CurrentCadence: UInt16! = 0
    public var CurrentResistance: UInt8! = 0
    public var CurrentWatt: UInt16! = 0
    public var CurrentInclination: UInt16! = 0
    public var CurrentDistance: UInt64! = 0
    public var lastCurrentSlope: UInt64! = 0;
    
    public var serviceToggle: UInt8 = 0

  private var rscService: CBMutableService!
  private var rscFeatureCharacteristic: CBMutableCharacteristic!
  private var rscSensorLocationCharacteristic: CBMutableCharacteristic!
  private var rscMeasurementCharacteristic: CBMutableCharacteristic!
  private var rscControlPointCharacteristic: CBMutableCharacteristic!
    
  private var WahooRunService: CBMutableService!
    private var WahooRunReadCharacteristic: CBMutableCharacteristic!
    private var WahooRunWriteCharacteristic: CBMutableCharacteristic!
    
  public var connected: Bool = false

  private var notificationTimer: Timer! = nil
  //var delegate: BLEPeripheralManagerDelegate?
    
    let SwiftDebug = swiftDebug()

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

        let FitnessMachineControlPointProperties: CBCharacteristicProperties = [.indicate, .write]
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

        let treadmilldataProperties: CBCharacteristicProperties = [.notify, .read]
          let treadmilldataPermissions: CBAttributePermissions = [.readable]
          self.treadmilldataCharacteristic = CBMutableCharacteristic(type: treadmilldataUuid,
                                               properties: treadmilldataProperties,
                                                           value: nil,
                                                           permissions: treadmilldataPermissions)
        
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
                                               treadmilldataCharacteristic,
                                               FitnessMachinestatusCharacteristic,
                                               TrainingStatusCharacteristic ]
        
        self.peripheralManager.add(FitnessMachineService)
        
        self.rscService = CBMutableService(type: RSCServiceUuid, primary: true)
        
        let rscFeatureProperties: CBCharacteristicProperties = [.read]
        let rscFeaturePermissions: CBAttributePermissions = [.readable]
        self.rscFeatureCharacteristic = CBMutableCharacteristic(type: RSCFeatureUuid,
                                                              properties: rscFeatureProperties,
                                                              value: Data (bytes: [0x02, 0x00]),
                                                              permissions: rscFeaturePermissions)

        let rscSensorLocationProperties: CBCharacteristicProperties = [.read]
        let rscSensorLocationPermissions: CBAttributePermissions = [.readable]
        self.rscSensorLocationCharacteristic = CBMutableCharacteristic(type: RSCSensorLocationUuid,
                                                              properties: rscSensorLocationProperties,
                                                              value: Data (bytes: [0x01]),
                                                              permissions: rscSensorLocationPermissions)

        let rscMeasurementProperties: CBCharacteristicProperties = [.notify, .read]
        let rscMeasurementPermissions: CBAttributePermissions = [.readable]
        self.rscMeasurementCharacteristic = CBMutableCharacteristic(type: RSCMeasurementUuid,
                                                              properties: rscMeasurementProperties,
                                                              value: nil,
                                                              permissions: rscMeasurementPermissions)

        let rscControlPointProperties: CBCharacteristicProperties = [.indicate, .write]
        let rscControlPointPermissions: CBAttributePermissions = [.writeable]
        self.rscControlPointCharacteristic = CBMutableCharacteristic(type: RSCControlPointUuid,
                                                              properties: rscControlPointProperties,
                                                              value: nil,
                                                              permissions: rscControlPointPermissions)
        
        rscService.characteristics = [rscFeatureCharacteristic, rscSensorLocationCharacteristic,
                                      rscMeasurementCharacteristic, rscControlPointCharacteristic ]
        self.peripheralManager.add(rscService)
        
        self.WahooRunService = CBMutableService(type: WahooRunServiceUuid, primary: true)
        
        let WahooRunReadProperties: CBCharacteristicProperties = [.indicate, .writeWithoutResponse]
        let WahooRunReadPermissions: CBAttributePermissions = [.readable,.writeable]
        self.WahooRunReadCharacteristic = CBMutableCharacteristic(type: WahooRunWriteUuid,
                                                          properties: WahooRunReadProperties,
                                                                    value: nil,
                                                                          permissions: WahooRunReadPermissions)

        let WahooRunWriteProperties: CBCharacteristicProperties = [.read, .write, .notify]
        let WahooRunWritePermissions: CBAttributePermissions = [.readable, .writeable]
        self.WahooRunWriteCharacteristic = CBMutableCharacteristic(type: WahooRunNotifyUuid,
                                                    properties: WahooRunWriteProperties,
                                                                  value: nil,
                                                                  permissions: WahooRunWritePermissions)

        WahooRunService.characteristics = [WahooRunReadCharacteristic,
                                           WahooRunWriteCharacteristic]
          self.peripheralManager.add(WahooRunService)

    default:
      print("Peripheral manager is down")
    }
  }
    
  func peripheralManager(_ peripheral: CBPeripheralManager, didAdd service: CBService, error: Error?) {
    if let uwError = error {
      print("Failed to add service with error: \(uwError.localizedDescription)")
      return
    }
    
    let advertisementData = [CBAdvertisementDataLocalNameKey: "KICKR RUN",
                          CBAdvertisementDataServiceUUIDsKey: [heartRateServiceUUID, FitnessMachineServiceUuid, RSCServiceUuid, WahooRunServiceUuid]] as [String : Any]
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
  
    func peripheralManager(_ peripheral: CBPeripheralManager, didReceiveWrite requests: [CBATTRequest]) {
        if let value = requests.first?.value {
              let hexString = value.map { String(format: "%02x", $0) }.joined(separator: " ")
              let debugMessage = "virtualtreadmill_zwift didReceiveWrite: " + String(describing: requests.first!.characteristic) + " " + hexString
              SwiftDebug.qtDebug(debugMessage)
        }
        
        if requests.first!.characteristic == self.FitnessMachineControlPointCharacteristic {
          if(requests.first!.value?.first == 0x11)
          {
                   var high : Int16 = ((Int16)(requests.first!.value![4])) << 8;
                     self.CurrentSlope = (Double)((Int16)(requests.first!.value![3]) + high);
                
                self.lastCurrentSlope = UInt64(Date().timeIntervalSince1970)
          }
          else if(requests.first!.value?.first == 0x03)
          {
              var high : Int16 = ((Int16)(requests.first!.value![2])) << 8;
              self.CurrentSlope = ((Double)((Int16)(requests.first!.value![1]) + high)) * 10.0;
                
              self.lastCurrentSlope = UInt64(Date().timeIntervalSince1970)
          }
            else if(requests.first!.value?.first == 0x05)
          {
                var high : UInt16 = (((UInt16)(requests.first!.value![2])) << 8);
                self.PowerRequested = (Double)((UInt16)(requests.first!.value![1]) + high);
          }
          self.connected = true;
          self.peripheralManager.respond(to: requests.first!, withResult: .success)
          print("Responded successfully to a read request")
           
          let funcCode: UInt8 = requests.first!.value![0]
          var response: [UInt8] = [0x80, funcCode , 0x01]
          let responseData = Data(bytes: &response, count: 3)
            
          self.peripheralManager.updateValue(responseData, for: self.FitnessMachineControlPointCharacteristic, onSubscribedCentrals: nil)
      }
    }
    
  func peripheralManager(_ peripheral: CBPeripheralManager, didReceiveRead request: CBATTRequest) {
    let debugMessage = "virtualtreadmill_zwift didReceiveRead: " + String(describing: request.characteristic)
    SwiftDebug.qtDebug(debugMessage)

    if request.characteristic == self.heartRateCharacteristic {
      request.value = self.calculateHeartRate()
      self.peripheralManager.respond(to: request, withResult: .success)
      print("Responded successfully to a read request")
    }
      else if request.characteristic == self.WahooRunReadCharacteristic {
          var response: [UInt8] = [0x2a, 0x08, 0x03, 0x12, 0x11, 0x22, 0x0f, 0x41, 0x54, 0x58, 0x20, 0x30, 0x34, 0x2c, 0x20, 0x53, 0x54, 0x58, 0x20, 0x30, 0x34, 0x00]
          var responseData = Data(bytes: &response, count: 22)
          request.value = responseData
        self.peripheralManager.respond(to: request, withResult: .success)
        print("Responded successfully to a read request Wahoo Run")
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
        self.notificationTimer = Timer.scheduledTimer(timeInterval: 0.3, target: self, selector: #selector(self.updateSubscribers), userInfo: nil, repeats: true)
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

    func calculateTreadmillData() -> Data {
        let flags0:UInt8 = 0x0C
        let flags1:UInt8 = 0x01
      
        var treadmillData: [UInt8] = [flags0, flags1, (UInt8)(self.NormalizeSpeed & 0xFF), (UInt8)((self.NormalizeSpeed >> 8) & 0xFF),
                                      (UInt8)(self.CurrentDistance & 0xFF), (UInt8)((self.CurrentDistance >> 8) & 0xFF), (UInt8)((self.CurrentDistance >> 16) & 0xFF),
                                      (UInt8)(self.CurrentInclination & 0xFF), (UInt8)((self.CurrentInclination >> 8) & 0xFF),
                                      (UInt8)(self.CurrentInclination & 0xFF), (UInt8)((self.CurrentInclination >> 8) & 0xFF),
                                      self.heartRate]
      let treadmillDataData = Data(bytes: &treadmillData, count: 13)
      return treadmillDataData
    }

    func calculateRSCMeasurement() -> Data {
    // Set flags to indicate distance is present (bit 1)
    let flags0: UInt8 = 0x02
    
    // Calculate speed (existing logic)
    let speed: UInt16 = UInt16(((Double(self.NormalizeSpeed) / 100.0) / 3.6) * 256.0)
    
    // Distance in meters (assuming you have a totalDistance property)
    // Using UInt32 as per BLE specification for distance
    let distance: UInt32 = UInt32(self.CurrentDistance) * 10
    
    // Create measurement array with distance
    var rscMeasurement: [UInt8] = [
        flags0,                              // Flags
        UInt8(speed & 0xFF),                // Speed LSB
        UInt8((speed >> 8) & 0xFF),         // Speed MSB
        UInt8(self.CurrentCadence & 0xFF),  // Cadence
        UInt8(distance & 0xFF),             // Distance LSB
        UInt8((distance >> 8) & 0xFF),      // Distance byte 2
        UInt8((distance >> 16) & 0xFF),     // Distance byte 3
        UInt8((distance >> 24) & 0xFF)      // Distance MSB
    ]
    
    let rscMeasurementData = Data(bytes: &rscMeasurement, count: 8)
    return rscMeasurementData
}
    
  @objc func updateSubscribers() {
    let heartRateData = self.calculateHeartRate()
    let indoorBikeData = self.calculateIndoorBike()
    let treadmillData = self.calculateTreadmillData()
    let rscMeasurementData = self.calculateRSCMeasurement()
    
    if(self.serviceToggle == 0)
    {
        let ok = self.peripheralManager.updateValue(heartRateData, for: self.heartRateCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = 1;
        }
    }
    else if(self.serviceToggle == 1)
    {
        let ok = self.peripheralManager.updateValue(treadmillData, for: self.treadmilldataCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = 2;
        }
    }
    else if(self.serviceToggle == 2)
    {
        let ok = self.peripheralManager.updateValue(rscMeasurementData, for: self.rscMeasurementCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = 3;
        }
    }
    else
    {
        let ok = self.peripheralManager.updateValue(indoorBikeData, for: self.indoorbikeCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = 0;
        }
    }
  }
  
} /// class-end
