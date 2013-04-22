//
// Kayak Electronics www.kayak-electronics.com
// 
// Double Digital Tachograph 
// Januar 2013.

// commands to run on Ubuntu for visualisation of sensor analog readings
// http://www.lysium.de/blog/index.php?/archives/234-Plotting-data-with-gnuplot-in-real-time.html
// screen -L /dev/ttyACM0
// cat /dev/ttyACM0 | tee sensors.log | ./driveGnuPlotStreams.pl 4 2 60 60 0 1100 0 1100 400x600+0+0 400x600+0+0 'sensorValue1' 'state1' 'senorValue2' 'state2' 0 0 1 1

#include <SPI.h>
#include "max7219.h"

//#define chipSelectPin 10
//#define SPI1_write(X) SPI.transfer(X)
unsigned int cnt; 
/*
// black&white sensor
#define SENSOR1 A2
#define SENSOR1_LEVEL 30
#define SENSOR1_PERIOD 4

// line of sight sensor
#define SENSOR2 A3
#define SENSOR2_LEVEL 600
#define SENSOR2_PERIOD 4

// cylinder size
#define PERIMETER 0.474

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); //lcd(7, 6, 5, 4, 3, 2);
// initialize the library with the numbers of the interface pins

int counter1 = 0;        // here we store main counter
int newState1;           // state of a sensor (LOW/HIGH)
int lastState1 = LOW;    // temp variable used for debaunce (previous state)
int lastSensorReading1;  // variable used for sampling value
long lastSensorReadingTime1; // variable used for sampling time

int counter2 = 0;       // here we store main counter
int newState2;          // state of a sensor (LOW/HIGH)
int lastState2 = LOW;   // temp variable used for debaunce (previous state)
int lastSensorReading2; // variable used for sampling value
long lastSensorReadingTime2;// variable used for sampling time

long lastDebounceTime1 = 0;  // the last time the output pin was toggled
long lastDebounceTime2 = 0;  // the last time the output pin was toggled
long debounceDelay = 20;    // the debounce time; increase if the output flickers

int lastInterval[2];         // interval between last two detections
unsigned long lastMillis[2]; // last time it was detected
int missedDetection[2];    // number of missedDetection

// print value on Lcd with right align
// set printDecimalPlaces to true if you want to print two decimal places
void printLcdRightAlign( float value, int col, int row, int printDecimalPlaces = false);
*/

void setup() { 
 
//  Chip_Select_Direction = 0;     // Set RC0 pin as output 
  pinMode(chipSelectPin, OUTPUT);
//  SPI1_init();                   // Initialize SPI module 
   SPI.begin();

  max7219_init1();               // initialize  max7219 
 
  for (cnt=0; cnt<10000; cnt++) { 
    display_number(cnt); 
    delay(50);
  //  Delay_ms(1000); 
  } 
 
 /*lcd.begin(16, 2);
 printLcdRightAlign( counter1, 6, 0); // value, col, row
 printLcdRightAlign( counter1 * PERIMETER , 15, 0, true ); // value, col, row, printDecimalPlaces
 printLcdRightAlign( counter2, 6, 1); // value, col, row
 printLcdRightAlign( counter2 * PERIMETER , 15, 1, true);
 Serial.begin(9600);
 pinMode(SENSOR1, INPUT);
 pinMode(SENSOR2, INPUT);
 */
}

void loop(){
//  readSensor1();
//  readSensor2();
}
/*
void readSensor1() 
{
  int sensorReading = analogRead(SENSOR1);  
  // sampling every 100ms for big difference of sensorReading and lastSensorReading
  // sampling every 500ms for small fluctuation or steady state
  if (((abs(sensorReading - lastSensorReading1)>10) && (millis()-lastSensorReadingTime1>100)) ||
          (millis() - lastSensorReadingTime1> 500)) {
    lastSensorReading1 = sensorReading ;
    lastSensorReadingTime1 = millis();
    // 0: is sensorReading
    Serial.print( "0:");
    Serial.println( sensorReading);
    // 1: is actual state show on graph as high or low
    if (newState1 == LOW)
      Serial.println( "1:20");
    else
      Serial.println( "1:700");
  }
  
  int reading;  
  if (sensorReading < SENSOR1_LEVEL) 
    reading = LOW;
  else
    reading = HIGH; //when there is no detection

  if (reading != lastState1)
  {
    // reset the debouncing timer 
    lastDebounceTime1 = millis(); 
  }
   
  if ((millis()-lastDebounceTime1) > debounceDelay) 
  {
    if (reading != newState1) 
    {
      newState1 = reading;
      if (newState1 == LOW)
      {
        counter1++;
        printLcdRightAlign( counter1, 6, 0);
        printLcdRightAlign( counter1 * PERIMETER , 15, 0, true);
        checkDiff( SENSOR1 - SENSOR1 );       
      }  
    }
  }
  lastState1 = reading;
}

void readSensor2() 
{
  int sensorReading = analogRead(SENSOR2);  
  // sampling every 100ms for big difference
  // sampling every 500ms for small fluctuation or steady state
  if (((abs(sensorReading - lastSensorReading2)>10)  && (millis()-lastSensorReadingTime1>100)) ||
      (millis() - lastSensorReadingTime2> 500)) {
    lastSensorReading2 = sensorReading ;
    lastSensorReadingTime2 = millis();
    // 2: is sensorReading
    Serial.print( "2:");
    Serial.println( sensorReading);
    // 3: is actual state show on graph as high or low
    if (newState2 == LOW)
      Serial.println( "3:230");
    else
      Serial.println( "3:1030");
  }
  
  int reading;  
  if (sensorReading < SENSOR2_LEVEL) 
    reading = LOW;
  else
    reading = HIGH; //when there is no detection


  if (reading != lastState2)
  {
    // reset the debouncing timer 
    lastDebounceTime2 = millis(); 
  }
   
  if ((millis()-lastDebounceTime2) > debounceDelay) 
  {
    if (reading != newState2) 
    {
      newState2 = reading;
      if (newState2 == LOW)
      {
        counter2++;        
        printLcdRightAlign( counter2, 6, 1);
        printLcdRightAlign( counter2 * PERIMETER , 15, 1, true);
        checkDiff( SENSOR2  - SENSOR1);
      }  
    }
  }
  lastState2 = reading;
}

void printLcdRightAlign( float value, int col, int row, int printDecimalPlaces )
{
  col -= (int) log10 (value);
  if ( printDecimalPlaces == true )
  {    
    col -= 3;
    lcd.setCursor( col, row );
    lcd.print( value );
  }
  else 
  {
    // value is float so we are typecasting to int
    lcd.setCursor( col, row );
    lcd.print( (long) value);
  }
}

void checkDiff(int sensorID)
{  
  // if there is any difference
  if ( (abs( (float)counter1 / SENSOR1_PERIOD - (float)counter2 / SENSOR2_PERIOD) > (float)1/4) )
  {
    if (((millis() - lastMillis[sensorID]) > 1.5 * lastInterval[sensorID] ) || 
         ( (millis() - lastMillis[sensorID]) < 0.5 * lastInterval[sensorID] ))
    {
      // sensorID is causing that difference
      lcd.setCursor( 0, sensorID ); //col,row
      lcd.print( ++missedDetection[sensorID]);
    }
    else
    {
      if (((millis() - lastMillis[!sensorID]) > 1.5 * lastInterval[!sensorID] ) || 
         ( (millis() - lastMillis[!sensorID]) < 0.5 * lastInterval[!sensorID] ))
      {
        // another sensor is causing that difference
        lcd.setCursor( 0, !sensorID ); //col,row
        lcd.print( ++missedDetection[!sensorID]);      
      }
      else
      {
        // unknown source
        lcd.setCursor( 7, sensorID ); //col,row
        lcd.print( "*" );              
      }
    }
  }
  else 
  {
      lcd.setCursor( 0, 0 ); //col,row
      lcd.print( "  ");          
      lcd.setCursor( 7, 0 ); //col,row
      lcd.print( "  ");              
      lcd.setCursor( 0, 1 ); //col,row
      lcd.print( "  ");
      lcd.setCursor( 7, 1 ); //col,row
      lcd.print( "  ");              
      missedDetection[0] = 0;      
      missedDetection[1] = 0;
  }
  lastInterval[sensorID] = millis() - lastMillis[sensorID];
  lastMillis[sensorID] = millis();
}
*/
