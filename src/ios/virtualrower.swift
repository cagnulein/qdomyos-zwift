import CoreBluetooth

let rowerUuid = CBUUID(string: "0x2AD1");

@objc public class virtualrower_zwift: NSObject {
    private var peripheralManager: rowerBLEPeripheralManagerZwift!
    
    @objc public override init() {
      super.init()
      peripheralManager = rowerBLEPeripheralManagerZwift()
    }
    
    @objc public func updateHeartRate(HeartRate: UInt8)
    {
        peripheralManager.heartRate = HeartRate
    }
    
    @objc public func readCurrentSlope() -> Double
    {
        return peripheralManager.CurrentSlope;
    }

    @objc public func readPowerRequested() -> Double
    {
        return peripheralManager.PowerRequested;
    }
    
    @objc public func updateFTMS(normalizeSpeed: UInt16, currentCadence: UInt16, currentResistance: UInt8, currentWatt: UInt16, CrankRevolutions: UInt16, LastCrankEventTime: UInt16, StrokesCount: UInt16, Distance: UInt32, KCal: UInt16, Pace: UInt16  ) -> Bool
    {
        peripheralManager.NormalizeSpeed = normalizeSpeed
        peripheralManager.CurrentCadence = currentCadence
        peripheralManager.CurrentResistance = currentResistance
        peripheralManager.CurrentWatt = currentWatt
        peripheralManager.lastCrankEventTime = LastCrankEventTime
        peripheralManager.crankRevolutions = CrankRevolutions
        peripheralManager.Distance = Distance
        peripheralManager.KCal = KCal
        peripheralManager.Pace = Pace
        peripheralManager.StrokesCount = StrokesCount

        return peripheralManager.connected;
    }
    
    @objc public func getLastFTMSMessage() -> Data? {
        peripheralManager.LastFTMSMessageReceivedAndPassed = peripheralManager.LastFTMSMessageReceived
        peripheralManager.LastFTMSMessageReceived?.removeAll()
        return peripheralManager.LastFTMSMessageReceivedAndPassed
    }
}

class rowerBLEPeripheralManagerZwift: NSObject, CBPeripheralManagerDelegate {
  private var peripheralManager: CBPeripheralManager!

  private var heartRateService: CBMutableService!
  private var heartRateCharacteristic: CBMutableCharacteristic!
  public var heartRate:UInt8! = 0

  private var FitnessMachineService: CBMutableService!
  private var FitnessMachineFeatureCharacteristic: CBMutableCharacteristic!
  private var supported_resistance_level_rangeCharacteristic: CBMutableCharacteristic!
  private var FitnessMachineControlPointCharacteristic: CBMutableCharacteristic!
  private var rowerCharacteristic: CBMutableCharacteristic!
  private var FitnessMachinestatusCharacteristic: CBMutableCharacteristic!
  private var TrainingStatusCharacteristic: CBMutableCharacteristic!
    public var CurrentSlope: Double! = 0
    public var PowerRequested: Double! = 0
    public var NormalizeSpeed: UInt16! = 0
    public var CurrentCadence: UInt16! = 0
    public var CurrentResistance: UInt8! = 0
    public var CurrentWatt: UInt16! = 0
    public var Pace: UInt16! = 0
    public var Distance: UInt32! = 0
    public var KCal: UInt16! = 0
    public var StrokesCount: UInt16! = 0
    
  private var CSCService: CBMutableService!
  private var CSCFeatureCharacteristic: CBMutableCharacteristic!
  private var SensorLocationCharacteristic: CBMutableCharacteristic!
  private var CSCMeasurementCharacteristic: CBMutableCharacteristic!
  private var SCControlPointCharacteristic: CBMutableCharacteristic!
    public var crankRevolutions: UInt16! = 0
    public var lastCrankEventTime: UInt16! = 0
    
    public var LastFTMSMessageReceived: Data?
    public var LastFTMSMessageReceivedAndPassed: Data?
    
    public var serviceToggle: UInt8 = 0

  public var connected: Bool = false

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

      let rowerProperties: CBCharacteristicProperties = [.notify, .read]
        let rowerPermissions: CBAttributePermissions = [.readable]
        self.rowerCharacteristic = CBMutableCharacteristic(type: rowerUuid,
                                             properties: rowerProperties,
                                                         value: nil,
                                                         permissions: rowerPermissions)
        
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
                                               rowerCharacteristic,
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
    
    let advertisementData = [CBAdvertisementDataLocalNameKey: "QZ",
                              CBAdvertisementDataServiceUUIDsKey: [heartRateServiceUUID, FitnessMachineServiceUuid, CSCServiceUUID]] as [String : Any]
    
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
    if requests.first!.characteristic == self.FitnessMachineControlPointCharacteristic {
        if(LastFTMSMessageReceived == nil || LastFTMSMessageReceived?.count == 0) {
            LastFTMSMessageReceived = requests.first!.value
        }
        if(requests.first!.value?.first == 0x11)
        {
               var high : Int16 = ((Int16)(requests.first!.value![4])) << 8;
                 self.CurrentSlope = (Double)((Int16)(requests.first!.value![3]) + high);
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
    else if request.characteristic == self.rowerCharacteristic {
        request.value = self.calculateRower()
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
    
  func calculateHeartRate() -> Data {
    //self.delegate?.BLEPeripheralManagerDidSendValue(self.heartRate)
    var heartRateBPM: [UInt8] = [0, self.heartRate, 0, 0, 0, 0, 0, 0]
    let heartRateData = Data(bytes: &heartRateBPM, count: MemoryLayout.size(ofValue: heartRateBPM))
    return heartRateData
  }
    
    func calculateRower() -> Data {
        let flags0:UInt8 = 0x2C
        let flags1:UInt8 = 0x03
      //self.delegate?.BLEPeripheralManagerCSCDidSendValue(flags, crankRevolutions: self.crankRevolutions, lastCrankEventTime: self.lastCrankEventTime)
        var rower: [UInt8] = [flags0, flags1,  (UInt8)(self.CurrentCadence & 0xFF), (UInt8)(self.StrokesCount & 0xFF), (UInt8)((self.StrokesCount >> 8) & 0xFF), (UInt8)(self.Distance & 0xFF), (UInt8)((self.Distance >> 8) & 0xFF), (UInt8)((self.Distance >> 16) & 0xFF), (UInt8)(self.Pace & 0xFF), (UInt8)((self.Pace >> 8) & 0xFF), (UInt8)(self.CurrentWatt & 0xFF), (UInt8)((self.CurrentWatt >> 8) & 0xFF), (UInt8)(self.KCal & 0xFF), (UInt8)((self.KCal >> 8) & 0xFF), (UInt8)(self.KCal & 0xFF), (UInt8)((self.KCal >> 8) & 0xFF), (UInt8)(self.KCal & 0xFF), self.heartRate, 0x00]
      let rowerData = Data(bytes: &rower, count: 19)
      return rowerData
    }
  
  @objc func updateSubscribers() {
    let heartRateData = self.calculateHeartRate()
    let rowerData = self.calculateRower()
    let cadenceData = self.calculateCadence()
    
    if(self.serviceToggle == 2)
    {
      let ok = self.peripheralManager.updateValue(cadenceData, for: self.CSCMeasurementCharacteristic, onSubscribedCentrals: nil)
      if(ok) {
          self.serviceToggle = 0
      }
    }
    else if(self.serviceToggle == 1)
    {
        let ok = self.peripheralManager.updateValue(heartRateData, for: self.heartRateCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = self.serviceToggle + 1
        }
    }
    else
    {
        let ok = self.peripheralManager.updateValue(rowerData, for: self.rowerCharacteristic, onSubscribedCentrals: nil)
        if(ok) {
            self.serviceToggle = self.serviceToggle + 1
        }
    }
  }
  
} /// class-end
