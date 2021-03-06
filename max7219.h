#define SPI1_write(X) SPI.transfer(X)

void max7219_init1() { 
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(0x09);      // BCD mode for digit decoding 
  SPI1_write(0x0F); 
  digitalWrite(chipSelectPin, HIGH);
 
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(0x0A); 
  SPI1_write(0x0F);      // Segment luminosity intensity 
  digitalWrite(chipSelectPin, HIGH);
 
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(0x0B); 
  SPI1_write(0x03);      // Display refresh 0x07
  digitalWrite(chipSelectPin, HIGH);
 
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(0x0F); 
  SPI1_write(0x00);      // No test 
  digitalWrite(chipSelectPin, HIGH);
 
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(0x0C); 
  SPI1_write(0x01);      // Turn on the display 
  digitalWrite(chipSelectPin, HIGH);

} 
 
void display_number(unsigned int data1) { 

  digitalWrite(chipSelectPin, LOW);
  SPI1_write(4);                 // send Thousands digit 
  if (data1>999)
    SPI1_write(data1/1000); 
  else
    SPI1_write(0x0F);
  digitalWrite(chipSelectPin, HIGH);
  
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(3);                 // send Hundreds digit 
  if (data1>99)
    SPI1_write((data1/100)%10); 
  else
    SPI1_write(0x0F);
  digitalWrite(chipSelectPin, HIGH);
  
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(2);                 // send Tens digit 
  if (data1>9)
    SPI1_write((data1/10)%10); 
  else
    SPI1_write(0x0F);
  digitalWrite(chipSelectPin, HIGH);
  
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(1);                 // send Ones digit 
  SPI1_write(data1%10); 
  digitalWrite(chipSelectPin, HIGH);
} 

void turn_off(unsigned long delayTime)
{
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(0x0C); 
  SPI1_write(0x00);      // Turn off the display 
  digitalWrite(chipSelectPin, HIGH);

  delay(delayTime);

  digitalWrite(chipSelectPin, LOW);
  SPI1_write(0x0C); 
  SPI1_write(0x01);      // Turn on the display 
  digitalWrite(chipSelectPin, HIGH);
}
 
void display_increment_number(unsigned int data1) { 

  if (data1%10 != 0)
  {
    digitalWrite(chipSelectPin, LOW);
    SPI1_write(1);                 // send Ones digit 
    SPI1_write(data1%10); 
    digitalWrite(chipSelectPin, HIGH);
    return;
  }
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(1);                 // send Ones digit as 0
  SPI1_write(0); 
  digitalWrite(chipSelectPin, HIGH);
    
  if ((data1/10)%10 != 0)
  {
    digitalWrite(chipSelectPin, LOW);
    SPI1_write(2);                 // send Tens digit 
    SPI1_write((data1/10)%10);    
    digitalWrite(chipSelectPin, HIGH);
    return;   
  }
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(2);                 // send Tens digit 
  SPI1_write(0);    
  digitalWrite(chipSelectPin, HIGH);
  
  if ((data1/100)%10 != 0)
  {
    digitalWrite(chipSelectPin, LOW);
    SPI1_write(3);                 // send Hundreds digit 
    SPI1_write((data1/100)%10);
    digitalWrite(chipSelectPin, HIGH);
    return;
  }  
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(3);                 // send Hundreds digit 
  SPI1_write(0);
  digitalWrite(chipSelectPin, HIGH);
      
  digitalWrite(chipSelectPin, LOW);
  SPI1_write(4);                 // send Thousands digit 
  SPI1_write(data1/1000); 
  digitalWrite(chipSelectPin, HIGH);
}
