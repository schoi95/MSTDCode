/*
  Board Selection Required:
  -------------------------
  In Arduino IDE, go to:
  Tools → Board → Seeed nRF52 Boards → "XIAO nRF52840 (No Updates)"
  Seeed Stuido XIAO nRF52840 board package and instructions added from
  the following URL: https://wiki.seeedstudio.com/XIAO_BLE/
*/

// =====================
// VARIABLE DESCRIPTIONS
// =====================
// calibrationFactor      → Adjusts raw HX711 output to pounds
// bleInterval            → Delay between BLE transmissions (ms)
// testDuration           → Duration of test session (ms)
// baselineOffset         → Weight offset from initial tare (lbs)
// lastValidReading       → Most recent force reading (lbs)
// peakForce              → Highest force recorded during test
// testActive             → Indicates if a test session is currently running
// testStartTime          → Timestamp when the test started
// previousMillis         → Tracks last BLE update timestamp
// connectionMillis       → Tracks time when BLE central connected
// blinkTimer             → Timestamp for blinking logic (LED)
// blinkState             → LED state toggle (used for flashing)
// isAdvertising          → Tracks advertising state manually (since BLE.advertising() doesn't exist)

// =====================
// PIN ASSIGNMENTS
// =====================
#include <ArduinoBLE.h>
#include "HX711.h"

#define DOUT 2               // HX711 data pin
#define CLK 3                // HX711 clock pin
#define RED_LED   10         // Red status LED
#define GREEN_LED 11         // Green status LED
#define BLUE_LED  12         // Blue status LED
#define BUTTON_PIN 4         // Jumper-wire trigger input

HX711 scale;

BLEService forceService("180C");  // Custom BLE service
BLECharacteristic forceDataCharacteristic("2A56", BLERead | BLENotify, 50);  // Transmits force readings

// Calibration and timing constants
const float calibrationFactor = -20767.5;  // Convert HX711 value to pounds
const long bleInterval = 500;              // BLE update interval (ms)
const unsigned long testDuration = 10000;  // Test length = 10 seconds

// Runtime variables
float baselineOffset = 0.0;
float lastValidReading = 0.0;
float peakForce = 0.0;
bool testActive = false;

unsigned long testStartTime = 0;
unsigned long previousMillis = 0;
unsigned long connectionMillis = 0;
unsigned long blinkTimer = 0;
bool blinkState = false;
bool isAdvertising = false;

void setLED(bool r, bool g, bool b) {
  digitalWrite(RED_LED, r ? LOW : HIGH);
  digitalWrite(GREEN_LED, g ? LOW : HIGH);
  digitalWrite(BLUE_LED, b ? LOW : HIGH);
}

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Jumper input

  setLED(true, false, false);  // Solid red = not connected

  // Initialize HX711
  scale.begin(DOUT, CLK);
  delay(100);
  while (!scale.is_ready()) {
    delay(100);
  }

  scale.set_scale(calibrationFactor);
  scale.tare();
  delay(1000);
  baselineOffset = scale.get_units(10);  // Baseline tare offset

  // Initialize BLE
  if (!BLE.begin()) {
    setLED(true, false, false); // Red = BLE failed
    while (1);
  }

  BLE.setLocalName("XIAO_nRF52840");
  BLE.setAdvertisedService(forceService);
  forceService.addCharacteristic(forceDataCharacteristic);
  BLE.addService(forceService);

  BLE.advertise();
  isAdvertising = true;

  blinkTimer = millis();
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    connectionMillis = millis();
    previousMillis = 0;
    blinkTimer = millis();
    blinkState = false;
    isAdvertising = false;

    while (central.connected()) {
      unsigned long currentMillis = millis();

      // Jumper wire trigger to start test
      if (!testActive && digitalRead(BUTTON_PIN) == LOW) {
        testActive = true;
        testStartTime = currentMillis;
        peakForce = 0.0;
      }

      // LED Behavior: Flash green during test, solid when idle
      if (testActive) {
        if (currentMillis - blinkTimer >= 500) {
          blinkState = !blinkState;
          blinkTimer = currentMillis;
          setLED(false, blinkState, false);  // Flash green
        }
      } else {
        setLED(false, true, false);  // Solid green = connected
      }

      // BLE transmission
      if (currentMillis - previousMillis >= bleInterval) {
        previousMillis = currentMillis;

        if (scale.is_ready()) {
          float pounds = abs(scale.get_units(5) - baselineOffset);
          lastValidReading = pounds;

          char message[40];
          snprintf(message, sizeof(message), "%lu|%.1f", currentMillis - connectionMillis, pounds);
          forceDataCharacteristic.writeValue((uint8_t*)message, strlen(message));

          if (testActive) {
            if (pounds > peakForce) peakForce = pounds;
            if ((currentMillis - testStartTime) >= testDuration) {
              testActive = false;
            }
          }
        }
      }
    }

    // Connection lost → resume advertising
    BLE.advertise();
    isAdvertising = true;
    blinkTimer = millis();
    blinkState = false;
  }

  // LED behavior when disconnected
  if (!BLE.connected()) {
    unsigned long now = millis();

    if (isAdvertising) {
      if (now - blinkTimer >= 500) {
        blinkState = !blinkState;
        blinkTimer = now;
        setLED(false, false, blinkState);  // Flash blue = advertising
      }
    } else {
      setLED(true, false, false);  // Solid red = idle
    }
  }
}
