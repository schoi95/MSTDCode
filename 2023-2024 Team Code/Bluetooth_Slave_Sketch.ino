
/**
 * Slave Bluetooth Module Sketch
 * 
 * Slave address -> 0021:11:01A356
 *
 * By: Brian LeSmith
 */

#include <SoftwareSerial.h>

SoftwareSerial BTSerial(2, 3);

void setup()
{
  Serial.begin(9600);
  BTSerial.begin(9600);
}

void loop()
{
  String output = "";

  if (BTSerial.available())
  {
    output = BTSerial.read();
  }

  Serial.println(output);
  /*
  String input = "";
  //Receiving data from Serial port
  if (BTSerial.available())
  {
    input = BTSerial.read();
    Serial.println(input);
  }

  int num = input.toInt();

  if (num > 580)
  {
    //Serial.println(num);
  }

  if (num < 520)
  {
    //Serial.println(num);
  }
  
  //Joystick centered
  if ()
  {
    //Does nothing
  }
  
  //Move motors forward, 
  if (false)
  {
    //Stop movement in the wrong direction
  }
  else
  {
    if (num > 570)
    {
      Serial.println("Move motors forward.");
    }
  }

  //Move motors backward, 
  if (false)
  {
    //Stop movement in the wrong direction
  }
  else
  {
    if (num < 520)
    {
      Serial.println("Move motors backward.");
    }
  }
  */
}
