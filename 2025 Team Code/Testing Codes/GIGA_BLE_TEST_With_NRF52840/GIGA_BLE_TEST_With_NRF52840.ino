#include <ArduinoBLE.h>

BLEDevice peripheral;
BLECharacteristic messageCharacteristic;

unsigned long connectionMillis = 0;
unsigned long lastMessageMillis = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  Serial.println("BLE Central - Scanning...");
  BLE.scan();
}

void loop() {
  if (!peripheral) {
    peripheral = BLE.available();
    if (peripheral && peripheral.localName() == "XIAO_nRF52840") {
      Serial.println("Found XIAO_nRF52840. Connecting...");
      BLE.stopScan();

      if (peripheral.connect()) {
        Serial.println("Connected!");
        connectionMillis = millis();  // <<<<<<<<<< RESET timer here
        lastMessageMillis = 0;         // Reset last message time too

        if (peripheral.discoverAttributes()) {
          Serial.println("Attributes discovered.");

          messageCharacteristic = peripheral.characteristic("2A56");

          if (messageCharacteristic) {
            Serial.println("Subscribed to characteristic!");
            messageCharacteristic.subscribe();
          } else {
            Serial.println("Characteristic not found.");
            peripheral.disconnect();
            BLE.scan();
          }
        } else {
          Serial.println("Attribute discovery failed.");
          peripheral.disconnect();
          BLE.scan();
        }
      } else {
        Serial.println("Connection failed.");
        BLE.scan();
      }
    }
  } else if (peripheral.connected()) {
    if (messageCharacteristic && messageCharacteristic.valueUpdated()) {
      int length = messageCharacteristic.valueLength();
      uint8_t buffer[length + 1];
      messageCharacteristic.readValue(buffer, length);
      buffer[length] = '\0';  // Null terminator

      String receivedMessage = (char*)buffer;

      // Print timestamps only AFTER connection
      unsigned long currentMillis = millis();
      unsigned long timeSinceConnection = currentMillis - connectionMillis;

      Serial.println("----------------------------------------------------");
      Serial.print("Received message: ");
      Serial.println(receivedMessage);

      Serial.print("Time since connection (ms): ");
      Serial.println(timeSinceConnection);

      if (lastMessageMillis > 0) {
        unsigned long timeBetweenMessages = currentMillis - lastMessageMillis;
        Serial.print("Time since last message (ms): ");
        Serial.println(timeBetweenMessages);
      } else {
        Serial.println("This is the first message since connecting!");
      }

      lastMessageMillis = currentMillis;
      Serial.println("----------------------------------------------------");
    }
  } else {
    Serial.println("Peripheral disconnected. Scanning...");
    peripheral = BLEDevice(); // Reset
    BLE.scan();
  }
}
