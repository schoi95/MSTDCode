#include <ArduinoBLE.h>
#include "HX711.h"

#define DOUT 2
#define CLK 3
HX711 scale;

// BLE setup
BLEService forceService("180C");
BLECharacteristic forceDataCharacteristic("2A56", BLERead | BLENotify, 50);

// Calibration settings
const float calibrationFactor = -20767.5;
const long bleInterval = 500; // BLE send interval (ms)
const unsigned long testDuration = 10000; // 10 seconds

// Runtime vars
float baselineOffset = 0.0;
float lastValidReading = 0.0;
float peakForce = 0.0;
bool testActive = false;

unsigned long testStartTime = 0;
unsigned long previousMillis = 0;
unsigned long connectionMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  scale.begin(DOUT, CLK);
  Serial.println("Initializing HX711...");
  while (!scale.is_ready()) {
    Serial.println("Waiting for HX711...");
    delay(100);
  }

  scale.set_scale(calibrationFactor);
  Serial.println("Taring scale...");
  scale.tare();
  delay(1000);

  baselineOffset = scale.get_units(10);
  Serial.print("Software tare baseline: ");
  Serial.println(baselineOffset, 4);

  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1);
  }

  BLE.setLocalName("XIAO_nRF52840");
  BLE.setAdvertisedService(forceService);
  forceService.addCharacteristic(forceDataCharacteristic);
  BLE.addService(forceService);
  BLE.advertise();

  Serial.println("BLE advertising started...");
  Serial.println("Press 'S' in Serial Monitor to start a 10-second test.");
}

void loop() {
  BLEDevice central = BLE.central();

  if (Serial.available()) {
    char input = Serial.read();
    if ((input == 'S' || input == 's') && !testActive) {
      Serial.println("Starting 10-second test session...");
      peakForce = 0.0;
      testStartTime = millis();
      testActive = true;
    }
  }

  if (central) {
    if (connectionMillis == 0) {
      Serial.print("Connected to central: ");
      Serial.println(central.address());
      connectionMillis = millis();
      previousMillis = 0;
    }

    while (central.connected()) {
      unsigned long currentMillis = millis();

      // BLE transmission
      if (currentMillis - previousMillis >= bleInterval) {
        previousMillis = currentMillis;

        if (scale.is_ready()) {
          float pounds = scale.get_units(5) - baselineOffset;
          pounds = abs(pounds);
          lastValidReading = pounds;

          char message[40];
          snprintf(message, sizeof(message), "%lu|%.1f", currentMillis - connectionMillis, pounds);
          forceDataCharacteristic.writeValue((uint8_t*)message, strlen(message));

          Serial.print("Sent: ");
          Serial.println(message);

          // Track peak force during test
          if (testActive) {
            if (pounds > peakForce) {
              peakForce = pounds;
            }

            if ((currentMillis - testStartTime) >= testDuration) {
              testActive = false;
              Serial.println("Test session complete.");
              Serial.print("Peak Force Recorded: ");
              Serial.print(peakForce, 1);
              Serial.println(" lbs\n");
              Serial.println("Press 'S' to start another test.");
            }
          }
        }
      }
    }

    Serial.println("Disconnected from central.");
    connectionMillis = 0;
    lastValidReading = 0;
    testActive = false;
  }
}
