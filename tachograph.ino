
//
// Kayak Electronics www.kayak-electronics.com
// 
// Double Digital Tachograph 
// Januar 2013.
// TODO: analogReadResolution to 9 bits

#define chipSelectPin 10    // this is used in max7219.h
#define USE_SERIAL
//#define SAVE_SAMPLE
//#define SAVE_HIST

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

#ifdef SAVE_SAMPLE
// rm screenlog.0
// screen -L /dev/ttyACM0
#ifdef SAVE_HIST
    unsigned int hist[512];  // this reserve 512*2=1KB RAM
    for(int i=0;i<512;i++)
      hist[i]=0;
    int temp;
    
    
    do 
    {
      temp = analogRead(SensorPin);
       hist[temp/2]++;
    }
    while(hist[temp/2]<65535);
    for(int i=0;i<512;i++)
    {
       mySerial.println(hist[i]);
    }
    while(1);  
#else
  const int MAX = 900;
  int record[MAX], i;
//  while(1)
  //  mySerial.println(analogRead(SensorPin));

  while(1){
    int temp = analogRead(SensorPin);
    record[i++]= temp;
    if (i==MAX)
    {
      for(i=0;i<MAX;i++)
       mySerial.println(record[i]);
     i=0;
    }
  }
  
#endif //#ifdef SAVE_HIST
}
void loop(){
}
#else  //#ifdef SAVE_SAMPLE
  if (digitalRead(RecordPin))
  {
     mySerial.println("counter");
     EEPROM_readAnything(0, highTreshold);
     EEPROM_readAnything(10, lowThreshold);
  }
  else
  {
   mySerial.println("recording state");
  
    int maxOfMinumumValue = 0 ;    // greatest minim value when light is detected
    int minOfMaximumValue = 1024;       // lowest maximum value when there is no light
    int minMaxRange = 0; //  minOfMaximumValue - maxOfMinumumValue
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
          localMin = highTreshold - 3*peak;
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
               mySerial.print(localMin);
               if (localMin < (lowThreshold + highTreshold)/2 ) 
               {
                 display_number(localMin-maxOfMinumumValue);
                 maxOfMinumumValue = localMin;
                 lowThreshold = maxOfMinumumValue+2*peak;
                 mySerial.print(" RESET lowThreshold to ");
                 mySerial.println(lowThreshold);
               }
               else
               {
                   mySerial.println(" nothing");

//                 highTreshold = (localMin+highTreshold)/2;
  //               mySerial.print("mitigate highTreshold to ");
    //             mySerial.println(highTreshold);                 
               }
             
               localMin = highTreshold - 3*peak;
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
            highTreshold = minOfMaximumValue-2*peak;
            mySerial.print(" highTreshold ");
            mySerial.println(highTreshold);
          }
          
          ///************************************************************************
          mySerial.print("*");
          digitalWrite(LedPin,HIGH);
          temp = analogRead(SensorPin);
          localMin = temp;
          detectedBetweenThresholds = false;          
          localMax = lowThreshold + peak*3;
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
               display_number(localMax-minOfMaximumValue);
               detectedBetweenThresholds = false;
               mySerial.print("localMax ");
               mySerial.print(localMax);
               minOfMaximumValue = localMax;
               highTreshold = minOfMaximumValue-2*peak;
               mySerial.print(" RESET HHHHHHHHH highTreshold to ");
               mySerial.println(highTreshold);
               localMax = lowThreshold + 3*peak;
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
            lowThreshold = maxOfMinumumValue+2*peak; //(lowThreshold + maxOfMinumumValue)/2;
            mySerial.print(" lowThreshold ");
            mySerial.println(lowThreshold);
          }          
          
          
          if (minMaxRange != minOfMaximumValue - maxOfMinumumValue )
          {
            minMaxRange = minOfMaximumValue - maxOfMinumumValue;
            display_number(minMaxRange);
            mySerial.print("range ");
            mySerial.println(minMaxRange);
            if (minMaxRange < 3 * peak)
            {
               mySerial.print("low range, reseting!!!");
               maxOfMinumumValue = 0 ;
               minOfMaximumValue = 1024;
               minMaxRange = 0;
               shouldRestartCalculation = false;
               highTreshold = peakLow;
               lowThreshold = peakLow - 300;
               display_number(0);
            }
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
 
    EEPROM_writeAnything(0, highTreshold);
    EEPROM_writeAnything(10, lowThreshold);
  
  } // if (digitalRead(RecordPin))

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
    mySerial.println(counter);
  }
} // void loop(){
#endif  //#ifdef SAVE_SAMPLE

