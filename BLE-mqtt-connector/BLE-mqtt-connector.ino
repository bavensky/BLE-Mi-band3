#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <ArduinoJson.h>
#include <MqttConnector.h>
#include <Wire.h>
#include <SPI.h>

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

#include "init_mqtt.h"
#include "_publish.h"
#include "_receive.h"
#include "_config.h"

MqttConnector *mqtt;

int relayPin = 15;
int relayPinState = HIGH;
int LED_PIN = 21;
boolean stateSend = false;

char myName[40];

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //      Serial.printf("Found device: %s \n", advertisedDevice.toString().c_str());
    }
};

void init_hardware()
{
  pinMode(relayPin, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(relayPin, relayPinState);;
  // serial port initialization
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void init_wifi() {
  WiFi.disconnect();
  delay(20);
  WiFi.mode(WIFI_STA);
  delay(50);
  const char* ssid =  WIFI_SSID.c_str();
  const char* pass =  WIFI_PASSWORD.c_str();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf ("Connecting to %s:%s\r\n", ssid, pass);
    delay(300);
  }
  Serial.println("WiFi Connected.");
}

void setup()
{
  init_hardware();
  init_wifi();
  init_mqtt();
}

void loop()
{
  mqtt->loop();

  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
  int count = foundDevices.getCount();
  for (int i = 0; i < count; i++)
  {
    BLEAdvertisedDevice d = foundDevices.getDevice(i);
    if (d.getName() == "Mi Band 3") {
      digitalWrite(LED_PIN, LOW);
      char deviceBuffer[100];
      deviceName = d.getName().c_str();
      deviceAddress = d.getAddress().toString().c_str();
      deviceRSSI = d.getRSSI();

      if (deviceAddress == "e6:37:63:e7:2f:4b" && deviceRSSI > -60) {
        digitalWrite(LED_PIN, HIGH);
        sprintf(deviceBuffer, "Name: %s| Address: %s| RSSI: %d\n", deviceName.c_str(), deviceAddress.c_str(), deviceRSSI);
        Serial.print(deviceBuffer);
        mqtt->sync_advpub("", "CMMC/CMMCFrontDoor/$/command", "OPEN", false);
      }
    }
  }
  pBLEScan->clearResults();
}
