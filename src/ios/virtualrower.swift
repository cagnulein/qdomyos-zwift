import CoreBluetooth

let rowerUuid = CBUUID(string: "0x2AD1");

// PM5 Concept2 UUIDs
let PM5_DISCOVERY_SERVICE_UUID = CBUUID(string: "CE060000-43E5-11E4-916C-0800200C9A66")
let PM5_DEVICE_INFO_SERVICE_UUID = CBUUID(string: "CE060010-43E5-11E4-916C-0800200C9A66")
let PM5_CONTROL_SERVICE_UUID = CBUUID(string: "CE060020-43E5-11E4-916C-0800200C9A66")
let PM5_ROWING_SERVICE_UUID = CBUUID(string: "CE060030-43E5-11E4-916C-0800200C9A66")

// PM5 Device Info characteristics
let PM5_MODEL_UUID = CBUUID(string: "CE060011-43E5-11E4-916C-0800200C9A66")
let PM5_SERIAL_UUID = CBUUID(string: "CE060012-43E5-11E4-916C-0800200C9A66")
let PM5_HARDWARE_REV_UUID = CBUUID(string: "CE060013-43E5-11E4-916C-0800200C9A66")
let PM5_FIRMWARE_REV_UUID = CBUUID(string: "CE060014-43E5-11E4-916C-0800200C9A66")
let PM5_MANUFACTURER_UUID = CBUUID(string: "CE060015-43E5-11E4-916C-0800200C9A66")
let PM5_ERG_MACHINE_TYPE_UUID = CBUUID(string: "CE060016-43E5-11E4-916C-0800200C9A66")

// PM5 Control characteristics
let PM5_CONTROL_RECEIVE_UUID = CBUUID(string: "CE060021-43E5-11E4-916C-0800200C9A66")
let PM5_CONTROL_TRANSMIT_UUID = CBUUID(string: "CE060022-43E5-11E4-916C-0800200C9A66")

// PM5 Rowing characteristics
let PM5_GENERAL_STATUS_UUID = CBUUID(string: "CE060031-43E5-11E4-916C-0800200C9A66")
let PM5_ADDITIONAL_STATUS_UUID = CBUUID(string: "CE060032-43E5-11E4-916C-0800200C9A66")
let PM5_ADDITIONAL_STATUS2_UUID = CBUUID(string: "CE060033-43E5-11E4-916C-0800200C9A66")
let PM5_SAMPLE_RATE_UUID = CBUUID(string: "CE060034-43E5-11E4-916C-0800200C9A66")
let PM5_STROKE_DATA_UUID = CBUUID(string: "CE060035-43E5-11E4-916C-0800200C9A66")
let PM5_ADDITIONAL_STROKE_DATA_UUID = CBUUID(string: "CE060036-43E5-11E4-916C-0800200C9A66")
let PM5_MULTIPLEXED_INFO_UUID = CBUUID(string: "CE060080-43E5-11E4-916C-0800200C9A66")

@objc public class virtualrower_zwift: NSObject {
    private var peripheralManager: rowerBLEPeripheralManagerZwift!

    @objc public override init() {
      super.init()
      peripheralManager = rowerBLEPeripheralManagerZwift(pm5Mode: false)
    }

    @objc public init(pm5Mode: Bool) {
      super.init()
      peripheralManager = rowerBLEPeripheralManagerZwift(pm5Mode: pm5Mode)
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

    @objc public func setPM5Mode(enabled: Bool)
    {
        peripheralManager.pm5Mode = enabled
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
    public var pm5ServiceToggle: UInt8 = 0

  public var connected: Bool = false

  private var notificationTimer: Timer! = nil

  // PM5 Mode
  public var pm5Mode: Bool = false
  private var startTime: Date = Date()
  private var pm5SampleRate: UInt8 = 0x01

  // PM5 Services
  private var PM5DeviceInfoService: CBMutableService!
  private var PM5ControlService: CBMutableService!
  private var PM5RowingService: CBMutableService!

  // PM5 Device Info Characteristics
  private var PM5ModelCharacteristic: CBMutableCharacteristic!
  private var PM5SerialCharacteristic: CBMutableCharacteristic!
  private var PM5HardwareRevCharacteristic: CBMutableCharacteristic!
  private var PM5FirmwareRevCharacteristic: CBMutableCharacteristic!
  private var PM5ManufacturerCharacteristic: CBMutableCharacteristic!
  private var PM5ErgMachineTypeCharacteristic: CBMutableCharacteristic!

  // PM5 Control Characteristics
  private var PM5ControlReceiveCharacteristic: CBMutableCharacteristic!
  private var PM5ControlTransmitCharacteristic: CBMutableCharacteristic!

  // PM5 Rowing Characteristics
  private var PM5GeneralStatusCharacteristic: CBMutableCharacteristic!
  private var PM5AdditionalStatusCharacteristic: CBMutableCharacteristic!
  private var PM5AdditionalStatus2Characteristic: CBMutableCharacteristic!
  private var PM5SampleRateCharacteristic: CBMutableCharacteristic!
  private var PM5StrokeDataCharacteristic: CBMutableCharacteristic!
  private var PM5AdditionalStrokeDataCharacteristic: CBMutableCharacteristic!
  private var PM5MultiplexedInfoCharacteristic: CBMutableCharacteristic!

  override init() {
    super.init()
    pm5Mode = false
    startTime = Date()
    peripheralManager = CBPeripheralManager(delegate: self, queue: nil)
  }

  init(pm5Mode: Bool) {
    super.init()
    self.pm5Mode = pm5Mode
    startTime = Date()
    peripheralManager = CBPeripheralManager(delegate: self, queue: nil)
  }
  
  func peripheralManagerDidUpdateState(_ peripheral: CBPeripheralManager) {
    switch peripheral.state {
    case .poweredOn:
      print("Peripheral manager is up and running")

      // Heart Rate Service (always added)
      self.heartRateService = CBMutableService(type: heartRateServiceUUID, primary: true)
      let characteristicProperties: CBCharacteristicProperties = [.notify, .read, .write]
      let characteristicPermissions: CBAttributePermissions = [.readable]
      self.heartRateCharacteristic = CBMutableCharacteristic(type: heartRateCharacteristicUUID,
                                                            properties: characteristicProperties,
                                                            value: nil,
                                                            permissions: characteristicPermissions)

      heartRateService.characteristics = [heartRateCharacteristic]
      self.peripheralManager.add(heartRateService)

      if pm5Mode {
        // PM5 Mode - Setup PM5 Services
        setupPM5Services()
      } else {
        // FTMS Mode - Original implementation
        setupFTMSServices()
      }

    default:
      print("Peripheral manager is down")
    }
  }

  func setupFTMSServices() {
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
  }

  func setupPM5Services() {
      print("Setting up PM5 services")

      // PM5 Device Info Service (CE060010)
      self.PM5DeviceInfoService = CBMutableService(type: PM5_DEVICE_INFO_SERVICE_UUID, primary: true)

      self.PM5ModelCharacteristic = CBMutableCharacteristic(type: PM5_MODEL_UUID,
                                                            properties: [.read],
                                                            value: "PM5".data(using: .utf8),
                                                            permissions: [.readable])

      self.PM5SerialCharacteristic = CBMutableCharacteristic(type: PM5_SERIAL_UUID,
                                                             properties: [.read],
                                                             value: "430000000".data(using: .utf8),
                                                             permissions: [.readable])

      self.PM5HardwareRevCharacteristic = CBMutableCharacteristic(type: PM5_HARDWARE_REV_UUID,
                                                                  properties: [.read],
                                                                  value: "802".data(using: .utf8),
                                                                  permissions: [.readable])

      self.PM5FirmwareRevCharacteristic = CBMutableCharacteristic(type: PM5_FIRMWARE_REV_UUID,
                                                                  properties: [.read],
                                                                  value: "2.18".data(using: .utf8),
                                                                  permissions: [.readable])

      self.PM5ManufacturerCharacteristic = CBMutableCharacteristic(type: PM5_MANUFACTURER_UUID,
                                                                   properties: [.read],
                                                                   value: "Concept2".data(using: .utf8),
                                                                   permissions: [.readable])

      self.PM5ErgMachineTypeCharacteristic = CBMutableCharacteristic(type: PM5_ERG_MACHINE_TYPE_UUID,
                                                                     properties: [.read],
                                                                     value: Data([0x00]), // 0 = Rower
                                                                     permissions: [.readable])

      PM5DeviceInfoService.characteristics = [PM5ModelCharacteristic, PM5SerialCharacteristic,
                                              PM5HardwareRevCharacteristic, PM5FirmwareRevCharacteristic,
                                              PM5ManufacturerCharacteristic, PM5ErgMachineTypeCharacteristic]
      self.peripheralManager.add(PM5DeviceInfoService)

      // PM5 Control Service (CE060020)
      self.PM5ControlService = CBMutableService(type: PM5_CONTROL_SERVICE_UUID, primary: true)

      self.PM5ControlReceiveCharacteristic = CBMutableCharacteristic(type: PM5_CONTROL_RECEIVE_UUID,
                                                                     properties: [.write, .writeWithoutResponse],
                                                                     value: nil,
                                                                     permissions: [.writeable])

      self.PM5ControlTransmitCharacteristic = CBMutableCharacteristic(type: PM5_CONTROL_TRANSMIT_UUID,
                                                                      properties: [.indicate],
                                                                      value: nil,
                                                                      permissions: [.readable])

      PM5ControlService.characteristics = [PM5ControlReceiveCharacteristic, PM5ControlTransmitCharacteristic]
      self.peripheralManager.add(PM5ControlService)

      // PM5 Rowing Service (CE060030)
      self.PM5RowingService = CBMutableService(type: PM5_ROWING_SERVICE_UUID, primary: true)

      self.PM5GeneralStatusCharacteristic = CBMutableCharacteristic(type: PM5_GENERAL_STATUS_UUID,
                                                                    properties: [.notify],
                                                                    value: nil,
                                                                    permissions: [.readable])

      self.PM5AdditionalStatusCharacteristic = CBMutableCharacteristic(type: PM5_ADDITIONAL_STATUS_UUID,
                                                                       properties: [.notify],
                                                                       value: nil,
                                                                       permissions: [.readable])

      self.PM5AdditionalStatus2Characteristic = CBMutableCharacteristic(type: PM5_ADDITIONAL_STATUS2_UUID,
                                                                        properties: [.notify],
                                                                        value: nil,
                                                                        permissions: [.readable])

      self.PM5SampleRateCharacteristic = CBMutableCharacteristic(type: PM5_SAMPLE_RATE_UUID,
                                                                 properties: [.read, .write],
                                                                 value: nil,
                                                                 permissions: [.readable, .writeable])

      self.PM5StrokeDataCharacteristic = CBMutableCharacteristic(type: PM5_STROKE_DATA_UUID,
                                                                 properties: [.notify],
                                                                 value: nil,
                                                                 permissions: [.readable])

      self.PM5AdditionalStrokeDataCharacteristic = CBMutableCharacteristic(type: PM5_ADDITIONAL_STROKE_DATA_UUID,
                                                                           properties: [.notify],
                                                                           value: nil,
                                                                           permissions: [.readable])

      self.PM5MultiplexedInfoCharacteristic = CBMutableCharacteristic(type: PM5_MULTIPLEXED_INFO_UUID,
                                                                      properties: [.notify],
                                                                      value: nil,
                                                                      permissions: [.readable])

      PM5RowingService.characteristics = [PM5GeneralStatusCharacteristic, PM5AdditionalStatusCharacteristic,
                                          PM5AdditionalStatus2Characteristic, PM5SampleRateCharacteristic,
                                          PM5StrokeDataCharacteristic, PM5AdditionalStrokeDataCharacteristic,
                                          PM5MultiplexedInfoCharacteristic]
      self.peripheralManager.add(PM5RowingService)
  }
    
  func peripheralManager(_ peripheral: CBPeripheralManager, didAdd service: CBService, error: Error?) {
    if let uwError = error {
      print("Failed to add service with error: \(uwError.localizedDescription)")
      return
    }

    var advertisementData: [String: Any]

    if pm5Mode {
      // PM5 advertising - device name + PM5 discovery service UUID
      advertisementData = [CBAdvertisementDataLocalNameKey: "PM5 430000000",
                           CBAdvertisementDataServiceUUIDsKey: [PM5_DISCOVERY_SERVICE_UUID]] as [String : Any]
    } else {
      // FTMS advertising
      advertisementData = [CBAdvertisementDataLocalNameKey: "QZ",
                           CBAdvertisementDataServiceUUIDsKey: [heartRateServiceUUID, FitnessMachineServiceUuid, CSCServiceUUID]] as [String : Any]
    }

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
    if requests.first!.characteristic == self.PM5SampleRateCharacteristic {
        if let value = requests.first!.value, value.count > 0 {
            self.pm5SampleRate = value[0]
            print("PM5 sample rate set to \(self.pm5SampleRate)")
        }
        self.peripheralManager.respond(to: requests.first!, withResult: .success)
        return
    }
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
    else if request.characteristic == self.PM5SampleRateCharacteristic {
        request.value = Data([self.pm5SampleRate])
        self.peripheralManager.respond(to: request, withResult: .success)
        print("Responded successfully to PM5 sample rate read request")
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
      let cadenceData = Data(bytes: &cadence, count: cadence.count)
      return cadenceData
    }
    
  func calculateHeartRate() -> Data {
    //self.delegate?.BLEPeripheralManagerDidSendValue(self.heartRate)
    var heartRateBPM: [UInt8] = [0, self.heartRate, 0, 0, 0, 0, 0, 0]
    let heartRateData = Data(bytes: &heartRateBPM, count: heartRateBPM.count)
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
    if pm5Mode {
      updatePM5Subscribers()
    } else {
      updateFTMSSubscribers()
    }
  }

  func updateFTMSSubscribers() {
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

  func updatePM5Subscribers() {
    guard PM5GeneralStatusCharacteristic != nil else { return }

    var ok = false

    switch pm5ServiceToggle {
    case 0:
      // Send General Status
      let generalStatus = buildPM5GeneralStatus()
      ok = self.peripheralManager.updateValue(generalStatus, for: self.PM5GeneralStatusCharacteristic, onSubscribedCentrals: nil)
      if ok && PM5MultiplexedInfoCharacteristic != nil {
        var muxGeneralStatus = Data([0x31])
        muxGeneralStatus.append(generalStatus)
        _ = self.peripheralManager.updateValue(muxGeneralStatus, for: self.PM5MultiplexedInfoCharacteristic, onSubscribedCentrals: nil)
      }

    case 1:
      // Send Additional Status
      let additionalStatus = buildPM5AdditionalStatus()
      ok = self.peripheralManager.updateValue(additionalStatus, for: self.PM5AdditionalStatusCharacteristic, onSubscribedCentrals: nil)
      if ok && PM5MultiplexedInfoCharacteristic != nil {
        var muxAdditionalStatus = Data([0x32])
        muxAdditionalStatus.append(additionalStatus)
        _ = self.peripheralManager.updateValue(muxAdditionalStatus, for: self.PM5MultiplexedInfoCharacteristic, onSubscribedCentrals: nil)
      }

    case 2:
      // Send Additional Status 2
      let additionalStatus2 = buildPM5AdditionalStatus2()
      ok = self.peripheralManager.updateValue(additionalStatus2, for: self.PM5AdditionalStatus2Characteristic, onSubscribedCentrals: nil)
      if ok && PM5MultiplexedInfoCharacteristic != nil {
        var muxAdditionalStatus2 = Data([0x33])
        muxAdditionalStatus2.append(additionalStatus2)
        _ = self.peripheralManager.updateValue(muxAdditionalStatus2, for: self.PM5MultiplexedInfoCharacteristic, onSubscribedCentrals: nil)
      }

    case 3:
      // Send Stroke Data
      let strokeData = buildPM5StrokeData()
      ok = self.peripheralManager.updateValue(strokeData, for: self.PM5StrokeDataCharacteristic, onSubscribedCentrals: nil)
      if ok && PM5MultiplexedInfoCharacteristic != nil {
        var muxStrokeData = Data([0x35])
        muxStrokeData.append(strokeData)
        _ = self.peripheralManager.updateValue(muxStrokeData, for: self.PM5MultiplexedInfoCharacteristic, onSubscribedCentrals: nil)
      }

    case 4:
      // Send Additional Stroke Data
      let additionalStrokeData = buildPM5AdditionalStrokeData()
      ok = self.peripheralManager.updateValue(additionalStrokeData, for: self.PM5AdditionalStrokeDataCharacteristic, onSubscribedCentrals: nil)
      if ok && PM5MultiplexedInfoCharacteristic != nil {
        var muxAdditionalStrokeData = Data([0x36])
        muxAdditionalStrokeData.append(additionalStrokeData)
        _ = self.peripheralManager.updateValue(muxAdditionalStrokeData, for: self.PM5MultiplexedInfoCharacteristic, onSubscribedCentrals: nil)
      }

    default:
      break
    }

    // Advance to next characteristic if update was successful
    if ok {
      pm5ServiceToggle += 1
      if pm5ServiceToggle > 4 {
        pm5ServiceToggle = 0
      }
    }
  }

  func getElapsedCentiseconds() -> UInt32 {
    let elapsed = Date().timeIntervalSince(startTime)
    return UInt32(elapsed * 100)
  }

  func buildPM5GeneralStatus() -> Data {
    // 19 bytes
    var value = Data(count: 19)
    let elapsed = getElapsedCentiseconds()

    // Elapsed time (24-bit LE)
    value[0] = UInt8(elapsed & 0xFF)
    value[1] = UInt8((elapsed >> 8) & 0xFF)
    value[2] = UInt8((elapsed >> 16) & 0xFF)

    // Distance in 0.1m units (24-bit LE)
    let distanceDecimeters = UInt32(Double(Distance) / 100.0) // Distance is in mm, convert to 0.1m
    value[3] = UInt8(distanceDecimeters & 0xFF)
    value[4] = UInt8((distanceDecimeters >> 8) & 0xFF)
    value[5] = UInt8((distanceDecimeters >> 16) & 0xFF)

    // Workout Type - 0 = Just Row Free
    value[6] = 0x00
    // Interval Type - 0 = None
    value[7] = 0x00
    // Workout State - 1 = Working
    value[8] = (CurrentWatt > 0 || CurrentCadence > 0) ? 0x01 : 0x00
    // Rowing State - 1 = Active
    value[9] = (CurrentWatt > 0 || CurrentCadence > 0) ? 0x01 : 0x00
    // Stroke State - 1 = Driving
    value[10] = (CurrentWatt > 0) ? 0x01 : 0x00

    // Total Work Distance in meters (24-bit LE)
    let totalDistanceMeters = UInt32(Double(Distance) / 1000.0)
    value[11] = UInt8(totalDistanceMeters & 0xFF)
    value[12] = UInt8((totalDistanceMeters >> 8) & 0xFF)
    value[13] = UInt8((totalDistanceMeters >> 16) & 0xFF)

    // Workout Duration (target) - 0
    value[14] = 0x00
    value[15] = 0x00
    value[16] = 0x00
    value[17] = 0x00 // Duration type
    value[18] = 110 // Drag factor

    return value
  }

  func buildPM5AdditionalStatus() -> Data {
    // 19 bytes
    var value = Data(count: 19)
    let elapsed = getElapsedCentiseconds()

    // Elapsed time (24-bit LE)
    value[0] = UInt8(elapsed & 0xFF)
    value[1] = UInt8((elapsed >> 8) & 0xFF)
    value[2] = UInt8((elapsed >> 16) & 0xFF)

    // Speed in 0.001 m/s units (16-bit LE)
    let speedMmPerSec = UInt16(Double(NormalizeSpeed) * 1000.0 / 3600.0)
    value[3] = UInt8(speedMmPerSec & 0xFF)
    value[4] = UInt8((speedMmPerSec >> 8) & 0xFF)

    // Stroke Rate (SPM)
    value[5] = UInt8(CurrentCadence & 0xFF)

    // Heart Rate
    value[6] = heartRate

    // Current Pace in 0.01 sec/500m (16-bit LE)
    value[7] = UInt8(Pace & 0xFF)
    value[8] = UInt8((Pace >> 8) & 0xFF)

    // Average Pace (same as current)
    value[9] = value[7]
    value[10] = value[8]

    // Rest Distance - 0
    value[11] = 0x00
    value[12] = 0x00

    // Rest Time - 0
    value[13] = 0x00
    value[14] = 0x00
    value[15] = 0x00

    // Average Power (16-bit LE)
    value[16] = UInt8(CurrentWatt & 0xFF)
    value[17] = UInt8((CurrentWatt >> 8) & 0xFF)

    // Erg Machine Type - 0 = Rower
    value[18] = 0x00

    return value
  }

  func buildPM5AdditionalStatus2() -> Data {
    // 20 bytes
    var value = Data(count: 20)
    let elapsed = getElapsedCentiseconds()

    // Elapsed time (24-bit LE)
    value[0] = UInt8(elapsed & 0xFF)
    value[1] = UInt8((elapsed >> 8) & 0xFF)
    value[2] = UInt8((elapsed >> 16) & 0xFF)

    // Interval Count
    value[3] = 0x01

    // Average Power (16-bit LE)
    value[4] = UInt8(CurrentWatt & 0xFF)
    value[5] = UInt8((CurrentWatt >> 8) & 0xFF)

    // Total Calories (16-bit LE)
    value[6] = UInt8(KCal & 0xFF)
    value[7] = UInt8((KCal >> 8) & 0xFF)

    // Split Average Pace (16-bit LE)
    value[8] = UInt8(Pace & 0xFF)
    value[9] = UInt8((Pace >> 8) & 0xFF)

    // Split Average Power (16-bit LE)
    value[10] = value[4]
    value[11] = value[5]

    // Split Average Calories (kCal/hour) - 0
    value[12] = 0x00
    value[13] = 0x00

    // Last Split Time - 0
    value[14] = 0x00
    value[15] = 0x00
    value[16] = 0x00

    // Last Split Distance - 0
    value[17] = 0x00
    value[18] = 0x00
    value[19] = 0x00

    return value
  }

  func buildPM5StrokeData() -> Data {
    // 20 bytes
    var value = Data(count: 20)
    let elapsed = getElapsedCentiseconds()

    // Elapsed time (24-bit LE)
    value[0] = UInt8(elapsed & 0xFF)
    value[1] = UInt8((elapsed >> 8) & 0xFF)
    value[2] = UInt8((elapsed >> 16) & 0xFF)

    // Distance in 0.1m units (24-bit LE)
    let distanceDecimeters = UInt32(Double(Distance) / 100.0)
    value[3] = UInt8(distanceDecimeters & 0xFF)
    value[4] = UInt8((distanceDecimeters >> 8) & 0xFF)
    value[5] = UInt8((distanceDecimeters >> 16) & 0xFF)

    // Drive Length (0.01m) - typical 1.4m
    value[6] = 140
    // Drive Time (0.01s) - typical 0.8s
    value[7] = 80

    // Stroke Recovery Time (16-bit LE, 0.01s)
    var recoveryTime: UInt16 = 170 // default 1.7s
    if CurrentCadence > 0 {
      let strokeTime = 60.0 / Double(CurrentCadence)
      let recoveryTimeDouble = max(0.5, strokeTime - 0.8) * 100.0
      recoveryTime = UInt16(recoveryTimeDouble)
      if recoveryTime < 50 { recoveryTime = 50 }
    }
    value[8] = UInt8(recoveryTime & 0xFF)
    value[9] = UInt8((recoveryTime >> 8) & 0xFF)

    // Stroke Distance (16-bit LE, 0.01m)
    var strokeDistance: UInt16 = 1000 // default 10m
    if CurrentCadence > 0 && NormalizeSpeed > 0 {
      let speedMs = Double(NormalizeSpeed) / 3.6
      let strokeTime = 60.0 / Double(CurrentCadence)
      strokeDistance = UInt16(speedMs * strokeTime * 100.0)
    }
    value[10] = UInt8(strokeDistance & 0xFF)
    value[11] = UInt8((strokeDistance >> 8) & 0xFF)

    // Peak Drive Force (16-bit LE, 0.1 lbs) - estimate from power
    var peakForce: UInt16 = 0
    if CurrentWatt > 0 && NormalizeSpeed > 0 {
      let speedMs = Double(NormalizeSpeed) / 3.6
      let forceN = Double(CurrentWatt) / speedMs
      peakForce = UInt16(forceN * 0.2248 * 10.0 * 1.5)
    }
    value[12] = UInt8(peakForce & 0xFF)
    value[13] = UInt8((peakForce >> 8) & 0xFF)

    // Average Drive Force (16-bit LE, 0.1 lbs)
    let avgForce = peakForce / 2
    value[14] = UInt8(avgForce & 0xFF)
    value[15] = UInt8((avgForce >> 8) & 0xFF)

    // Work Per Stroke (16-bit LE, Joules)
    var workPerStroke: UInt16 = 0
    if CurrentCadence > 0 && CurrentWatt > 0 {
      let strokeTime = 60.0 / Double(CurrentCadence)
      workPerStroke = UInt16(Double(CurrentWatt) * strokeTime)
    }
    value[16] = UInt8(workPerStroke & 0xFF)
    value[17] = UInt8((workPerStroke >> 8) & 0xFF)

    // Stroke Count (16-bit LE)
    value[18] = UInt8(StrokesCount & 0xFF)
    value[19] = UInt8((StrokesCount >> 8) & 0xFF)

    return value
  }

  func buildPM5AdditionalStrokeData() -> Data {
    // 17 bytes
    var value = Data(count: 17)
    let elapsed = getElapsedCentiseconds()

    // Elapsed time (24-bit LE)
    value[0] = UInt8(elapsed & 0xFF)
    value[1] = UInt8((elapsed >> 8) & 0xFF)
    value[2] = UInt8((elapsed >> 16) & 0xFF)

    // Stroke Power (16-bit LE, watts)
    value[3] = UInt8(CurrentWatt & 0xFF)
    value[4] = UInt8((CurrentWatt >> 8) & 0xFF)

    // Stroke Calories (16-bit LE)
    var strokeCalories: UInt16 = 0
    if CurrentCadence > 0 && StrokesCount > 0 {
      strokeCalories = UInt16(Double(KCal) * 1000.0 / Double(StrokesCount))
    }
    value[5] = UInt8(strokeCalories & 0xFF)
    value[6] = UInt8((strokeCalories >> 8) & 0xFF)

    // Stroke Count (16-bit LE)
    value[7] = UInt8(StrokesCount & 0xFF)
    value[8] = UInt8((StrokesCount >> 8) & 0xFF)

    // Projected Work Time - 0
    value[9] = 0x00
    value[10] = 0x00
    value[11] = 0x00

    // Projected Work Distance - 0
    value[12] = 0x00
    value[13] = 0x00
    value[14] = 0x00

    // Work Per Stroke (16-bit LE, Joules)
    var workPerStroke: UInt16 = 0
    if CurrentCadence > 0 && CurrentWatt > 0 {
      let strokeTime = 60.0 / Double(CurrentCadence)
      workPerStroke = UInt16(Double(CurrentWatt) * strokeTime)
    }
    value[15] = UInt8(workPerStroke & 0xFF)
    value[16] = UInt8((workPerStroke >> 8) & 0xFF)

    return value
  }

} /// class-end
