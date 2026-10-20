/**
 * FTMS treadmill -> Running Speed and Cadence bridge.
 *
 * The ESP32 scans for a Fitness Machine Service (0x1826), connects to the
 * Treadmill Data characteristic (0x2ACD), and republishes speed plus a
 * calculated running cadence through the standard RSC service (0x1814).
 * This lets a Garmin watch use the ESP32 as a running cadence sensor.
 *
 * NimBLE-Arduino 2.x is used for simultaneous client and server operation.
 */

#include <Arduino.h>
#include <NimBLEDevice.h>

static const NimBLEUUID FTMS_SERVICE_UUID("1826");
static const NimBLEUUID FTMS_TREADMILL_DATA_UUID("2ACD");

static const NimBLEUUID RSC_SERVICE_UUID("1814");
static const NimBLEUUID RSC_FEATURE_UUID("2A54");
static const NimBLEUUID RSC_MEASUREMENT_UUID("2A53");
static const NimBLEUUID SENSOR_LOCATION_UUID("2A5D");
static const NimBLEUUID RSC_CONTROL_POINT_UUID("2A55");

// Keep these values aligned with QZ's speed-to-cadence calculation.
// Change them before flashing if the bridge is used by another person.
static constexpr float USER_HEIGHT_CM = 170.0f;
static constexpr bool USER_IS_MALE = true;

static constexpr uint32_t SCAN_TIME_MS = 5000;
static constexpr uint32_t SAMPLE_TIMEOUT_MS = 3000;
static constexpr uint32_t RSC_NOTIFY_INTERVAL_MS = 500;

static NimBLEServer *rscServer = nullptr;
static NimBLECharacteristic *rscMeasurement = nullptr;
static const NimBLEAdvertisedDevice *foundTreadmill = nullptr;
static NimBLEClient *ftmsClient = nullptr;
static NimBLERemoteCharacteristic *ftmsData = nullptr;

static bool connectRequested = false;
static bool ftmsConnected = false;
static float speedKph = 0.0f;
static uint32_t lastSpeedSampleMs = 0;
static uint32_t lastDistanceUpdateMs = 0;
static uint32_t lastRscNotifyMs = 0;
static uint32_t lastRscLogMs = 0;
static float distanceMeters = 0.0f;

static uint16_t readUint16LE(const uint8_t *data) {
    return static_cast<uint16_t>(data[0]) |
           (static_cast<uint16_t>(data[1]) << 8);
}

static float strideLengthMeters() {
    const float heightMeters = USER_HEIGHT_CM / 100.0f;
    return heightMeters * (USER_IS_MALE ? 0.415f : 0.413f);
}

static uint8_t cadenceFromSpeed(float speedKph) {
    if (speedKph < 0.1f) {
        return 0;
    }

    const float speedMetersPerSecond = speedKph / 3.6f;
    const float cadence = (speedMetersPerSecond / strideLengthMeters()) * 60.0f;
    return static_cast<uint8_t>(constrain(lroundf(cadence), 0L, 255L));
}

static void parseTreadmillData(const uint8_t *data, size_t length) {
    if (length < 2) {
        return;
    }

    const uint16_t flags = readUint16LE(data);
    const bool moreData = (flags & 0x0001) != 0;

    // With More Data set, instantaneous speed is not present in the packet.
    if (moreData || length < 4) {
        return;
    }

    // FTMS treadmill speed is uint16, resolution 0.01 km/h.
    speedKph = static_cast<float>(readUint16LE(data + 2)) / 100.0f;
    lastSpeedSampleMs = millis();

    Serial.printf("FTMS speed: %.2f km/h, RSC cadence: %u spm\n",
                  speedKph, cadenceFromSpeed(speedKph));
}

static void treadmillNotificationCallback(NimBLERemoteCharacteristic *,
                                           uint8_t *data,
                                           size_t length,
                                           bool) {
    parseTreadmillData(data, length);
}

class RscServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override {
        Serial.printf("Garmin connected: %s\n", connInfo.getAddress().toString().c_str());
        server->updateConnParams(connInfo.getConnHandle(), 12, 24, 0, 180);
    }

    void onDisconnect(NimBLEServer *, NimBLEConnInfo &, int) override {
        Serial.println("Garmin disconnected; restarting RSC advertising");
        NimBLEDevice::startAdvertising();
    }
};

class RscMeasurementCallbacks : public NimBLECharacteristicCallbacks {
    void onSubscribe(NimBLECharacteristic *, NimBLEConnInfo &connInfo, uint16_t subValue) override {
        Serial.printf("RSC measurement subscription from %s: 0x%04X\n",
                      connInfo.getAddress().toString().c_str(), subValue);
    }
};

class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice *device) override {
        if (!device->isAdvertisingService(FTMS_SERVICE_UUID)) {
            return;
        }

        Serial.printf("FTMS treadmill found: %s\n", device->toString().c_str());
        NimBLEDevice::getScan()->stop();
        foundTreadmill = device;
        connectRequested = true;
    }

    void onScanEnd(const NimBLEScanResults &, int reason) override {
        if (!ftmsConnected && !connectRequested) {
            Serial.printf("FTMS scan ended (%d); scanning again\n", reason);
            NimBLEDevice::getScan()->start(SCAN_TIME_MS, false, true);
        }
    }
};

class FtmsClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient *) override {
        Serial.println("Connected to FTMS treadmill");
    }

    void onDisconnect(NimBLEClient *, int reason) override {
        ftmsConnected = false;
        ftmsData = nullptr;
        speedKph = 0.0f;
        Serial.printf("FTMS treadmill disconnected (%d); scanning again\n", reason);
        NimBLEDevice::getScan()->start(SCAN_TIME_MS, false, true);
    }
};

static ScanCallbacks scanCallbacks;
static FtmsClientCallbacks ftmsClientCallbacks;
static RscMeasurementCallbacks rscMeasurementCallbacks;

static bool connectToTreadmill() {
    if (foundTreadmill == nullptr) {
        return false;
    }

    NimBLEClient *client = NimBLEDevice::getClientByPeerAddress(foundTreadmill->getAddress());
    if (client == nullptr) {
        client = NimBLEDevice::getDisconnectedClient();
    }
    if (client == nullptr) {
        client = NimBLEDevice::createClient();
        client->setClientCallbacks(&ftmsClientCallbacks, false);
        client->setConnectionParams(12, 24, 0, 180);
        client->setConnectTimeout(10000);
    }

    if (!client->connect(foundTreadmill)) {
        Serial.println("Could not connect to FTMS treadmill");
        return false;
    }

    NimBLERemoteService *service = client->getService(FTMS_SERVICE_UUID);
    if (service == nullptr) {
        Serial.println("FTMS service 0x1826 not found after connecting");
        client->disconnect();
        return false;
    }

    NimBLERemoteCharacteristic *dataCharacteristic =
        service->getCharacteristic(FTMS_TREADMILL_DATA_UUID);
    if (dataCharacteristic == nullptr || !dataCharacteristic->canNotify()) {
        Serial.println("FTMS treadmill data characteristic 0x2ACD is unavailable");
        client->disconnect();
        return false;
    }

    if (!dataCharacteristic->subscribe(true, treadmillNotificationCallback)) {
        Serial.println("Could not subscribe to FTMS treadmill data");
        client->disconnect();
        return false;
    }

    ftmsClient = client;
    ftmsData = dataCharacteristic;
    ftmsConnected = true;
    lastSpeedSampleMs = millis();
    Serial.println("Subscribed to FTMS treadmill data");
    return true;
}

static void startRscServer() {
    rscServer = NimBLEDevice::createServer();
    rscServer->setCallbacks(new RscServerCallbacks());

    NimBLEService *service = rscServer->createService(RSC_SERVICE_UUID);

    NimBLECharacteristic *feature = service->createCharacteristic(
        RSC_FEATURE_UUID, NIMBLE_PROPERTY::READ);
    const uint8_t featureValue[] = {0x02, 0x00}; // Total Distance supported.
    feature->setValue(featureValue, sizeof(featureValue));

    NimBLECharacteristic *sensorLocation = service->createCharacteristic(
        SENSOR_LOCATION_UUID, NIMBLE_PROPERTY::READ);
    const uint8_t sensorLocationValue[] = {0x01}; // Top of shoe.
    sensorLocation->setValue(sensorLocationValue, sizeof(sensorLocationValue));

    rscMeasurement = service->createCharacteristic(
        RSC_MEASUREMENT_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    rscMeasurement->setCallbacks(&rscMeasurementCallbacks);
    const uint8_t emptyMeasurement[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    rscMeasurement->setValue(emptyMeasurement, sizeof(emptyMeasurement));

    service->createCharacteristic(
        RSC_CONTROL_POINT_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE);

    service->start();

    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    advertising->enableScanResponse(true);
    advertising->setName("QZ FTMS RSC");
    // Bluetooth SIG appearance 0x0440: Generic Running Walking Sensor.
    advertising->setAppearance(0x0440);
    advertising->addServiceUUID(RSC_SERVICE_UUID);
    const bool advertisingStarted = advertising->start();
    Serial.printf("RSC server advertising as QZ FTMS RSC: %s\n",
                  advertisingStarted ? "started" : "failed");
}

static void updateDistance() {
    const uint32_t now = millis();
    if (lastDistanceUpdateMs == 0) {
        lastDistanceUpdateMs = now;
        return;
    }

    const uint32_t elapsedMs = now - lastDistanceUpdateMs;
    lastDistanceUpdateMs = now;
    if (elapsedMs > 1000 || now - lastSpeedSampleMs > SAMPLE_TIMEOUT_MS) {
        return;
    }

    distanceMeters += (speedKph / 3.6f) * (static_cast<float>(elapsedMs) / 1000.0f);
}

static void notifyRsc() {
    if (rscServer->getConnectedCount() == 0) {
        return;
    }

    const uint32_t now = millis();
    if (now - lastRscNotifyMs < RSC_NOTIFY_INTERVAL_MS) {
        return;
    }
    lastRscNotifyMs = now;

    const float currentSpeed = now - lastSpeedSampleMs <= SAMPLE_TIMEOUT_MS ? speedKph : 0.0f;
    const uint16_t speedRaw = static_cast<uint16_t>(constrain(
        lroundf((currentSpeed / 3.6f) * 256.0f), 0L, 65535L));
    const uint8_t cadence = cadenceFromSpeed(currentSpeed);
    const uint32_t distanceDecimeters = static_cast<uint32_t>(lroundf(distanceMeters * 10.0f));

    uint8_t measurement[] = {
        0x02, // Total Distance present.
        static_cast<uint8_t>(speedRaw & 0xFF),
        static_cast<uint8_t>((speedRaw >> 8) & 0xFF),
        cadence,
        static_cast<uint8_t>(distanceDecimeters & 0xFF),
        static_cast<uint8_t>((distanceDecimeters >> 8) & 0xFF),
        static_cast<uint8_t>((distanceDecimeters >> 16) & 0xFF),
        static_cast<uint8_t>((distanceDecimeters >> 24) & 0xFF),
    };

    rscMeasurement->setValue(measurement, sizeof(measurement));
    const bool sent = rscMeasurement->notify();
    if (now - lastRscLogMs >= 1000) {
        Serial.printf("RSC packet: %.2f km/h, raw speed=%u, cadence=%u, distance=%lu dm, notify=%s\n",
                      currentSpeed, speedRaw, cadence, static_cast<unsigned long>(distanceDecimeters),
                      sent ? "ok" : "failed");
        lastRscLogMs = now;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting QZ FTMS treadmill to RSC bridge");
    Serial.printf("Configured stride length: %.2f m\n", strideLengthMeters());

    NimBLEDevice::init("QZ FTMS RSC");
    NimBLEDevice::setPower(3);
    startRscServer();

    NimBLEScan *scan = NimBLEDevice::getScan();
    scan->setScanCallbacks(&scanCallbacks, false);
    scan->setInterval(100);
    scan->setWindow(100);
    scan->setActiveScan(true);
    scan->start(SCAN_TIME_MS, false, true);
    Serial.println("Scanning for a treadmill advertising FTMS 0x1826");
}

void loop() {
    if (connectRequested) {
        connectRequested = false;
        if (!connectToTreadmill()) {
            NimBLEDevice::getScan()->start(SCAN_TIME_MS, false, true);
        }
    }

    updateDistance();
    notifyRsc();
    delay(10);
}
