
/**
 * 
 * 
 * 
 */

#include <SPI.h>
#include <RF24.h>
#include <Stepper.h>
#include <nRF24L01.h>

//Pin numbers defined
#define MOTOR_STEP_PIN 2
#define MOTOR_DIR_PIN 3

#define STOP_SWITCH_FRONT A2
#define STOP_SWITCH_BACK A3

#define ONE_REVOLUTION 800
#define MAX_SPEED 800

#define DOUBLE 2

RF24 radio(7, 8); //CE, CSN

const byte address[6] = "00001";

Stepper myMotor = Stepper(ONE_REVOLUTION, MOTOR_DIR_PIN, MOTOR_STEP_PIN);

unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

//Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package
{
  byte joystick_Y;
};

Data_Package data;

void setup()
{
  Serial.begin(115200);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(217);

  //Initialize as receiver
  radio.startListening();
  resetData();
}

void loop()
{
  int theYInput;

  if (radio.available())
  {
    radio.read(&data, sizeof(Data_Package));
    lastReceiveTime = millis();
  }

  theYInput = data.joystick_Y;

  //Checks whether to keep receving data or not
  currentTime = millis();
  //If current time is more then 1 second since we have recived the last data, that means we have lost connection
  if (currentTime - lastReceiveTime > 1000)
  {
    // If the connection is lost, resets the data. Prevents unwanted behavior
    resetData();
  }

  Serial.print("Joystick_Y: ");
  Serial.println(theYInput);
  delay(100);

  //Joystick centered
  if (theYInput > 120 && theYInput < 150)
  {
    //Does nothing
  }

  //Move motors forward, 
  if (digitalRead(STOP_SWITCH_FRONT) == LOW)
  {
    //Stop movement in the wrong direction
  }
  else
  {
    if (theYInput > 160)
    {
      myMotor.step(DOUBLE);
    }
  }

  //Move motors backward, 
  if (digitalRead(STOP_SWITCH_BACK) == LOW)
  {
    //Stop movement in the wrong direction
  }
  else
  {
    if (theYInput < 110)
    {
      myMotor.step(-DOUBLE);
    }
  }
}

void resetData()
{
  //Resets to initial default values
  data.joystick_Y = 135;
}
