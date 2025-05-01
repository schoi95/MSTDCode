#include <ArduinoBLE.h>
#include "HX711.h"

// HX711 wiring
#define DOUT 2
#define CLK 3
HX711 scale;

const float calibrationFactor = 10000.0;
long baselineOffset = 0;  // New: store the idle reading

BLEService forceService("180C");
BLECharacteristic forceDataCharacteristic("2A56", BLERead | BLENotify, 50);

unsigned long connectionMillis = 0;
unsigned long previousMillis = 0;
const long interval = 500; // 0.5 sec

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Start BLE
  if (!BLE.begin()) {
    Serial.println("BLE failed to start!");
    while (1);
  }

  // Start HX711
  scale.begin(DOUT, CLK);
  Serial.println("Initializing HX711...");

  while (!scale.is_ready()) {
    Serial.println("Waiting for HX711...");
    delay(100);
  }

  scale.tare(); // First zeroing
  delay(500);   // Wait a bit for stable reading

  // NEW: Manually capture the baseline reading after tare
  baselineOffset = scale.read();
  Serial.print("Baseline offset: ");
  Serial.println(baselineOffset);

  // BLE setup
  BLE.setLocalName("XIAO_nRF52840");
  BLE.setAdvertisedService(forceService);
  forceService.addCharacteristic(forceDataCharacteristic);
  BLE.addService(forceService);
  BLE.advertise();

  Serial.println("Peripheral advertising started...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    connectionMillis = millis();
    previousMillis = 0;

    while (central.connected()) {
      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        if (scale.is_ready()) {
          long raw = scale.read();
          long adjusted = raw - baselineOffset;
          float pounds = -1.0 * ((float)adjusted / calibrationFactor);

          String message = String(currentMillis - connectionMillis) + "|" + String(pounds, 2);
          forceDataCharacteristic.writeValue(message.c_str());

          Serial.print("Sent: ");
          Serial.println(message);
        } else {
          Serial.println("Scale not ready.");
        }
      }
    }

    Serial.println("Disconnected from central.");
    connectionMillis = 0;
  }
}
