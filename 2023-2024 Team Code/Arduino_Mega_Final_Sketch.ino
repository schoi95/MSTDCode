
/**
 * Muscle Strength Testing Device Sketch
 * 
 * This sketch is essentially the main sketch that controls the LCD screen, the load sensors and calculations,
 * as well as the saving and formatting of the file that is saved to the SD card. This sketch is intended to be
 * run on a Arduino Mega R3. Uncomment serial statements if you are connected to a PC and need to view diagnostics
 * through the Serial monitor.
 * 
 * As a little side note, I Brian LeSmith, the author of this sketch was hired on as a private contractor to this
 * capstone by the University of Washington and this capstone's sponsor, Dr. Farnes. The previous team's work
 * is essentially unusable as the code contains no documentation and is done in a very analog manner not
 * utilizing libaries or the like. I did the utmost that I could to ensure the software side of things
 * would be good to go for a fully functioning prototype, the first one in the history of this capstone.
 * I will leave all of the work that I did, even things I did not ultimately complete or get working,
 * in the hopes that you, the next team will either be able to use it, or can possibly get it working.
 * 
 * @author: Brian LeSmith
 */

#include <SPI.h>
#include "HX711.h"
#include <SdFat.h>
#include <IRremote.h>
#include <LCDWIKI_GUI.h>
#include <LCDWIKI_SPI.h>

//Using define instead of constants because it is better to
#define MODEL ST7796S

//LCD pins defined
#define CS A0
#define RESET A4
#define DC A8
#define LED A12

#define MOSI 28
#define SCK 29
#define MISO 30

//SD card reader pins defined
#define SD_PIN 53

//IR receiver pin defined
#define RECV_PIN 35

//Pins for HX711 module defined
#define DOUT_PIN_LOAD_1 22
#define CLOCK_PIN_LOAD_1 23
#define DOUT_PIN_LIMB_1 34
#define CLOCK_PIN_LIMB_1 36
#define DOUT_PIN_LIMB_2 38
#define CLOCK_PIN_LIMB_2 40

//Rotation of LCD screen defined
#define ROTATION 3

//Hexadecimal colors defined
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

/**
 * Calibration to convert floating point numbers to LB
 * Quite literally the only thing used from the previous team was the limb
 * calibration factor. We found the body calibration factor through trial
 * and error.
 */
#define calibration_factor_body -9750.0
#define calibration_factor_limb -93.5542

//SD and LCD object initialized
SdFat SD;
LCDWIKI_SPI myLCD(MODEL, CS, DC, MISO, MOSI, RESET, SCK, LED);

//Load sensors declared
HX711 myLoadCell1;
HX711 myLoadCell2;
HX711 myLoadCell3;

File dataFile;

//Graph variables for the live data on the LCD
const int graphWidth = myLCD.Get_Display_Height();
const int graphHeight = myLCD.Get_Display_Width();
const int graphX = 0;
const int graphY = 0;

int previousValue = 0;
int xPos = 0;

int counter = 0;

void setup()
{
  //Using a higher baud rate since the arduino mega is capable and it speeds things up
  //Serial.begin(115200);

  //Initializing the IR receiver
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);

  //LCD initialized, background color set
  myLCD.Init_LCD();
  myLCD.Set_Rotation(ROTATION);
  myLCD.Fill_Screen(BLACK);

  //LCD text size and color set
  myLCD.Set_Text_colour(WHITE);
  myLCD.Set_Text_Size(2);
  myLCD.Set_Text_Mode(0);
  myLCD.Set_Text_Back_colour(BLACK);

  //SD card initialization
  if (!SD.begin(SD_PIN))
  {
    //Serial.println("SD card could not be initialized!");
  }
  else
  {
    //Serial.println("SD card was successfully initialized!");
  }

  //Initializing load cells
  myLoadCell1.begin(DOUT_PIN_LIMB_1, CLOCK_PIN_LIMB_1);
  myLoadCell2.begin(DOUT_PIN_LIMB_2, CLOCK_PIN_LIMB_2);
  myLoadCell3.begin(DOUT_PIN_LOAD_1, CLOCK_PIN_LOAD_1);

  //Setting load cell scale according to calibration factor
  myLoadCell1.set_scale(calibration_factor_limb);
  myLoadCell2.set_scale(calibration_factor_limb);
  myLoadCell3.set_scale(calibration_factor_body);

  //Always tare the load cells
  myLoadCell1.tare();
  myLoadCell2.tare();
  myLoadCell3.tare();

  //Function checks the file on the SD card and deletes it if it is already there
  //checkSDCard();

  //Header for the csv file
  dataFile.print(F("LB"));
  dataFile.print(F(","));
  dataFile.println(F("ms"));
}

void loop()
{
  int irCode;

  if (IrReceiver.decode())
  {
    //Parses the IR signal for specific commands
    irCode = IrReceiver.decodedIRData.command;
    //Serial.println(irCode);

    //Switch case for saving profiles
    switch (irCode)
    {
      case 22:
        //Serial.println("Weight scale zeroed.");
        //Lets the user know when the load cells are zeroing
        myLCD.Print_String(F("Zeroing"), 385, 10);
        myLCD.Print_String(F("Zeroing"), 385, 30);
        myLoadCell1.tare();
        myLoadCell2.tare();
        myLoadCell3.tare();
        break;
      case 0xC:
        //Serial.println("Pressing 1");
        dataFile = SD.open("person_1.txt", FILE_WRITE);
        myLCD.Print_String(F("Recording Profile 1"), 10, 50);
        break;
      case 0x18:
        //Serial.println("Pressing 2");
        dataFile = SD.open("person_2.txt", FILE_WRITE);
        myLCD.Print_String(F("Recording Profile 2"), 10, 50);
        break;
      case 0x5E:
        //Serial.println("Pressing 3");
        dataFile = SD.open("person_3.txt", FILE_WRITE);
        myLCD.Print_String(F("Recording Profile 3"), 10, 50);
        break;
      case 0x8:
        //Serial.println("Pressing 4");
        dataFile = SD.open("person_4.txt", FILE_WRITE);
        myLCD.Print_String(F("Recording Profile 4"), 10, 50);
        break;
      case 0x1C:
        //Serial.println("Pressing 5");
        dataFile = SD.open("person_5.txt", FILE_WRITE);
        myLCD.Print_String(F("Recording Profile 5"), 10, 50);
        break;
      case 0x5A:
        //Serial.println("Pressing 6");
        dataFile = SD.open("person_6.txt", FILE_WRITE);
        myLCD.Print_String(F("Recording Profile 6"), 10, 50);
        break;
      case 0x42:
        //Serial.println("Pressing 7");
        dataFile = SD.open("person_7.txt", FILE_WRITE);
        myLCD.Print_String(F("Recording Profile 7"), 10, 50);
        break;
      case 0x52:
        //Serial.println("Pressing 8");
        dataFile = SD.open("person_8.txt", FILE_WRITE);
        myLCD.Print_String(F("Recording Profile 8"), 10, 50);
        break;
      case 0x4A:
        //Serial.println("Pressing 9");
        dataFile = SD.open("person_9.txt", FILE_WRITE);
        myLCD.Print_String(F("Recording Profile 9"), 10, 50);
        break;
    }

    //Resumes looking for the next IR signal
    IrReceiver.resume();
  }

  //Reading data from the load sensors
  float theLoad1 = myLoadCell1.get_units();
  float theLoad2 = myLoadCell2.get_units();
  float theLoad3 = myLoadCell3.get_units();

  //Calculating arm sensor value
  float weightTotal = theLoad1 + theLoad2;
  float calibratedValue = weightTotal / 453.59237;
  String output = String(calibratedValue);

  //Scaled value is calculated for the graph scale
  int scaledValue = map(99.9 - calibratedValue, 0, 100, graphHeight - 1, 0);

  //Draw the new line
  myLCD.Set_Draw_color(RED);
  myLCD.Draw_Line(xPos, graphY + graphHeight - previousValue - 1, xPos + graphWidth, graphY + graphHeight - scaledValue - 1);

  //Erase previous line to draw another line (and/or this is "updating" the live data)
  myLCD.Set_Draw_color(BLACK);
  myLCD.Draw_Line(xPos, graphY + graphHeight - previousValue - 1, xPos + graphWidth, graphY + graphHeight - scaledValue - 1);

  //Save the current value for the next loop
  previousValue = scaledValue;
  
  //Printing arm sensor weight
  myLCD.Print_String(F("Arm sensor: "), 10, 10);
  myLCD.Print_String(output, 190, 10);
  myLCD.Print_String(F("LB"), 270, 10);

  //Printing body sensor weight
  myLCD.Print_String(F("Weight sensor: "), 10, 30);
  myLCD.Print_String(String(theLoad3), 190, 30);
  myLCD.Print_String(F("LB"), 270, 30);

  //Printing the scale of the graph
  myLCD.Print_String(F("0"), 465, 300);
  myLCD.Print_String(F("50"), 455, 145);

  //Printing the state of the load cells
  myLCD.Print_String(F("Working"), 385, 10);
  myLCD.Print_String(F("Working"), 385, 30);

  //Serial.print(calibratedValue, 2);
  //Serial.println(F(" lb"));

  //Checking if a user is standing in the cage to record data
  if (theLoad3 > 5.0)
  {
    //Saving arm sensor data to SD card
    dataFile.print(calibratedValue, 2);
    dataFile.print(F(","));
    dataFile.println(counter);

    dataFile.flush();
  }

  //The counter is specifically for the data which is taken every 50ms
  counter += 50;
}

/**
 * This function is for checking if the SD card exists, and then
 * initializing it if it does.
 */
void checkSDCard()
{
  char fileName[] = "test.txt";
  boolean flag = SD.exists(fileName);

  //Deletes the file if it already exists
  if (flag)
  {
    SD.remove(fileName);
    //Serial.println("File deleted.");
  }

  //Creates and opens the file for writing on the SD card
  dataFile = SD.open("test.txt", FILE_WRITE);

  /**
   * Lets the user know, if the arduino is hooked up to a PC with the serial monitor
   * enabled if the file was created successfully or not.
   */
  if (dataFile)
  {
    //Serial.println("Data file created successfully.");
  }
  else
  {
    //Serial.println("Error creating data file.");
  }
}
