
//
// Kayak Electronics www.kayak-electronics.com
// 
// Double Digital Tachograph 
// Januar 2013.
// TODO: analogReadResolution to 9 bits

#define chipSelectPin 10    // this is used in max7219.h
#define USE_SERIAL

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
  display_number(0);

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
    display_number(1);

    digitalWrite(LedPin,HIGH);

    // analogRead takes 100us, 
    // but oscilation are on 10ms (light) and  1ms (led)
    // so 15ms = 150*0.1ms
    for (int i=0;i<150;i++)
      { 
        int temp = analogRead(SensorPin);
        if (temp<peak1)
          peak1=temp;
        if (temp>peak2)
          peak2 = temp;
      }
      
    unsigned int hist[512];
    unsigned int minValue = 1024; 
    unsigned int maxValue = 0; 
    for(int i=0;i<512;i++)
      hist[i]=0;    
    int minPeakIndex=0;

  
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
      if (hist[temp/2]==65535)
      {
     
//        display_number(++counter*100);
        int maxPeak=0;
        int maxPeakIndex=0;
        int minPeak=1023;
        minPeakIndex=0;
        bool peakFound = false;
        int notPeak=0;
        
        int a = hist[minValue/2];
        int b = hist[minValue/2 +1];
        for(int i=minValue/2+2;i<maxValue/2;i++)
        {
          int temp = (a+b+hist[i])/3;
          a = b;
          b = temp;
          
          if (!peakFound)
          {
            if (temp > maxPeak)
            {
              maxPeak = temp;
              maxPeakIndex = i;
              notPeak --;
              if (notPeak<0)
                notPeak = 0;
            }
            else
            {
              notPeak++;
              if (notPeak > 5)
                peakFound = true;
            }
          }
          else // peakFound
          {
            if (temp < minPeak)
            {
              minPeak = temp;
              minPeakIndex = i;
              notPeak --;
              if (notPeak<0)
                notPeak = 0;
            }
            else
            {
              if (temp > maxPeak)
              {
#ifdef USE_SERIAL
                Serial.print("should not be here ");
                Serial.print(temp);
                Serial.println(maxPeak);

                
#endif                
                maxPeak=0;
                maxPeakIndex = 0;
                break;
              }
              notPeak++;
              if (notPeak > 2)
                break;
            }
          } // end peakFound
        } // end for
        
        display_number(maxPeakIndex);
        delay(10000);
        display_number(minPeakIndex);
        delay(10000);

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
        Serial.print("maxPeakIndex ");
        Serial.println(maxPeakIndex);    
        Serial.print("minPeakIndex ");
        Serial.println(minPeakIndex);
    
        for(int i=0;i<512;i++)
        {
          Serial.println(hist[i]);
        }
#endif           
        for(int i=0;i<512;i++)
          hist[i]=0;
        minValue = 1024; 
        maxValue = 0; 
      }
      
    }
    
    lowThreshold = minPeakIndex*2;
    treshold = lowThreshold + (signed)(peak2-peak1)*5;
//    if (lowThreshold < lowValueLow+(peak2-peak1)/2)
  //     lowThreshold=lowValueLow+(peak2-peak1)/2;
         
 #ifdef USE_SERIAL
    Serial.print("threshold ");
    Serial.println(treshold);
    Serial.print("lowThreshold ");
    Serial.println(lowThreshold);
#endif   
 
 
    EEPROM_writeAnything(0, treshold);
    EEPROM_writeAnything(10, lowThreshold);
  
  } //end digitalRead

    digitalWrite(LedPin,HIGH);

  display_number(counter);
  digitalWrite(13,LOW);

  } //end setup

void loop(){
  if (!LightDetected && analogRead(SensorPin) < lowThreshold)
  {
    digitalWrite(LedPin,HIGH);
    LightDetected = true;
    delay(15);
  }
  if ( LightDetected && analogRead(SensorPin) > treshold)
  {
    digitalWrite(LedPin,LOW);
    LightDetected = false;
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

