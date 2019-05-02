/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/
#include <Arduino.h>
#include <sstream>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define SCAN_TIME  1 // seconds
BLEScan* pBLEScan;

String deviceName;
String deviceAddress;
int16_t deviceRSSI;
uint16_t countDevice;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Found device: %s \n", advertisedDevice.toString().c_str());
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("BLEDevice init...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);

  int count = foundDevices.getCount();
  for (int i = 0; i < count; i++)
  {
    BLEAdvertisedDevice d = foundDevices.getDevice(i);
    if (d.getName() == "Mi Band 3") {
      char deviceBuffer[100];
      deviceName = d.getName().c_str();
      deviceAddress = d.getAddress().toString().c_str();
      deviceRSSI = d.getRSSI();

      if (deviceAddress == "e6:37:63:e7:2f:4b" && deviceRSSI > -70) {
        sprintf(deviceBuffer, "Name: %s| Address: %s| RSSI: %d\n", deviceName.c_str(), deviceAddress.c_str(), deviceRSSI);
        Serial.print(deviceBuffer);
      }
    }
  }

  pBLEScan->clearResults();
}
