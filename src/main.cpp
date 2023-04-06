#include <mbed.h>

#include "pixy2.h"
#include "Serial.h"

// Configure the I2C pins
I2C i2c(D14, D15); // sda, scl

int main()
{
  wait(5); // wait for the camera to boot up
  i2c.frequency(100000);
  const int addr = 0x54 << 1;

  uint8_t data[] = {0xae, 0xc1, 0x0e, 0x00}; // get version
  // uint8_t data[] = {174, 193, 22, 2, 1, 0}; // lamp
  // uint8_t data[] = {174, 193, 24, 0}; // get fps
  i2c.write(addr, (const char *)data, sizeof(data));

  uint8_t receivedData[10];
  i2c.read(addr, (char *)receivedData, sizeof(receivedData));
  for (size_t i = 0; i < sizeof(receivedData); i++)
  {
    printf("%02x ", receivedData[i]);
  }
  printf("\n");

  while (1)
  {
    uint8_t data[] = {174, 193, 24, 0}; // get fps
    i2c.write(addr, (const char *)data, sizeof(data));

    uint8_t receivedData[10];
    // Receive data over I2C
    i2c.read(addr, (char *)receivedData, sizeof(receivedData));

    int32_t value;
    memcpy(&value, &receivedData[sizeof(receivedData) - 4], sizeof(value));

    printf("%d FPS\r\n", value);

    wait(1); // wait for the camera to boot up
  }
}

// uint8_t data[] = {174, 193, 24, 0}; // get fps
// i2c.write(addr, (const char *)data, sizeof(data));

// uint8_t receivedData[10];
// // Receive data over I2C
// i2c.read(addr, (char *)receivedData, sizeof(receivedData));

// int32_t value;
// memcpy(&value, &receivedData[sizeof(receivedData) - 4], sizeof(value));

// printf("%d FPS\r\n", value);