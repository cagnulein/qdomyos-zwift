#include <NimBLEDevice.h>

#define INDOOR_BIKE_DATA_UUID   "00002AD2-0000-1000-8000-00805f9b34fb"
#define CUSTOM_SERVICE_UUID     "ce060000-43e5-11e4-916c-0800200c9a66"

NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pIndoorBikeDataChar = nullptr;

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        Serial.println("Client connected");
    };

    void onDisconnect(NimBLEServer* pServer) {
        Serial.println("Client disconnected");
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting NimBLE Server");

  NimBLEDevice::init("PM5 431431183 Row");

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* pFtmService = pServer->createService("1826");
  //NimBLEService* pCustomService = pServer->createService(CUSTOM_SERVICE_UUID);

  pIndoorBikeDataChar = pFtmService->createCharacteristic(
                          INDOOR_BIKE_DATA_UUID,
                          NIMBLE_PROPERTY::READ |
                          NIMBLE_PROPERTY::NOTIFY
                        );

  pFtmService->start();
  //pCustomService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(pFtmService->getUUID());
  //pAdvertising->addServiceUUID(CUSTOM_SERVICE_UUID);
  const std::string data = { 0x01, 0x10, 0x00 }; // Imposta i valori desiderati
  pAdvertising->setServiceData(pFtmService->getUUID(), data);
  pAdvertising->start();

  Serial.println("Advertising started");
}

void loop() {
  // Metti qui il tuo codice principale, da eseguire ripetutamente
  // Ad esempio, potresti aggiornare il valore della caratteristica Indoor Bike Data
}