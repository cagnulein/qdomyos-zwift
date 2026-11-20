#include <Arduino.h>
#include <NimBLEDevice.h>
#include <esp_mac.h>

// 0 = strict_trace, 1 = trace_plus_name, 2 = trace_plus_tuned_interval
#define IFIT_ADV_PROFILE 1
#define IFIT_USE_STATIC_RANDOM_ADDR 1

namespace {
const char *kLogTag = "ifit_adv_only";
const NimBLEUUID kDummyServiceUuid("00001533-1412-efde-1523-785feabcd123");
const NimBLEUUID kDummyCharUuid("00001535-1412-efde-1523-785feabcd123");
const char *kStaticRandomAddress = "E5:BD:DD:54:A4:2A";
const char *kGapDeviceName = "I_EB";

const uint8_t kIfitServiceData[] = {
    0x04, 0x90, 0x02, 0x69, 0xde, 0x06, 0x5f, 0xca,
    0x60, 0x39, 0x89, 0xfc, 0x27, 0xb1, 0xce, 0x6d,
    0x3f, 0xfa, 0x24, 0x2f, 0x03, 0xe4
};

void logBtMac() {
  uint8_t mac[6] = {0};
  esp_read_mac(mac, ESP_MAC_BT);
  Serial.printf("[%s] BT MAC %02X:%02X:%02X:%02X:%02X:%02X\n",
                kLogTag, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void logStaticRandomAddress() {
  Serial.printf("[%s] static random addr %s\n", kLogTag, kStaticRandomAddress);
}

const char *profileName() {
#if IFIT_ADV_PROFILE == 0
  return "strict_trace";
#elif IFIT_ADV_PROFILE == 1
  return "trace_plus_name";
#else
  return "trace_plus_tuned_interval";
#endif
}
}  // namespace

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *server, NimBLEConnInfo &connInfo) override {
    Serial.printf("[%s] client connected: %s\n",
                  kLogTag, connInfo.getAddress().toString().c_str());
    server->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 60);
  }

  void onDisconnect(NimBLEServer *server, NimBLEConnInfo &connInfo, int reason) override {
    Serial.printf("[%s] client disconnected: %s reason=%d\n",
                  kLogTag, connInfo.getAddress().toString().c_str(), reason);
    NimBLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.printf("[%s] boot\n", kLogTag);
  Serial.printf("[%s] adv profile %s\n", kLogTag, profileName());
  logBtMac();
  logStaticRandomAddress();

  NimBLEDevice::init(kGapDeviceName);
#if IFIT_USE_STATIC_RANDOM_ADDR
  bool addrTypeOk = NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM);
  bool addrOk = NimBLEDevice::setOwnAddr(NimBLEAddress(std::string(kStaticRandomAddress), BLE_OWN_ADDR_RANDOM));
  Serial.printf("[%s] setOwnAddrType(random)=%s\n", kLogTag, addrTypeOk ? "true" : "false");
  Serial.printf("[%s] setOwnAddr(%s)=%s\n", kLogTag, kStaticRandomAddress, addrOk ? "true" : "false");
#endif

  Serial.printf("[%s] nimble addr %s\n", kLogTag, NimBLEDevice::getAddress().toString().c_str());

  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  NimBLEService *service = server->createService(kDummyServiceUuid);
  NimBLECharacteristic *characteristic = service->createCharacteristic(
      kDummyCharUuid, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  characteristic->setValue("");
  service->start();

  NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData advData;
  std::string serviceData(reinterpret_cast<const char *>(kIfitServiceData), sizeof(kIfitServiceData));

  advData.setFlags(0x06);
  advData.setServiceData(NimBLEUUID((uint16_t)0xFCF1), serviceData);

  advertising->setAdvertisementData(advData);
#if IFIT_ADV_PROFILE == 1
  NimBLEAdvertisementData scanResponse;
  scanResponse.setName(kGapDeviceName);
  advertising->setScanResponseData(scanResponse);
  advertising->enableScanResponse(true);
#else
  advertising->enableScanResponse(false);
#endif

#if IFIT_ADV_PROFILE == 2
  advertising->setMinInterval(0x00a0);
  advertising->setMaxInterval(0x00a0);
#endif

  bool started = advertising->start();

  Serial.printf("[%s] advertising start result=%s\n", kLogTag, started ? "true" : "false");
  Serial.printf("[%s] service data: ", kLogTag);
  for (size_t i = 0; i < sizeof(kIfitServiceData); ++i) {
    Serial.printf("%02x", kIfitServiceData[i]);
  }
  Serial.println();
}

void loop() {
  delay(1000);
}
