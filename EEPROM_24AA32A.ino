//Arduino test sketch for Microchip EEPROM 24AA32A 32 Kbit
//The device is organized as a single block of 4096 x 8-bit
//The 24AA32A also has a page write capability for up to 32 bytes of data

//A0, A1, A2 and WP are LOW
//eeprom address B01010000

#include <Wire.h>
#define EEPROM_ADDRESS B01010000

uint8_t readData(uint8_t eeprom_address, unsigned int byte_address, uint8_t &data_byte)
{
  uint8_t result;
  Wire.beginTransmission(eeprom_address);
  
  //write Address High Byte
  uint8_t size = Wire.write(byte_address >> 8);
  //write Address Low Byte
  if (Wire.write(byte_address & 0xFF) == 0)
    size = 0;
    
  if (size == 0)
    result = 10;  //Failed to set address
  else
  {
    result = Wire.endTransmission(0);
    if (result == 0)
    {
      size = Wire.requestFrom(eeprom_address, (uint8_t)1, (uint8_t)1);//address, bytes_to_read, stop_after_request
      if (size == 0)
        result = 12;  //no bytes returned from the slave device
      else
      { 
        if (Wire.available() > 0)  //number of bytes available to read
        {
          data_byte = Wire.read();
          result = 0;  //success  
        }
        else
          result = 11;  //no bytes to read
      }
    }
  }
  return result;

 /* 
  *  return values
  *  0 = success
  *  1 = data too long to fit in buffer
  *  2 = receive NAK when transmiting address
  *  3 = receive NAK when transmiting data
  *  4 = other error
  *  10 = failed to set address
  *  11 = no bytes to read
  *  12 = no bytes returned from the slave device
  */
}

uint8_t writeData(uint8_t eeprom_address, unsigned int byte_address, uint8_t data_byte)
{
  uint8_t result;
  Wire.beginTransmission(eeprom_address);

  //write Address High Byte
  uint8_t size = Wire.write(byte_address >> 8);
  //write Address Low Byte
  if (Wire.write(byte_address & 0xFF) == 0)
    size = 0;
    
  if (size == 0) 
    result = 10;   //Failed to set address
  else
  {
    size = Wire.write(data_byte);
    if (size == 0)
    { 
      result = 11;     //Failed to write data
      Wire.endTransmission(true);
    }
    else
      result = Wire.endTransmission(true);
  }
  return result;
  
 /* 
  *  return values
  *  0 = success
  *  1 = data too long to fit in buffer
  *  2 = receive NAK when transmiting address
  *  3 = receive NAK when transmiting data
  *  4 = other error
  *  10 = failed to set address
  *  11 = failed to write data
  */
}

//page write up to 30 bytes data to eeprom
//Wire buffer = 32 Byte - 2 Byte Address = 30 Byte
//It is only possible to write in 32-byte blocks.
//E.g. 0 - 31, 32 - 63, 64 - 95... so 32*n - (32*n+31).
//It is not possible to write over the limit of the block.
//When that happens, the beginning of the block will be overwritten.
uint8_t pageWriteData(uint8_t eeprom_address, unsigned int byte_address, uint8_t quantity, uint8_t *buffer)
{
  uint8_t result;  
  Wire.beginTransmission(eeprom_address);

  //write Address High Byte
  uint8_t size = Wire.write(byte_address >> 8);
  //write Address Low Byte
  if (Wire.write(byte_address & 0xFF) == 0)
    size = 0;
  
  if (size == 0) 
    result = 10;   //Failed to set address
  else
  {
    size = Wire.write(buffer, quantity);
    if (size == 0)
    { 
      result = 11;     //Failed to write data
      Wire.endTransmission(true);
    }
    else
      result = Wire.endTransmission(true);
  }
  return result;
  
 /* 
  *  return values
  *  0 = success
  *  1 = data too long to fit in buffer
  *  2 = receive NAK when transmiting address
  *  3 = receive NAK when transmiting data
  *  4 = other error
  *  10 = failed to set address
  *  11 = failed to write data
  */
}

//sequential read of up to 30 Bytes
//eeprom allows you to read the whole memory, but the Wire library only has a 32-byte buffer
uint8_t readDataSequential(uint8_t eeprom_address, unsigned int byte_address, uint8_t quantity, uint8_t *data_bytes)
{
  uint8_t result;
  uint8_t stop_after_request = 1;
  Wire.beginTransmission(eeprom_address);
  
  //write Address High Byte
  uint8_t size = Wire.write(byte_address >> 8);
  //write Address Low Byte
  if (Wire.write(byte_address & 0xFF) == 0)
    size = 0;
    
  if (size == 0)
    result = 10;  //Failed to set address
  else
  {
    result = Wire.endTransmission(false);
    if (result == 0)
    {
      size = Wire.requestFrom(eeprom_address, quantity, stop_after_request);  //address, bytes_to_read, stop_after_request
      if (size == 0)
        result = 12;  //no bytes returned from the slave device
      else
      { 
        if (Wire.available() == quantity)  //number of bytes available to read
        {
          for (int byte_count = 0; byte_count < quantity; byte_count++)
            data_bytes[byte_count] = Wire.read();
          result = 0;  //success  
        }
        else
          result = 11;  //no bytes to read
      }
    }
  }
  return result;

 /* 
  *  return values
  *  0 = success
  *  1 = data too long to fit in buffer
  *  2 = receive NAK when transmiting address
  *  3 = receive NAK when transmiting data
  *  4 = other error
  *  10 = failed to set address
  *  11 = no bytes to read
  *  12 = no bytes returned from the slave device
  */
}



void setup()
{  
  
  Serial.begin(9600);
  delay(1000);//wait for serial monitor init
  
  Wire.begin();
  
  unsigned int byte_address = 0;
  uint8_t data_byte_w = 0;
  uint8_t data_byte_r = 255;
  uint8_t result=255;
  uint8_t buffer_r[30];
  int quantity_r = 30;
  uint8_t buffer_w[30];
  int quantity_w = 30;

  //init write array
  for (int byte_count = 0; byte_count < 30; byte_count++)
  {
    buffer_w[byte_count] = 2;
  }

  //init read array
  for (int byte_count = 0; byte_count < 30; byte_count++)
  {
    buffer_r[byte_count] = 255;
  }
 /*
  for (int byte_count = 0; byte_count < 0; byte_count++)
  {
    if(byte_count<2048)
      result=writeDataByte(EEPROM_ADDRESS, byte_count, byte_count/8);
    else
      result=writeDataByte(EEPROM_ADDRESS, byte_count, 256-byte_count/8);
    delay(5);   //wait until write is completed
  }*/
  result = pageWriteData(EEPROM_ADDRESS, byte_address, quantity_w, buffer_w);
  delay(5); //wait until write is completed
  
  //read whole memory
  for (int byte_count = 0; byte_count < 4096; byte_count++)
  {
    readData(EEPROM_ADDRESS, byte_count, data_byte_r);
    Serial.print(byte_count); Serial.print(" = ");Serial.println(data_byte_r);    
  }
 
  readDataSequential(EEPROM_ADDRESS, byte_address, quantity_r, buffer_r);
  for (int byte_count = 0; byte_count < quantity_r; byte_count++)
  {
    Serial.println(buffer_r[byte_count]);
  }
}

void loop()
{
}
