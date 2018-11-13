//
// Kayak Electronics www.kayak-electronics.com
// 
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

  if (digitalRead(RecordPin))
  {

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
  }
  else
  {
    mySerial.println("recording state");

    int maxOfMinimumValue = 0 ;    // greatest minim value when light is detected
    int minOfMaximumValue = 1024;       // lowest maximum value when there is no light
    int minMaxRange = 0; //  minOfMaximumValue - maxOfMinimumValue
    int peakLow = 1000; // here we store low value
    int peakHigh = 0;   // here we store high value of noise

    bool shouldRestartCalculation = false;
    display_number(0);

    peakLow = 1000; // here we store low value
    peakHigh = 0;   // here we store high value of noise
    // analogRead takes 100us=0.1ms, 
    // but oscilation are on 10ms (light) and  1ms (7 seg display)
    // so 15ms = 150*0.1ms
    for (int i=0;i<150;i++)
    { 
      int temp = analogRead(SensorPin);
      if (temp<peakLow)
        peakLow=temp;
      if (temp>peakHigh)
        peakHigh = temp;
      //        delay(1); i+=10; // if delay(1) is used then it should be 15 rounds
    }
    int peak = peakHigh - peakLow;
    mySerial.print("peak ");
    mySerial.println(peak);
    display_number(peak);

    int start = peakLow;
    int globalMin = start;
    int blackMin = start;
    bool wasBelow=false;
    unsigned long timeBetween, timeLast = millis();
    for (int i = 0;i<55;)
    {
      int localMin = blackMin - peak;
      bool wasBelow = false;
      while(1)
      {
        int temp = analogRead(SensorPin);
        if (temp < localMin )
        {
          localMin=temp;
          wasBelow = true;
        }
        if (wasBelow && temp > blackMin+ peak)
        {
          break;
        }
      }// 
      if (localMin < globalMin + (blackMin-globalMin) / 2 )
      {
        unsigned long now = millis();
        timeBetween = now - timeLast;
        timeLast = now;
        mySerial.print(i++);
        mySerial.print(" ");
        mySerial.println(timeBetween);
        if (blackMin == start)
        {
          blackMin = (globalMin + start)/2;
        }
        if (localMin < globalMin)
        {
          globalMin = localMin;
          mySerial.print("*** globalMin ");
          mySerial.println(globalMin);
        }

      }
      else 
      {
        if (localMin < blackMin )
        {
          blackMin = localMin;       
        }
        mySerial.print("blackMin ");
        mySerial.println(blackMin); 
      }
    }

    mySerial.print("peakLow ");
    mySerial.println(peakLow);
    while(1);
    EEPROM_writeAnything(0, highThreshold);
    EEPROM_writeAnything(10, lowThreshold);

  } // if (digitalRead(RecordPin))

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

