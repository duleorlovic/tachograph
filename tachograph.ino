
//
// Kayak Electronics www.kayak-electronics.com
// 
// Double Digital Tachograph 
// Januar 2013.
// TODO: analogReadResolution to 9 bits

#define chipSelectPin 10    // this is used in max7219.h
//#define USE_SERIAL
#define SAMPLE_SIZE 2000

#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <SPI.h>
#include "max7219.h"

const int SensorPin = A1;
const int ResistorPin = A2;
const int RecordPin = 5;
const int LedPin = 4;

unsigned int counter = 0; 
unsigned int treshold = 0; 
unsigned int lowThreshold = 1000; 

bool LightDetected = false;

void setup() { 
  pinMode(SensorPin,INPUT);
  pinMode(ResistorPin,INPUT);
  pinMode(RecordPin,INPUT);
  pinMode(LedPin,OUTPUT);
  pinMode(chipSelectPin, OUTPUT);
  pinMode(A6,INPUT);//we have connected 5 V here only because connector
  digitalWrite(A6,HIGH);
 
#ifdef USE_SERIAL
   Serial.begin(9600);
   Serial.println("start");
#endif
  digitalWrite(13,HIGH);
  digitalWrite(RecordPin,HIGH); // pull up resistor
  SPI.begin();
  max7219_init1();               // initialize  max7219 


  if (digitalRead(RecordPin))
  {
#ifdef USE_SERIAL
   Serial.println("counter");
#endif
     EEPROM_readAnything(0, treshold);
     EEPROM_readAnything(10, lowThreshold);
  }
  else
  {
#ifdef USE_SERIAL
   Serial.println("recording state");
#endif
    unsigned int peak1 = 1000; 
    unsigned int peak2 = 0; 


    digitalWrite(LedPin,HIGH);

    // analogRead takes 100us, 
    // but oscilation are on 10ms (light) and  1ms (led)
    // so 15ms = 150*0.1ms
    // delay 1 is 15
    for (int i=0;i<15;i++)
      { 
        int temp = analogRead(SensorPin);
        if (temp<peak1)
          peak1=temp;
        if (temp>peak2)
          peak2 = temp;
        delay(1);
      }
      
    unsigned int hist[512];
    unsigned int minValue = 1024; 
    unsigned int maxValue = 0; 
    for(int i=0;i<512;i++)
      hist[i]=0;    

  
    while(! digitalRead(RecordPin))
    {
      int temp = analogRead(SensorPin);

      if (temp > maxValue)
      {
        maxValue = temp;
      }
      if (temp < minValue)
      {
        minValue = temp;
      }
      display_number(maxValue - minValue);

      hist[temp/2]++;
      if (hist[temp/2]==SAMPLE_SIZE) //65535
      {
#ifdef USE_SERIAL
                Serial.println("stop measuring, start calculating");
#endif                
             
         digitalWrite(LedPin,LOW);
//        display_number(++counter*100);
        int i;
        for(i=minValue/2+2;i<maxValue/2;i++)
        {
          if (hist[i]>SAMPLE_SIZE/5)
            break;
        }

        lowThreshold = i*2 - (peak2-peak1);
//        lowThreshold = (minValue + i*2)/2;
        if (lowThreshold < minValue)
        {
#ifdef USE_SERIAL
          Serial.print("lowThreshold < minValue ");
          Serial.print(lowThreshold);
          Serial.print(" ");
          Serial.println(minValue);
#endif
          lowThreshold = (i*2 + minValue) / 2;
        } 
         
        treshold = i*2 + (signed)(peak2-peak1);
 //       treshold = (i*2 + maxValue) / 2;
        
        if (treshold > maxValue)
        {
#ifdef USE_SERIAL
          Serial.print("treshold > maxValue ");
          Serial.print(treshold);
          Serial.print(" ");
          Serial.println(maxValue);
#endif
          treshold = (i*2 + maxValue) / 2;
        } 
        

#ifdef USE_SERIAL
        Serial.print("minValue ");
        Serial.println(minValue);
        Serial.print("maxValue ");
        Serial.println(maxValue);
        Serial.print("peak2 - peak1 = ");
        Serial.print(peak2);    
        Serial.print(" - ");
        Serial.print(peak1);
        Serial.print(" = ");
        Serial.println(peak2 - peak1);
        Serial.print("2*i ");
        Serial.println(2*i);
        Serial.print("threshold ");
        Serial.println(treshold);
        Serial.print("lowThreshold ");
        Serial.println(lowThreshold);
    
        for(int i=0;i<512;i++)
        {
          Serial.println(hist[i]);
        }
#endif   
      display_number(treshold);
      while(! digitalRead(RecordPin));

      }
      
    
    }
  
 
    EEPROM_writeAnything(0, treshold);
    EEPROM_writeAnything(10, lowThreshold);
  
  } //end digitalRead



  display_number(counter);
  digitalWrite(13,LOW);

  } //end setup

void loop(){
  if (!LightDetected && analogRead(SensorPin) < lowThreshold)
  {
    digitalWrite(LedPin,HIGH);
    LightDetected = true;
    delay(10);
  }
  if ( LightDetected && analogRead(SensorPin) > treshold)
  {
    digitalWrite(LedPin,LOW);
    LightDetected = false;
    counter++;
    if (counter==10000)
      counter = 0;
    display_number(counter);
    delay(2);
#ifdef USE_SERIAL
    Serial.println(counter);
 //       Serial.println("       ");

#endif

  }
}

