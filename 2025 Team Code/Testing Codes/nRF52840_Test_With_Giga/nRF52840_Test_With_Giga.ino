#include <ArduinoBLE.h>

BLEService messageService("180C"); // Custom service UUID
BLECharacteristic messageCharacteristic("2A56", BLERead | BLENotify, 100); // 100 bytes buffer for message

unsigned long connectionMillis = 0;
unsigned long previousMillis = 0;
const long interval = 3000; // 3 seconds

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("XIAO_nRF52840");
  BLE.setAdvertisedService(messageService);

  messageService.addCharacteristic(messageCharacteristic);
  BLE.addService(messageService);

  messageCharacteristic.writeValue("Waiting for connection...");

  BLE.advertise();
  Serial.println("Peripheral advertising started...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    connectionMillis = millis();   // <<<<<< Start timer when connected
    previousMillis = 0;             // Reset sending timer

    while (central.connected()) {
      unsigned long currentMillis = millis();
      
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // Calculate time since connection
        unsigned long timeSinceConnection = currentMillis - connectionMillis;
        
        String message = String(timeSinceConnection) + "|Hello MSTD Team";
        messageCharacteristic.writeValue(message.c_str());
        
        Serial.print("Sent message: ");
        Serial.println(message);
      }
    }

    Serial.println("Disconnected from central");
    connectionMillis = 0;   // Optional: Reset when disconnected
  }
}
