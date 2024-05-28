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
    
    @objc public init(disable_hr: Bool, garmin_bluetooth_compatibility: Bool, tacx: Bool) {
      super.init()
        peripheralManager = BLEPeripheralManagerZwift(disable_hr: disable_hr, garmin_bluetooth_compatibility: garmin_bluetooth_compatibility, tacx: tacx)
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
    
    @objc public func updateFTMS(normalizeSpeed: UInt16, currentCadence: UInt16, currentResistance: UInt8, currentWatt: UInt16, CrankRevolutions: UInt16, LastCrankEventTime: UInt16) -> Bool
    {
        peripheralManager.NormalizeSpeed = normalizeSpeed
        peripheralManager.CurrentCadence = currentCadence
        peripheralManager.CurrentResistance = currentResistance
        peripheralManager.CurrentWatt = currentWatt
        peripheralManager.lastCrankEventTime = LastCrankEventTime
        peripheralManager.crankRevolutions = CrankRevolutions

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
    private var tacx: Bool = false
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

    
    public var LastFTMSMessageReceived: Data?
    public var LastFTMSMessageReceivedAndPassed: Data?
    
    public var serviceToggle: UInt8 = 0

  public var connected: Bool = false

  private var notificationTimer: Timer! = nil
  //var delegate: BLEPeripheralManagerDelegate?

  init(disable_hr: Bool, garmin_bluetooth_compatibility: Bool, tacx: Bool) {
    super.init()
    self.disable_hr = disable_hr
    self.tacx = tacx
    self.garmin_bluetooth_compatibility = garmin_bluetooth_compatibility

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

        
    default:
      print("Peripheral manager is down")
    }
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
      } else if(tacx) {
          let advertisementData = [CBAdvertisementDataLocalNameKey: "Tacx Neo 17867",
                                CBAdvertisementDataServiceUUIDsKey: [PowerServiceUUID, CSCServiceUUID]] as [String : Any]
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
  
    func peripheralManager(_ peripheral: CBPeripheralManager, didReceiveWrite requests: [CBATTRequest]) {
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
        print("Responded successfully to a read request")

        let funcCode: UInt8 = requests.first!.value![0]
        var response: [UInt8] = [0x80, funcCode , 0x01]
        let responseData = Data(bytes: &response, count: 3)
          
        self.peripheralManager.updateValue(responseData, for: self.FitnessMachineControlPointCharacteristic, onSubscribedCentrals: nil)
        }
    }
    
  func peripheralManager(_ peripheral: CBPeripheralManager, didReceiveRead request: CBATTRequest) {
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
        self.notificationTimer = Timer.scheduledTimer(timeInterval: 0.2, target: self, selector: #selector(self.updateSubscribers), userInfo: nil, repeats: true)
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
  
  @objc func updateSubscribers() {
    if(self.serviceToggle == 3 || garmin_bluetooth_compatibility)
    {
        let powerData = self.calculatePower()
        let ok = self.peripheralManager.updateValue(powerData, for: self.PowerMeasurementCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = 0
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
