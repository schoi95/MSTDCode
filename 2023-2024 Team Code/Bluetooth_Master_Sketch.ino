
/**
 * Master Bluetooth Module Sketch
 * 
 * Slave address -> 0021:11:01A356
 *
 * By: Brian LeSmith
 */

#include <SoftwareSerial.h>

//#define JOYSTICK_X A1
#define JOYSTICK_Y A0

SoftwareSerial BTSerial(2, 3);

void setup()
{
  Serial.begin(9600);
  BTSerial.begin(9600);

  pinMode(JOYSTICK_Y, INPUT_PULLUP);
}

void loop()
{
  int theYInput = analogRead(JOYSTICK_Y);

  String data = String(theYInput);
  
  BTSerial.println(data);

  delay(100);

  while (BTSerial.available())
  {
    char c = BTSerial.read();
    Serial.write(c); //Slave response
  }
  /*
  int theYInput = analogRead(JOYSTICK_Y);

  //Joystick centered
  if (theYInput > 400 && theYInput < 420)
  {
    //Does nothing
  }

  if (theYInput > 440)
  {
    BTSerial.write(F("Move motors forward.\n"));
    Serial.println("Move motors forwards.");
  }
  
  if (theYInput < 380)
  {
    BTSerial.write(F("Move motors backwards.\n"));
    Serial.println("Move motors backwards.");
  }
  */
}
