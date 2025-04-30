
/**
 * Muscle Strength Testing Device Sketch - Motor Controls
 * 
 * This sketch is for the Arduino Uno R3 that controls the motors through an IR sensor and remote.
 * 
 * IR was utilized for wireless control instead of bluetooth or RF as both when I was working with them,
 * tended to be very unstable and at times unusable. IR made the most sense since we only needed to point
 * at the cage for motor movement and it was much simpler to work with and code. You may notice there are
 * limit and/or stop switches coded but they are not on the cage. The code was written and tested, and proved
 * to work but the team ran out of time in trying to create a wiring harness and add them to the cage.
 * 
 * @author: Brian LeSmith
 */

#include <IRremote.h>
#include <MobaTools.h>

//Pin numbers defined
#define MOTOR_STEP_PIN 2
#define MOTOR_DIR_PIN 3

//IR receiver pin defined
#define RECV_PIN 4

//Stop switches defined
#define STOP_SWITCH_FRONT A2
#define STOP_SWITCH_BACK A3

//Variables relating to Motor parameters defined
#define ONE_REVOLUTION 3200
#define MAX_SPEED 800

#define DIRECTION 1

//Timing manipulation variables
unsigned long lastReceivedTime = 0;
const unsigned long timeOut = 125;

//Motor enabled with how much a single revolution should be and an internal variable
MoToStepper myMotor(ONE_REVOLUTION, STEPDIR);

void setup()
{
  //Using a higher baud rate since the arduino uno is capable and it speeds things up
  Serial.begin(115200);

  //Initializing the IR receiver
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);

  //Setting the motors pins, maximum speed, and acceleration or how it "ramps up"
  myMotor.attach(MOTOR_STEP_PIN, MOTOR_DIR_PIN);
  myMotor.setSpeed(MAX_SPEED);
  myMotor.setRampLen(50);

  //Analog declarations for the stop switches
  pinMode(STOP_SWITCH_FRONT, INPUT_PULLUP);
  pinMode(STOP_SWITCH_BACK, INPUT_PULLUP);
}

void loop()
{
  int irCode;

  if (IrReceiver.decode())
  {
    //Parses the IR signal for specific commands
    irCode = IrReceiver.decodedIRData.command;
    Serial.println(irCode);

    //Move motors forward
    if (digitalRead(STOP_SWITCH_FRONT) == LOW)
    {
      //Stop movement in the wrong direction
      myMotor.stop();
      Serial.println("Motors stopped.");
    }
    else
    {
      //Only allow movement in this direction if the stop switch is down
      if (irCode == 9)
      {
        myMotor.rotate(DIRECTION);
        Serial.println("Motors moving forward.");
        lastReceivedTime = millis();
      }
    }

    //Move motors backward
    if (digitalRead(STOP_SWITCH_BACK) == LOW)
    {
      //Stop movement in the wrong direction
      myMotor.stop();
      Serial.println("Motors stopped.");
    }
    else
    {
      //Only allow movement in this direction if the stop switch is down
      if (irCode == 7)
      {
        myMotor.rotate(-DIRECTION);
        Serial.println("Motors moving backward.");
        lastReceivedTime = millis();
      }
    }

    //Resumes looking for the next IR signal
    IrReceiver.resume();
  }

  /**
   * Detects if the user has let go of the movement button on the IR remote
   * for 125ms, if they have, the motors stop. This creates an near no delay
   * effect where the motors move when a button is HELD down but not when it
   * is let go. This logic must be outside of the if statement of IR.decode()
   * but still in the sketch loop() function.
   */
  if (millis() - lastReceivedTime > timeOut)
  {
    //Stops motor movement
    myMotor.stop();
    Serial.println(F("Motors stopped."));
  }
}
