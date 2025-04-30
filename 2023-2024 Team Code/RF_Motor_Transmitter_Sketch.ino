
/**
 * 
 * 
 * 
 */

#include <SPI.h>
#include <RF24.h>
#include <Wire.h>
#include <nRF24L01.h>

#define JOYSTICK_Y A0

RF24 radio(7, 8); //CE, CSN
//Address
const byte address[6] = "00001";

// Max size of this struct is 32 bytes which is the NRF24L01 buffer limit
struct Data_Package
{
  byte joystick_Y;
};

Data_Package data;

void setup()
{
  Serial.begin(115200);

  //RF communication
  radio.begin();
  radio.openWritingPipe(address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(217);

  //Initialize joystick pins
  pinMode(JOYSTICK_Y, INPUT_PULLUP);

  //Default values set up
  data.joystick_Y = 135;
}

void loop()
{
  //
  //int theYInput = analogRead(JOYSTICK_Y);
  data.joystick_Y = map(analogRead(JOYSTICK_Y), 15, 1023, 0, 255);

  Serial.println(data.joystick_Y);
  
  //Send the data package that is a structure to the receiver
  radio.write(&data, sizeof(Data_Package));
}
