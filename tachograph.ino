//
// Kayak Electronics www.kayak-electronics.com
// https://github.com/duleorlovic/tachograph
// Double Digital Tachograph 
// Januar 2013.
// TODO: analogReadResolution to 9 bits

//#define USE_SERIAL

#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <SPI.h>
#define chipSelectPin 10    // this is used in max7219.h, also 12 and 13 pin+
#include "max7219.h"

const int SensorPin = A0;
const int SecondDisplayPin = 4;
const int MeterPin = 3;
const int SecondSignalPin = 2;  // signal from second machine
const int RecordPin = 6;
const int LedPin = 7;
const int StopRelayPin = 9;
const int SlowDownRelayPin = 8;

const int FirstNumberPin = A4;
const int SecondNumberPin = A3;
const int ThirdNumberPin = A5;
const int FourthNumberPin = A2;
const int SlowDownNumberPin = A1;


#ifdef USE_SERIAL
#define mySerial Serial
#else // #ifdef USE_SERIAL
// if we do not use serial port
class classSerial{
  public:
    void begin(char) {};
    void print(char*) {};
    void print(int) {};
    void println(char*) {};
    void println(int) {};
} mySerial;
#endif

int slowDown = 10;
int counter = 0;
volatile int secondCounter = 0;
int displayedSecondCounter = 0;
int targetCounter = 0;
int highThreshold = 0; 
int lowThreshold = 1024; 
bool wasSecondCounter = false;
volatile unsigned long timeLast = 0;

bool LightDetected = false;  // assume that we start when there is no light

void updateSecondCounter()
{
  unsigned long now = millis();
  if (now - timeLast > 15)
  {
    secondCounter++;
  }
  timeLast = now;
}
void setup() { 
  //  pinMode(SensorPin,INPUT);
  //  pinMode(ResistorPin,INPUT);
  //  pinMode(RecordPin,INPUT);
  pinMode(LedPin,OUTPUT);
  pinMode(chipSelectPin, OUTPUT);
  pinMode(SlowDownRelayPin, OUTPUT);
  pinMode(StopRelayPin, OUTPUT);
  digitalWrite(RecordPin,HIGH); // pull up resistor
  digitalWrite(SecondDisplayPin,HIGH); // pull up resistor
  digitalWrite(SlowDownRelayPin,HIGH);  // disable slow down
  digitalWrite(StopRelayPin,HIGH);  // disable slow down
  attachInterrupt(SecondSignalPin - 2, updateSecondCounter, FALLING);

  mySerial.begin(9600);
  mySerial.println("start");
  SPI.begin();
  max7219_init1();               // initialize  max7219 

  int numberAnalog[4];
  numberAnalog[0] = analogRead(FirstNumberPin);
  numberAnalog[1] = analogRead(SecondNumberPin);
  numberAnalog[2] = analogRead(ThirdNumberPin);
  numberAnalog[3] = analogRead(FourthNumberPin);
  mySerial.print("   ");
  int decade = 1;
  for (int i =0;i<4;i++)
  {
    mySerial.print(numberAnalog[i]);
    mySerial.print(" ");
    targetCounter += (numberAnalog[i]+32)/64 * decade;
    decade *= 10;
  }
  mySerial.println(targetCounter);
  display_number(targetCounter);
  delay(3000);

  slowDown = (analogRead(SlowDownNumberPin)+32)/64;

  EEPROM_readAnything(0, highThreshold);
  EEPROM_readAnything(10, lowThreshold);
  mySerial.print("slowDown ");
  mySerial.println(slowDown);
  mySerial.print("threshold ");
  mySerial.println(highThreshold);
  mySerial.print("lowThreshold ");
  mySerial.println(lowThreshold);     
  mySerial.println("counter");
  secondCounter = 0;
  display_number(secondCounter);

} //end setup

// **********************************************************
void loop(){
  // **********************************************************  
  if (secondCounter != displayedSecondCounter)
  {
    displayedSecondCounter = secondCounter;
    display_increment_number(displayedSecondCounter);
    if (displayedSecondCounter == targetCounter - slowDown)
    {
      digitalWrite(SlowDownRelayPin,LOW);
    }
    if (displayedSecondCounter == targetCounter)
    {
      digitalWrite(StopRelayPin,LOW);
      delay(5000);
      digitalWrite(StopRelayPin,HIGH);
      digitalWrite(SlowDownRelayPin,HIGH);
      secondCounter = displayedSecondCounter = 0;
      counter = 0;
      display_number(0);
    }
  }
  delay(10);
} // void loop(){
