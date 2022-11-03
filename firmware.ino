/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
   Has a characteristic of: 6E400002-B5A3-F393-E0A9-E50E24DCCA9E - used for receiving data with "WRITE" 
   Has a characteristic of: 6E400003-B5A3-F393-E0A9-E50E24DCCA9E - used to send data with  "NOTIFY"

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   In this example rxValue is the data received (only accessible inside that function).
   And txValue is the data to be sent, in this example just a byte incremented every second. 
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <BH1750.h>
#include "DHT.h"

BH1750 lightMeter(0x23); /* 0x23 */
DHT dht(16, DHT11);

BLEServer* pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID                     "9A1A0000-76E2-4C05-AA14-81629336ACB8" 
#define CHARACTERISTIC_UUID_ILLUMINATION "9A1A0001-76E2-4C05-AA14-81629336ACB8"
#define CHARACTERISTIC_UUID_HUMIDITY     "9A1A0002-76E2-4C05-AA14-81629336ACB8"
#define CHARACTERISTIC_UUID_TEMPERATURE  "9A1A0003-76E2-4C05-AA14-81629336ACB8"
#define CHARACTERISTIC_UUID_VOLTAGE      "9A1A0004-76E2-4C05-AA14-81629336ACB8"
#define CHARACTERISTIC_UUID_SOILHUMIDITY "9A1A0005-76E2-4C05-AA14-81629336ACB8"
#define CHARACTERISTIC_UUID_SOILSALT     "9A1A0006-76E2-4C05-AA14-81629336ACB8"

BLECharacteristic* characteristicIllumination = nullptr;
BLECharacteristic* characteristicHumidity     = nullptr;
BLECharacteristic* characteristicTemperature  = nullptr;
BLECharacteristic* characteristicVoltage      = nullptr;
BLECharacteristic* characteristicSoilHumidity = nullptr;
BLECharacteristic* characteristicSoilSalt     = nullptr;

const int POWER_CTRL = 4;


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

BLECharacteristic* addTxCharacteristic(char* uuid) {
  BLEService* pService = pServer->createService(uuid);
  Serial.print("Add characteristic with uuid ");
  Serial.println(uuid);
  BLECharacteristic* pTxCharacteristic = pService->createCharacteristic(uuid, BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  delay(50);
  return pTxCharacteristic;
}

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("Gaia Plant Sensor");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Add Characteristics
  characteristicIllumination = addTxCharacteristic(CHARACTERISTIC_UUID_ILLUMINATION);
  characteristicHumidity     = addTxCharacteristic(CHARACTERISTIC_UUID_HUMIDITY    );
  characteristicTemperature  = addTxCharacteristic(CHARACTERISTIC_UUID_TEMPERATURE );
  characteristicVoltage      = addTxCharacteristic(CHARACTERISTIC_UUID_VOLTAGE     );
  characteristicSoilHumidity = addTxCharacteristic(CHARACTERISTIC_UUID_SOILHUMIDITY);
  characteristicSoilSalt     = addTxCharacteristic(CHARACTERISTIC_UUID_SOILSALT    );

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  pinMode(POWER_CTRL, OUTPUT);
  digitalWrite(POWER_CTRL, 1);

  pinMode(32, INPUT);
  pinMode(33, INPUT);
  pinMode(34, INPUT);
  delay(1000);

  Wire.begin(25, 26);

  if (!lightMeter.begin()) {
    Serial.println("Could not find a valid BH1750 sensor, check wiring!");
  }

  dht.begin();
  
}

void sendValueVoid(BLECharacteristic* characteristic, void* data, int size) {
  uint8_t buffer[size] = { 0 };
  memcpy(buffer, data, size);
  characteristic->setValue(buffer, size);
  characteristic->notify();
  delay(10);
}

template<typename T>
void sendValue(BLECharacteristic* characteristic, T data) {
  sendValueVoid(characteristic, (void*) &data, sizeof(T));
}

void loop() {
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    Serial.println("on connect");
  }

  if(!deviceConnected) return;
  Serial.println("send values");

  double illumination = lightMeter.readLightLevel();
  sendValue(characteristicIllumination, illumination);

  double humidity = dht.readHumidity();
  sendValue(characteristicHumidity, humidity);

  double temperature = dht.readTemperature();
  sendValue(characteristicTemperature, temperature);

  double soilHumidity = (int)analogRead(32);
  sendValue(characteristicSoilHumidity, soilHumidity);

  uint16_t volt = analogRead(33);
  double voltage = ((float)volt / 4095.0) * 6.6 * 1100;
  sendValue(characteristicVoltage, voltage);

  // SOIL Minerals level
  uint8_t samples = 120;
  uint32_t humi = 0;
  uint16_t array[120];
  for (int i = 0; i < samples; i++) {
      array[i] = analogRead(34);
      delay(2);
  }
  std::sort(array, array + samples);
  for (int i = 1; i < samples - 1; i++) {
      humi += array[i];
  }
  humi /= samples - 2;
  uint8_t saltperc = humi;
  sendValue(characteristicSoilSalt, (int64_t)saltperc);

  // Replace with deep sleep
  sleep(10);
}