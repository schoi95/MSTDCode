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
        connectionMillis = millis();
        lastMessageMillis = 0;

        if (peripheral.discoverAttributes()) {
          messageCharacteristic = peripheral.characteristic("2A56");
          if (messageCharacteristic) {
            messageCharacteristic.subscribe();
            Serial.println("Subscribed to characteristic!");
          } else {
            Serial.println("Characteristic not found.");
            peripheral.disconnect();
            BLE.scan();
          }
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
      buffer[length] = '\0';

      String message = (char*)buffer;
      int separatorIndex = message.indexOf('|');

      if (separatorIndex != -1) {
        String timestamp = message.substring(0, separatorIndex);
        String poundsStr = message.substring(separatorIndex + 1);

        float pounds = poundsStr.toFloat();
        unsigned long now = millis();

        Serial.println("----------------------------------------------------");
        Serial.print("Force: ");
        Serial.print(pounds, 2);
        Serial.println(" lbs");

        Serial.print("Time since connection: ");
        Serial.println(now - connectionMillis);

        if (lastMessageMillis > 0) {
          Serial.print("Time since last message: ");
          Serial.println(now - lastMessageMillis);
        } else {
          Serial.println("First message received.");
        }

        lastMessageMillis = now;
        Serial.println("----------------------------------------------------");
      } else {
        Serial.println("Malformed message.");
      }
    }
  } else {
    Serial.println("Peripheral disconnected. Scanning...");
    peripheral = BLEDevice();
    BLE.scan();
  }
}
