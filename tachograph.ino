
//
// Kayak Electronics www.kayak-electronics.com
// 
// Double Digital Tachograph 
// Januar 2013.

#define chipSelectPin 10    // this is used in max7219.h
//#define USE_SERIAL

#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <SPI.h>
#include "max7219.h"

const int SensorPin = A0;
const int TresholdPin = 9;
const int LedPin = 6;

unsigned int counter = 0; 
unsigned int treshold = 0; 
long lowThreshold = 1000; 

bool LightDetected = false;

void setup() { 
  pinMode(LedPin,OUTPUT);
  pinMode(SensorPin,INPUT);
  pinMode(TresholdPin,INPUT);
  digitalWrite(TresholdPin,HIGH);

#ifdef USE_SERIAL
   Serial.begin(9600);
#endif
  pinMode(chipSelectPin, OUTPUT);
  SPI.begin();
  max7219_init1();               // initialize  max7219 
 
 
  if (digitalRead(TresholdPin))
  {
    EEPROM_readAnything(0, treshold);
    EEPROM_readAnything(10, lowThreshold);
  }
  else
  {
    unsigned int lowValue = 0; 
    unsigned int lowValueLow = 1000; 
    unsigned int peak1 = 1000; 
    unsigned int peak2 = 0; 

    for (int i=0;i<3000;i++)
      { 
        int temp = analogRead(SensorPin);
        if (temp<peak1)
          peak1=temp;
        if (temp>peak2)
          peak2 = temp;
          
      }
      
         
      display_number(0);
      lowValue = analogRead(SensorPin);
      delay(500);
      for (int i=0;i<30000;i++)
      { 
        int temp = analogRead(SensorPin);
        if (temp > lowValue)
        {
          lowValue = temp;
           display_number(lowValue);
        }
        if (temp < lowValueLow)
        {
          lowValueLow = temp;
        }
      }
#ifdef USE_SERIAL
    Serial.print("lowValue ");
    Serial.println(lowValue);
    Serial.print("peak2 ");
    Serial.println(peak2 - peak1);
#endif
    display_number(lowValue);
    //delay(2000);
    digitalWrite(LedPin,HIGH);
  //  display_number(lowValueLow);
    //delay(2000);
    
    int highValue = lowValue;
    for (long i=0;i<60000;i++)
    {
      int temp = analogRead(SensorPin);
      if (temp>highValue)
      {
       highValue = temp;
     display_number(highValue);
      }
//      delay(10);
    }
#ifdef USE_SERIAL
    Serial.print("highValue ");
    Serial.println(highValue);
#endif
    digitalWrite(LedPin,LOW);

      display_number(highValue);
    
    treshold = lowValue + 3 * (peak2-peak1) / 5 ;
    lowThreshold = (signed)treshold-(signed)(peak2-peak1) - 30;
    if (lowThreshold < lowValueLow+(peak2-peak1)/2)
       lowThreshold=lowValueLow+(peak2-peak1)/2;
         
 
 #ifdef USE_SERIAL
    Serial.print("lowThreshold ");
    Serial.println(lowThreshold);
#endif   
    display_number(treshold);
    delay(5000);
    display_number(lowThreshold);
    delay(5000);
    display_number(peak2-peak1);
    delay(5000);
 
 
    EEPROM_writeAnything(0, treshold);
    EEPROM_writeAnything(10, lowThreshold);
    
#ifdef USE_SERIAL
    Serial.print("treshold ");
    Serial.print(treshold);
    Serial.print(" razlika ");
    Serial.println(highValue - lowValue);
#endif
    while(! digitalRead(TresholdPin));
  
  } //end digitalRead

  display_number(counter);
  } //end setup

void loop(){
  if (LightDetected && analogRead(SensorPin) < lowThreshold)
  {
    digitalWrite(LedPin,LOW);
    LightDetected = false;
    delay(15);
  }
  if (!LightDetected && analogRead(SensorPin) > treshold)
  {
    digitalWrite(LedPin,HIGH);
    LightDetected = true;
    counter++;
    if (counter==10000)
      counter = 0;
    display_number(counter);
    delay(10);
#ifdef USE_SERIAL
    Serial.println(counter);
 //       Serial.println("       ");

#endif

  }
}

