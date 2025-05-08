#include <ArduinoBLE.h>
#include "HX711.h"
#include "nrf_nvmc.h"  // For flash memory on Seeed nRF52840

// HX711 pin configuration
#define DOUT 2
#define CLK 3
HX711 scale;

// BLE service and characteristic
BLEService forceService("180C");
BLECharacteristic forceDataCharacteristic("2A56", BLERead | BLENotify, 50);

// Flash memory location for calibration
#define FLASH_CALIB_ADDR 0x7F000
float calibrationFactor = 1.0;
float offset = 0.0;
float lastValidReading = 0.0;

// BLE timing
unsigned long connectionMillis = 0;
unsigned long previousMillis = 0;
const long interval = 500;  // Send interval in milliseconds

// ===== Flash Helpers =====
bool calibrationExists() {
  float value = *(float*)FLASH_CALIB_ADDR;
  return isfinite(value) && value > 0.0 && value < 100000.0;
}

float readCalibration() {
  return *(float*)FLASH_CALIB_ADDR;
}

void saveCalibration(float value) {
  uint32_t* addr = (uint32_t*)FLASH_CALIB_ADDR;

  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos;
  while (!NRF_NVMC->READY);
  NRF_NVMC->ERASEPAGE = (uint32_t)addr;
  while (!NRF_NVMC->READY);

  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
  while (!NRF_NVMC->READY);
  nrf_nvmc_write_words((uint32_t)FLASH_CALIB_ADDR, (uint32_t*)&value, 1);
  while (!NRF_NVMC->READY);

  NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
  Serial.println("Calibration factor saved to flash.");
}

// ===== Calibration =====
void performCalibration() {
  Serial.println("\n=== Calibration Mode ===");
  Serial.println("Press [Enter] to TARE with empty scale...");
  while (!Serial.available());
  while (Serial.available()) Serial.read();  // Clear buffer
  scale.set_scale(1.0);
  scale.tare();
  delay(1000);
  Serial.println("Tare complete.");

  Serial.println("Place known weight and press [Enter]...");
  while (!Serial.available());
  while (Serial.available()) Serial.read();  // Clear buffer
  long rawAverage = scale.get_units(10);
  Serial.print("Raw average: ");
  Serial.println(rawAverage);

  Serial.println("Enter known weight in POUNDS:");
  float knownWeight = 0.0;
  while (knownWeight <= 0.0) {
    while (!Serial.available());
    knownWeight = Serial.parseFloat();
  }

  calibrationFactor = abs((float)rawAverage / knownWeight);
  Serial.print("New calibration factor: ");
  Serial.println(calibrationFactor, 4);
  saveCalibration(calibrationFactor);

  scale.set_scale(calibrationFactor);
  delay(500);
  scale.tare();
  delay(500);
  offset = scale.get_units(10);  // Get offset reading after final tare

  Serial.print("Final tare complete. Offset: ");
  Serial.print(offset, 2);
  Serial.println(" lbs");
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1);
  }

  scale.begin(DOUT, CLK);
  Serial.println("Initializing HX711...");
  while (!scale.is_ready()) {
    Serial.println("Waiting for HX711...");
    delay(100);
  }

  // Load or prompt for calibration
  if (calibrationExists()) {
    calibrationFactor = readCalibration();
    Serial.print("Loaded calibration factor: ");
    Serial.println(calibrationFactor, 4);

    Serial.print("Recalibrate? (Y/N): ");
    while (!Serial.available());
    char answer = Serial.read();
    if (answer == 'Y' || answer == 'y') {
      performCalibration();
    }
  } else {
    performCalibration();
  }

  scale.set_scale(calibrationFactor);
  delay(500);
  scale.tare();
  delay(500);
  offset = scale.get_units(10);
  lastValidReading = 0;

  Serial.print("Initial reading after tare: ");
  Serial.println(0.0, 2);

  // BLE setup
  BLE.setLocalName("XIAO_nRF52840");
  BLE.setAdvertisedService(forceService);
  forceService.addCharacteristic(forceDataCharacteristic);
  BLE.addService(forceService);
  BLE.advertise();

  Serial.println("BLE advertising started...");
}

// ===== Main Loop =====
void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'C' || cmd == 'c') {
      performCalibration();
    }
  }

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
          float pounds = -(scale.get_units(1) - offset);
          pounds = abs(pounds);  // Ensure positive output

          // Optional outlier filter (can be removed or adjusted)
          if (abs(pounds - lastValidReading) > 100) {
            Serial.println("Outlier detected, skipping.");
            continue;
          }

          lastValidReading = pounds;
          String message = String(currentMillis - connectionMillis) + "|" + String(pounds, 1);
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
