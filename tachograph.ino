
//
// Kayak Electronics www.kayak-electronics.com
// 
// Double Digital Tachograph 
// Januar 2013.
// TODO: analogReadResolution to 9 bits

#define chipSelectPin 10    // this is used in max7219.h
<<<<<<< HEAD
#define USE_SERIAL
=======
//#define USE_SERIAL
>>>>>>> fc2b9bd2864f67812bf1dac5dea2fea84377bd31
#define SAMPLE_SIZE 2000

#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <SPI.h>
#include "max7219.h"

const int SensorPin = A1;
const int ResistorPin = A2;
const int RecordPin = 5;
const int LedPin = 4;

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

int counter = 0; 
int highTreshold = 0; 
int lowThreshold = 1024; 

bool LightDetected = false;  // assume that we start when there is no light

void setup() { 
//  pinMode(SensorPin,INPUT);
//  pinMode(ResistorPin,INPUT);
//  pinMode(RecordPin,INPUT);
  pinMode(LedPin,OUTPUT);
  pinMode(chipSelectPin, OUTPUT);
//  pinMode(A6,INPUT);//we have connected 5 V here only because connector
  digitalWrite(A6,HIGH);
  digitalWrite(RecordPin,HIGH); // pull up resistor

   mySerial.begin(9600);
   mySerial.println("start");
  SPI.begin();
  max7219_init1();               // initialize  max7219 
<<<<<<< HEAD
=======

>>>>>>> fc2b9bd2864f67812bf1dac5dea2fea84377bd31

  if (digitalRead(RecordPin))
  {
     mySerial.println("counter");
     EEPROM_readAnything(0, highTreshold);
     EEPROM_readAnything(10, lowThreshold);
  }
  else
  {
<<<<<<< HEAD
   mySerial.println("recording state");
//    unsigned int hist[512];  // this reserve 512*2=1KB RAM
//    for(int i=0;i<512;i++)
//      hist[i]=0;    
    int maxOfMinumumValue = 0 ;    // greatest minim value when light is detected
    int minOfMaximumValue = 1024;       // lowest maximum value when there is no light
    int minMaxRange = 0; //  minOfMaximumValue - maxOfMinumumValue
    int peakLow = 1000; // here we store low value
    int peakHigh = 0;   // here we store high value of noise
    
    bool shouldRestartCalculation = false;
    display_number(0);
=======
#ifdef USE_SERIAL
   Serial.println("recording state");
#endif
    unsigned int peak1 = 1000; 
    unsigned int peak2 = 0; 


    digitalWrite(LedPin,HIGH);
>>>>>>> fc2b9bd2864f67812bf1dac5dea2fea84377bd31

    peakLow = 1000; // here we store low value
    peakHigh = 0;   // here we store high value of noise
    // analogRead takes 100us=0.1ms, 
    // but oscilation are on 10ms (light) and  1ms (7 seg display)
    // so 15ms = 150*0.1ms
<<<<<<< HEAD
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
    display_number(peak);
    mySerial.print("peak ");
    mySerial.println(peak);
    
    int temp,localMax,  localMin ;
    bool detectedBetweenThresholds = false;
    highTreshold = peakLow;
    lowThreshold = peakLow - 300;//10 * peak;
//      analogWrite(3,lowThreshold/4);
    mySerial.print("highTreshold ");
    mySerial.println(highTreshold);
    mySerial.print("lowThreshold ");
    mySerial.println(lowThreshold);
    while (! digitalRead(RecordPin))
    {
          //....................................................................
          mySerial.print(".");
          digitalWrite(LedPin,LOW);
          temp = analogRead(SensorPin);    // this is next at highThreshold
          localMax = temp;
          localMin = highTreshold - 5*peak;
          detectedBetweenThresholds = false;          
          while(temp > lowThreshold)    // start at highThreshhold and at lowThreshold
          {
            if (temp > localMax)
            {
              localMax = temp;
            }
            if ( temp < localMin)
            {
              detectedBetweenThresholds = true;
              localMin = temp;
            }
            if (detectedBetweenThresholds && temp > highTreshold )
            {
               detectedBetweenThresholds = false;
               mySerial.print("localMin ");
               mySerial.println(localMin);
               if (localMin < (lowThreshold + highTreshold)/2 ) 
               {
                 lowThreshold = localMin+peak;
                 mySerial.print("RESET lowThreshold to ");
                 mySerial.println(lowThreshold);
               }
               else
               {
//                 highTreshold = (localMin+highTreshold)/2;
  //               mySerial.print("mitigate highTreshold to ");
    //             mySerial.println(highTreshold);                 
               }
             
               localMin = highTreshold - 5*peak;
            }
            temp = analogRead(SensorPin);
          } // while(temp > lowThreshold)
          if (localMax < minOfMaximumValue)
          {
            mySerial.print("minOfMaximumValue ");
            mySerial.print(minOfMaximumValue);
            minOfMaximumValue = localMax;          
            mySerial.print(" -> ");
            mySerial.print(minOfMaximumValue);
            highTreshold = minOfMaximumValue-peak;
            mySerial.print(" highTreshold ");
            mySerial.println(highTreshold);
          }
          
          ///************************************************************************
          mySerial.print("*");
          digitalWrite(LedPin,HIGH);
          temp = analogRead(SensorPin);
          localMin = temp;
          detectedBetweenThresholds = false;          
          localMax = lowThreshold + peak*5;
          while(temp < highTreshold)
          {
            if (temp < localMin)
            {
              localMin = temp;
            }  // if (temp < localMin)
            if (temp > localMax)
            {
              detectedBetweenThresholds = true;
              localMax = temp;
//              mySerial.print("-");
            }
            if (detectedBetweenThresholds && temp < lowThreshold )
            {
                detectedBetweenThresholds = false;
               mySerial.print("localMax ");
               mySerial.print(localMax);
               highTreshold = localMax;
               mySerial.print(" RESET ****** highTreshold to ");
               mySerial.println(highTreshold);
               localMax = lowThreshold + 5*peak;
            }
            temp = analogRead(SensorPin);
          } // while(temp < highTreshold)
          if (localMin > maxOfMinumumValue)
          {
            mySerial.print("maxOfMinumumValue ");
            mySerial.print(maxOfMinumumValue);
            maxOfMinumumValue = localMin;
            mySerial.print(" ---------> ");
            mySerial.print(maxOfMinumumValue);
            lowThreshold = maxOfMinumumValue+peak; //(lowThreshold + maxOfMinumumValue)/2;
            mySerial.print(" lowThreshold ");
            mySerial.println(lowThreshold);
          }          
          
          
          if (minMaxRange != minOfMaximumValue - maxOfMinumumValue )
          {
            minMaxRange = minOfMaximumValue - maxOfMinumumValue;
            display_number(minMaxRange);
            mySerial.print("range ");
            mySerial.println(minMaxRange);
          }
    } //    while(! digitalRead(RecordPin)
      
    mySerial.print("minOfMaximumValue ");
    mySerial.println(minOfMaximumValue);
    mySerial.print("maxOfMinumumValue ");
    mySerial.println(maxOfMinumumValue);
    mySerial.print("peakHigh - peakLow = ");
    mySerial.print(peakHigh);    
    mySerial.print(" - ");
    mySerial.print(peakLow);
    mySerial.print(" = ");
    mySerial.println(peakHigh - peakLow);
    mySerial.print("threshold ");
    mySerial.println(highTreshold);
    mySerial.print("lowThreshold ");
    mySerial.println(lowThreshold);
=======
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
  
>>>>>>> fc2b9bd2864f67812bf1dac5dea2fea84377bd31
 
    EEPROM_writeAnything(0, highTreshold);
    EEPROM_writeAnything(10, lowThreshold);
  
<<<<<<< HEAD
  } // if (digitalRead(RecordPin))
=======
  } //end digitalRead


>>>>>>> fc2b9bd2864f67812bf1dac5dea2fea84377bd31

  display_number(counter);

} //end setup

// **********************************************************
void loop(){
// **********************************************************  
  if (!LightDetected && analogRead(SensorPin) < lowThreshold)
  {
    digitalWrite(LedPin,HIGH);
    LightDetected = true;
    delay(10);
  }
  if ( LightDetected && analogRead(SensorPin) > highTreshold)
  {
    digitalWrite(LedPin,LOW);
    LightDetected = false;
    counter++;
    if (counter==10000)
      counter = 0;
    display_number(counter);
    delay(2);
<<<<<<< HEAD
    mySerial.println(counter);
=======
#ifdef USE_SERIAL
    Serial.println(counter);
 //       Serial.println("       ");

#endif

>>>>>>> fc2b9bd2864f67812bf1dac5dea2fea84377bd31
  }
} // void loop(){

