/*
 * QZ ProForm Wi-Fi -> BLE / ANT+ FE-C bridge
 *
 * ESP32 side:
 *  - connects to the same ProForm / iFit WebSocket endpoint used by QZ's
 *    proformwifibike driver
 *  - exposes corrected power + cadence as BLE Cycling Power
 *  - exchanges metrics / FE-C target-power commands with the XIAO nRF52840
 *
 * Required Arduino libraries:
 *  - arduinoWebSockets (Links2004)
 *  - ArduinoJson 7.x
 *
 * The Bluetooth classes are provided by the ESP32 Arduino core.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// -----------------------------------------------------------------------------
// User configuration
// -----------------------------------------------------------------------------

static constexpr char WIFI_SSID[] = "YOUR_ESSID";
static constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
static constexpr char PROFORM_BIKE_IP[] = "192.168.1.100";

// Same meaning as QZ Settings -> Watt Gain.
// QZ applies the gain to reported power and compensates ERG requests by dividing
// the requested target by the gain before sending Target Watts to the bike.
static constexpr float WATT_GAIN = 0.85f;

// UART2 pins on a classic ESP32 DevKitC / ESP-WROOM-32.
// ESP32 TX2 (GPIO17) -> XIAO D7/RX
// ESP32 RX2 (GPIO16) <- XIAO D6/TX
static constexpr int UART_RX_PIN = 16;
static constexpr int UART_TX_PIN = 17;
static constexpr uint32_t UART_BAUD = 115200;

// -----------------------------------------------------------------------------
// ProForm protocol constants
// -----------------------------------------------------------------------------

static constexpr uint16_t PROFORM_WS_PORT = 80;
static constexpr char PROFORM_WS_PATH[] = "/control";
static constexpr char WORKOUT_TYPE_ERG[] = "WATTS_GOAL";

static constexpr uint32_t WIFI_RETRY_MS = 5000;
static constexpr uint32_t METRICS_TO_ANT_MS = 250;
static constexpr uint32_t BLE_NOTIFY_MS = 1000;
static constexpr uint32_t TELEMETRY_STALE_MS = 3000;

// -----------------------------------------------------------------------------
// State
// -----------------------------------------------------------------------------

struct BikeMetrics {
  float rawWatts = 0.0f;
  float correctedWatts = 0.0f;
  float cadenceRpm = 0.0f;
  float speedKph = 0.0f;
  float distanceKm = 0.0f;
  float resistance = 0.0f;
  float incline = 0.0f;
  float targetWatts = 0.0f;
  uint32_t lastRxMs = 0;
};

static BikeMetrics metrics;
static WebSocketsClient webSocket;
static HardwareSerial antSerial(2);

static bool webSocketConnected = false;
static bool ergModeSelected = false;
static uint32_t lastWiFiAttemptMs = 0;
static uint32_t lastMetricsForwardMs = 0;
static uint32_t lastBleNotifyMs = 0;
static uint32_t bridgeStartMs = 0;

// BLE Cycling Power service, matching the existing QZ_ESP32 sketch intent.
static BLEServer *bleServer = nullptr;
static BLECharacteristic *cyclingPowerMeasurement = nullptr;
static uint16_t crankRevolutions = 0;
static uint16_t crankEventTime1024 = 0;
static uint32_t lastCrankMs = 0;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

static float jsonNumber(JsonVariantConst value, float fallback = 0.0f) {
  if (value.is<const char *>()) {
    return atof(value.as<const char *>());
  }
  if (value.is<float>()) {
    return value.as<float>();
  }
  if (value.is<double>()) {
    return static_cast<float>(value.as<double>());
  }
  if (value.is<int>()) {
    return static_cast<float>(value.as<int>());
  }
  if (value.is<long>()) {
    return static_cast<float>(value.as<long>());
  }
  return fallback;
}

static bool hasFreshTelemetry() {
  return metrics.lastRxMs != 0 && (millis() - metrics.lastRxMs) <= TELEMETRY_STALE_MS;
}

static void sendProFormSet(const char *key, const String &value) {
  if (!webSocketConnected) {
    Serial.printf("[ProForm] Cannot set %s: WebSocket disconnected\n", key);
    return;
  }

  JsonDocument doc;
  doc["type"] = "set";
  doc["values"][key] = value;

  String payload;
  serializeJson(doc, payload);
  webSocket.sendTXT(payload);

  Serial.printf("[ProForm] TX %s\n", payload.c_str());
}

static void applyErgTarget(float requestedWatts) {
  if (requestedWatts <= 0.0f) {
    return;
  }

  // ANT+ FE-C target-power range is 0..4000 W. Keep requests bounded even if
  // a malformed UART message reaches us.
  requestedWatts = constrain(requestedWatts, 1.0f, 4000.0f);

  if (!ergModeSelected) {
    sendProFormSet("Workout Type", WORKOUT_TYPE_ERG);
    ergModeSelected = true;
  }

  // This is intentionally equivalent to proformwifibike::innerWriteResistance:
  //   bike_target = requestPower / watt_gain
  // with WATT_GAIN=0.85 by default.
  const float bikeTargetWatts = requestedWatts / WATT_GAIN;
  sendProFormSet("Target Watts", String(bikeTargetWatts, 2));

  Serial.printf("[ERG] Garmin %.2f W -> ProForm %.2f W (gain %.3f)\n",
                requestedWatts, bikeTargetWatts, WATT_GAIN);
}

// -----------------------------------------------------------------------------
// UART protocol between ESP32 and XIAO
//
// ESP32 -> XIAO:
//   M,<power_w>,<cadence_rpm>,<speed_mm_s>,<distance_m>,<elapsed_quarter_s>
//
// XIAO -> ESP32:
//   T4,<target_power_quarter_watts>
//
// Integer quarter-watts avoid floating-point formatting on the XIAO.
// -----------------------------------------------------------------------------

static void processAntUartLine(const String &line) {
  unsigned int targetX4 = 0;

  if (sscanf(line.c_str(), "T4,%u", &targetX4) == 1) {
    const float targetWatts = static_cast<float>(targetX4) / 4.0f;
    applyErgTarget(targetWatts);
    return;
  }

  Serial.printf("[ANT UART] Unknown line: %s\n", line.c_str());
}

static void serviceAntUart() {
  static String line;
  while (antSerial.available()) {
    const char c = static_cast<char>(antSerial.read());

    if (c == '\n') {
      line.trim();
      if (!line.isEmpty()) {
        processAntUartLine(line);
      }
      line = "";
    } else if (c != '\r') {
      if (line.length() < 96) {
        line += c;
      } else {
        line = "";
      }
    }
  }
}

static void forwardMetricsToAnt() {
  if (millis() - lastMetricsForwardMs < METRICS_TO_ANT_MS) {
    return;
  }
  lastMetricsForwardMs = millis();

  float power = 0.0f;
  float cadence = 0.0f;
  float speedKph = 0.0f;
  float distanceKm = metrics.distanceKm;

  if (hasFreshTelemetry()) {
    power = metrics.correctedWatts;
    cadence = metrics.cadenceRpm;
    speedKph = metrics.speedKph;
  }

  const uint16_t powerW = static_cast<uint16_t>(constrain(lroundf(power), 0L, 4095L));
  const uint8_t cadenceRpm = static_cast<uint8_t>(constrain(lroundf(cadence), 0L, 254L));
  const uint16_t speedMmPerSec =
      static_cast<uint16_t>(constrain(lroundf((speedKph / 3.6f) * 1000.0f), 0L, 65535L));
  const uint32_t distanceM =
      static_cast<uint32_t>(max(0.0f, distanceKm) * 1000.0f);
  const uint8_t elapsedQuarterSeconds =
      static_cast<uint8_t>(((millis() - bridgeStartMs) / 250UL) & 0xFF);

  antSerial.printf("M,%u,%u,%u,%lu,%u\n",
                   powerW,
                   cadenceRpm,
                   speedMmPerSec,
                   static_cast<unsigned long>(distanceM),
                   elapsedQuarterSeconds);
}

// -----------------------------------------------------------------------------
// ProForm WebSocket
// -----------------------------------------------------------------------------

static void parseProFormTelemetry(const uint8_t *payload, size_t length) {
  JsonDocument doc;
  const DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.printf("[ProForm] JSON error: %s\n", error.c_str());
    return;
  }

  JsonVariantConst valuesVariant = doc["values"];
  if (!valuesVariant.is<JsonObjectConst>()) {
    return;
  }

  JsonObjectConst values = valuesVariant.as<JsonObjectConst>();
  const uint32_t now = millis();

  // Keep the same aliases handled by QZ's proformwifibike::characteristicChanged.
  if (!values["Current KPH"].isNull()) {
    metrics.speedKph = jsonNumber(values["Current KPH"], metrics.speedKph);
  } else if (!values["KPH"].isNull()) {
    metrics.speedKph = jsonNumber(values["KPH"], metrics.speedKph);
  }

  if (!values["Kilometers"].isNull()) {
    metrics.distanceKm = jsonNumber(values["Kilometers"], metrics.distanceKm);
  } else if (!values["Chilometri"].isNull()) {
    metrics.distanceKm = jsonNumber(values["Chilometri"], metrics.distanceKm);
  }

  if (!values["RPM"].isNull()) {
    metrics.cadenceRpm = jsonNumber(values["RPM"], metrics.cadenceRpm);
  }

  bool receivedPower = false;
  if (!values["Current Watts"].isNull()) {
    metrics.rawWatts = jsonNumber(values["Current Watts"], metrics.rawWatts);
    receivedPower = true;
  } else if (!values["Watt attuali"].isNull()) {
    metrics.rawWatts = jsonNumber(values["Watt attuali"], metrics.rawWatts);
    receivedPower = true;
  }

  // QZ Metric(METRIC_WATT) applies watt_gain to positive received power.
  if (receivedPower || metrics.rawWatts > 0.0f) {
    metrics.correctedWatts =
        metrics.cadenceRpm > 0.0f ? metrics.rawWatts * WATT_GAIN : 0.0f;
  }

  if (!values["Actual Incline"].isNull()) {
    metrics.incline = jsonNumber(values["Actual Incline"], metrics.incline);
  } else if (!values["Incline"].isNull()) {
    metrics.incline = jsonNumber(values["Incline"], metrics.incline);
  }

  if (!values["Resistance"].isNull()) {
    metrics.resistance = jsonNumber(values["Resistance"], metrics.resistance);
  }

  if (!values["Target Watts"].isNull()) {
    metrics.targetWatts = jsonNumber(values["Target Watts"], metrics.targetWatts);
  }

  metrics.lastRxMs = now;
}

static void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      webSocketConnected = true;
      ergModeSelected = false;
      Serial.printf("[ProForm] WebSocket connected: ws://%s%s\n",
                    PROFORM_BIKE_IP, PROFORM_WS_PATH);
      break;

    case WStype_DISCONNECTED:
      webSocketConnected = false;
      ergModeSelected = false;
      Serial.println("[ProForm] WebSocket disconnected");
      break;

    case WStype_TEXT:
      parseProFormTelemetry(payload, length);
      break;

    default:
      break;
  }
}

static void startWebSocket() {
  webSocket.begin(PROFORM_BIKE_IP, PROFORM_WS_PORT, PROFORM_WS_PATH);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(3000);
  webSocket.enableHeartbeat(15000, 3000, 2);
}

static void maintainWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  const uint32_t now = millis();
  if (now - lastWiFiAttemptMs < WIFI_RETRY_MS) {
    return;
  }

  lastWiFiAttemptMs = now;
  Serial.printf("[WiFi] Connecting to %s\n", WIFI_SSID);
  WiFi.disconnect();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// -----------------------------------------------------------------------------
// BLE Cycling Power
// -----------------------------------------------------------------------------

class QzBleServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *server) override {
    (void)server;
    Serial.println("[BLE] Client connected");
  }

  void onDisconnect(BLEServer *server) override {
    (void)server;
    Serial.println("[BLE] Client disconnected");
    BLEDevice::startAdvertising();
  }
};

static void setupBleCyclingPower() {
  BLEDevice::init("QZ-ProForm-Bridge");

  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new QzBleServerCallbacks());

  BLEService *service = bleServer->createService(BLEUUID((uint16_t)0x1818));

  BLECharacteristic *feature = service->createCharacteristic(
      BLEUUID((uint16_t)0x2A65), BLECharacteristic::PROPERTY_READ);
  BLECharacteristic *sensorLocation = service->createCharacteristic(
      BLEUUID((uint16_t)0x2A5D), BLECharacteristic::PROPERTY_READ);
  cyclingPowerMeasurement = service->createCharacteristic(
      BLEUUID((uint16_t)0x2A63),
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  // Cycling Power Feature: crank revolution data supported.
  const uint32_t featureBits = (1UL << 3);
  feature->setValue(reinterpret_cast<const uint8_t *>(&featureBits), sizeof(featureBits));

  // Sensor location: rear hub (0x0D), matching the previous QZ_ESP32 sketch.
  const uint8_t location = 0x0D;
  sensorLocation->setValue(&location, 1);

  cyclingPowerMeasurement->addDescriptor(new BLE2902());

  service->start();

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLEUUID((uint16_t)0x1818));
  advertising->setScanResponse(true);
  advertising->start();

  Serial.println("[BLE] Cycling Power advertising as QZ-ProForm-Bridge");
}

static void updateCrankData(float cadenceRpm) {
  if (cadenceRpm <= 0.0f) {
    return;
  }

  const uint32_t revolutionPeriodMs =
      static_cast<uint32_t>(60000.0f / cadenceRpm);

  if (lastCrankMs == 0) {
    lastCrankMs = millis();
    return;
  }

  while ((millis() - lastCrankMs) >= revolutionPeriodMs) {
    lastCrankMs += revolutionPeriodMs;
    crankRevolutions++;
    crankEventTime1024 =
        static_cast<uint16_t>(((static_cast<uint64_t>(lastCrankMs) * 1024ULL) / 1000ULL) & 0xFFFF);
  }
}

static void notifyBleCyclingPower() {
  if (cyclingPowerMeasurement == nullptr ||
      millis() - lastBleNotifyMs < BLE_NOTIFY_MS) {
    return;
  }
  lastBleNotifyMs = millis();

  const float cadence = hasFreshTelemetry() ? metrics.cadenceRpm : 0.0f;
  const float power = hasFreshTelemetry() ? metrics.correctedWatts : 0.0f;

  updateCrankData(cadence);

  // Cycling Power Measurement:
  // bit 5 -> crank revolution data present
  const uint16_t flags = 0x0020;
  const int16_t powerW =
      static_cast<int16_t>(constrain(lroundf(power), -32768L, 32767L));

  uint8_t packet[8];
  packet[0] = static_cast<uint8_t>(flags & 0xFF);
  packet[1] = static_cast<uint8_t>((flags >> 8) & 0xFF);
  packet[2] = static_cast<uint8_t>(powerW & 0xFF);
  packet[3] = static_cast<uint8_t>((powerW >> 8) & 0xFF);
  packet[4] = static_cast<uint8_t>(crankRevolutions & 0xFF);
  packet[5] = static_cast<uint8_t>((crankRevolutions >> 8) & 0xFF);
  packet[6] = static_cast<uint8_t>(crankEventTime1024 & 0xFF);
  packet[7] = static_cast<uint8_t>((crankEventTime1024 >> 8) & 0xFF);

  cyclingPowerMeasurement->setValue(packet, sizeof(packet));
  cyclingPowerMeasurement->notify();
}

// -----------------------------------------------------------------------------
// Arduino entry points
// -----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(300);

  bridgeStartMs = millis();

  antSerial.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lastWiFiAttemptMs = millis();

  setupBleCyclingPower();
  startWebSocket();

  Serial.println("QZ ProForm Wi-Fi / ANT+ FE-C bridge started");
}

void loop() {
  maintainWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    webSocket.loop();
  }

  serviceAntUart();
  forwardMetricsToAnt();
  notifyBleCyclingPower();

  delay(1);
}
