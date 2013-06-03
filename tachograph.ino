
//
// Kayak Electronics www.kayak-electronics.com
// 
// Double Digital Tachograph 
// Januar 2013.
// TODO: analogReadResolution to 9 bits

#define USE_SERIAL
//#define SAVE_SAMPLE
//#define SAVE_HIST

#include <EEPROM.h>
#include "EEPROMAnything.h"

#include <SPI.h>
#define chipSelectPin 10    // this is used in max7219.h, also 12 and 13 pin+
#include "max7219.h"

#define SLOW_DOWN 10

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
const int FifthNumberPin = A1;


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
    
  while(1)
    mySerial.println(analogRead(SensorPin));

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

    int numberAnalog[5];
    numberAnalog[0] = analogRead(FirstNumberPin);
    numberAnalog[1] = analogRead(SecondNumberPin);
    numberAnalog[2] = analogRead(ThirdNumberPin);
    numberAnalog[3] = analogRead(FourthNumberPin);
    numberAnalog[4] = analogRead(FifthNumberPin);
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

    EEPROM_readAnything(0, highThreshold);
    EEPROM_readAnything(10, lowThreshold);
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
    int peakLowTemp = peakLow;
    mySerial.print("peak ");
    mySerial.println(peak);
    display_number(peak);
    
    minOfMaximumValue = peakHigh;
    maxOfMinimumValue = peakLow;
    bool wasBelow=false,wasAbove=false;
    for (int i = 0;i<6;)
    {
       int temp = analogRead(SensorPin);
       if (temp < maxOfMinimumValue - peak)
       {
         if (wasAbove)
         {
           i++;
           wasAbove = false;
           mySerial.println("above");
           minOfMaximumValue = (minOfMaximumValue + peakHigh) / 2;
         }
         if (temp<peakLow)
            peakLow=temp;
         wasBelow = true;
       }
       if (temp > minOfMaximumValue)
       {
         if (wasBelow )
         {
           i++;
           wasBelow = false;
           mySerial.println("below maxMin");
           maxOfMinimumValue = (maxOfMinimumValue + peakLow)/2;
         }
         if (temp>peakHigh)
            peakHigh = temp;
          wasAbove = true;
       }
    }

    mySerial.print("range ");
    mySerial.println(peakHigh - peakLow);
    
    int temp,localMax,  localMin ;
    bool detectedBetweenThresholds = false;
    minOfMaximumValue = peakLowTemp; //peakHigh;
    maxOfMinimumValue = peakLow;
    highThreshold = minOfMaximumValue - 20 ;
    lowThreshold = maxOfMinimumValue + 20;
    mySerial.print("highThreshold ");
    mySerial.println(highThreshold);
    mySerial.print("lowThreshold ");
    mySerial.println(lowThreshold);
    display_number(highThreshold - lowThreshold);
    while (! digitalRead(RecordPin))
    {
          //....................................................................
          mySerial.print(".");
          digitalWrite(LedPin,LOW);
          temp = analogRead(SensorPin);    // this is next at highThreshold
          localMax = temp;
          localMin = lowThreshold+5*(highThreshold-lowThreshold)/8;
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
            if (detectedBetweenThresholds && temp > highThreshold )
            {
               detectedBetweenThresholds = false;
               if (localMin < lowThreshold+3*(highThreshold-lowThreshold)/8 ) 
               {
                 mySerial.print("localMin ");
                 mySerial.print(localMin);
                 //display_number(localMin-maxOfMinimumValue);
                 maxOfMinimumValue = localMin;
                 lowThreshold = maxOfMinimumValue+20;//peak;
                 mySerial.print(" RESET lowThreshold to ");
                 mySerial.println(lowThreshold);
               }
               else
               {
                   mySerial.print("_");
               }
             
               localMin = lowThreshold+5*(highThreshold-lowThreshold)/8;
            }
            temp = analogRead(SensorPin);
          } // while(temp > lowThreshold)
          if (localMax < minOfMaximumValue)
          {
            //mySerial.print("minOfMaximumValue ");
            //mySerial.print(minOfMaximumValue);
            minOfMaximumValue = localMax;          
            mySerial.print(" -> ");
            mySerial.print(minOfMaximumValue);
            mySerial.print(" H ");
            mySerial.print(highThreshold);
            mySerial.print(" -> ");
            highThreshold = minOfMaximumValue-20;//peak;
            mySerial.println(highThreshold);
          }
          
          ///************************************************************************
          mySerial.print("*");
          digitalWrite(LedPin,HIGH);
          temp = analogRead(SensorPin);
          localMin = temp;
          detectedBetweenThresholds = false;          
          localMax = (lowThreshold + highThreshold)/2;
          while(temp < highThreshold)
          {
            if (temp < localMin)
            {
              localMin = temp;
            }  // if (temp < localMin)
            if (temp > localMax)
            {
              detectedBetweenThresholds = true;
              localMax = temp;
            }
            if (detectedBetweenThresholds && temp < lowThreshold )
            {
               //display_number(localMax-minOfMaximumValue);
               detectedBetweenThresholds = false;
               mySerial.print("localMax ");
               mySerial.print(localMax);
               minOfMaximumValue = localMax;
               highThreshold = minOfMaximumValue-20;
               mySerial.print(" RESET HHHHHHHHH highThreshold to ");
               mySerial.println(highThreshold);
               localMax = (lowThreshold + highThreshold)/2;
            }
            temp = analogRead(SensorPin);
          } // while(temp < highThreshold)
          if (localMin > maxOfMinimumValue)
          {
            //mySerial.print("maxOfMinimumValue ");
            //mySerial.print(maxOfMinimumValue);
            maxOfMinimumValue = localMin;
            //mySerial.print(" ---------> ");
            //mySerial.print(maxOfMinimumValue);
            mySerial.print(" L ");
            mySerial.print(lowThreshold);
            mySerial.print(" +> ");
            lowThreshold = maxOfMinimumValue+20;//peak; //(lowThreshold + maxOfMinimumValue)/2;
            mySerial.println(lowThreshold);
          }          
          
          counter++;
          if (minMaxRange != highThreshold - lowThreshold )
          {
            minMaxRange = highThreshold - lowThreshold;
            display_number(minMaxRange);
            //mySerial.print("range ");
            //mySerial.println(minMaxRange);
            if (minMaxRange < peak)
            {
               mySerial.print("low range, reseting!!!");
               display_number(peakHigh);
               turn_off(600);
               display_number(highThreshold);
               delay(2000);
               turn_off(600);
               display_number(lowThreshold);
               delay(2000);
               turn_off(600);
               display_number(peakLow);
               delay(2000);
               turn_off(600);
               minOfMaximumValue = peakLowTemp;//peakHigh;
               maxOfMinimumValue = peakLow;
               highThreshold = minOfMaximumValue - 20 ;
               lowThreshold = maxOfMinimumValue + 20;
               shouldRestartCalculation = false;
            }
          }
    } //    while(! digitalRead(RecordPin)
      
    mySerial.print("minOfMaximumValue ");
    mySerial.println(minOfMaximumValue);
    mySerial.print("maxOfMinimumValue ");
    mySerial.println(maxOfMinimumValue);
    mySerial.print("peakHigh - peakLow = ");
    mySerial.print(peakHigh);    
    mySerial.print(" - ");
    mySerial.print(peakLow);
    mySerial.print(" = ");
    mySerial.println(peakHigh - peakLow);
    mySerial.print("threshold ");
    mySerial.println(highThreshold);
    mySerial.print("lowThreshold ");
    mySerial.println(lowThreshold);
 
    EEPROM_writeAnything(0, highThreshold);
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
    delay(8);
  }
  if ( LightDetected && analogRead(SensorPin) > highThreshold)
  {
    digitalWrite(LedPin,LOW);
    LightDetected = false;
    counter++;
    if (counter==10000)
      counter = 0;
    if (!wasSecondCounter)
    {
      display_increment_number(counter);
      
      if (counter == targetCounter - SLOW_DOWN)
      {
         digitalWrite(SlowDownRelayPin,LOW);
      }
      if (counter == targetCounter)
      {
         digitalWrite(StopRelayPin,LOW);
         digitalWrite(SlowDownRelayPin,HIGH);
         delay(5000);
         digitalWrite(StopRelayPin,HIGH);
         secondCounter = displayedSecondCounter = 0;
         counter = 0;
         display_number(0);
      }
    }
    delay(5);
  }
  if (digitalRead(SecondDisplayPin))
  {
    if (wasSecondCounter)
    {
       if (secondCounter != displayedSecondCounter)
       {
          displayedSecondCounter = secondCounter;
          display_increment_number(displayedSecondCounter);
          if (displayedSecondCounter == targetCounter - SLOW_DOWN)
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
    }
    else
    {
      wasSecondCounter = true;
      displayedSecondCounter = secondCounter;
      display_number(displayedSecondCounter);
    }
  }
  else
  {
    if (wasSecondCounter)
    {
      wasSecondCounter = false;
      display_number(counter);
    }
  }
} // void loop(){
#endif  //#ifdef SAVE_SAMPLE

