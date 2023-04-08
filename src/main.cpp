#include <mbed.h>

#include "pixy2.h"
#include "Serial.h"

struct Vector
{
  void print()
  {
    char buf[64];
    sprintf(buf, "vector: (%d %d) (%d %d) index: %d flags %d", m_x0, m_y0, m_x1, m_y1, m_index, m_flags);
    printf("%s \r\n", buf);
  }

  uint8_t m_x0;
  uint8_t m_y0;
  uint8_t m_x1;
  uint8_t m_y1;
  uint8_t m_index;
  uint8_t m_flags;
};

// Vector *getLine(uint8_t *receivedData, uint8_t offset)
// {
//   uint8_t new_arr[6];
//   for (int i = 0; i < 6; i++)
//   {
//     new_arr[i] = receivedData[i + offset];
//   }
//   return (Vector *)new_arr;
// }

// Configure the I2C pins
I2C i2c(D14, D15); // sda, scl
const int addr = 0x54 << 1;

int main()
{
  wait(5); // wait for the camera to boot up
  i2c.frequency(100000);

  // uint8_t data[] = {0xae, 0xc1, 0x0e, 0x00}; // get version
  // // uint8_t data[] = {174, 193, 22, 2, 1, 0}; // lamp
  // // uint8_t data[] = {174, 193, 24, 0}; // get fps
  // i2c.write(addr, (const char *)data, sizeof(data));

  // uint8_t receivedData[10];
  // i2c.read(addr, (char *)receivedData, sizeof(receivedData));
  // for (size_t i = 0; i < sizeof(receivedData); i++)
  // {
  //   printf("%02x ", receivedData[i]);
  // }
  // printf("\n");

  while (1)
  {
    uint8_t data[] = {174, 193, 48, 2, 1, 1}; // get main features
    i2c.write(addr, (const char *)data, sizeof(data));
    uint8_t receivedData[40];
    i2c.read(addr, (char *)receivedData, sizeof(receivedData));
    for (size_t i = 0; i < sizeof(receivedData); i++)
    {
      printf("%02x ", receivedData[i]);
    }
    printf("\r\n");

    // 6 bytes per line

    uint8_t packetType = receivedData[2];
    uint8_t payloadLength = receivedData[3];

    if (packetType != 49)
    {
      printf("Invalid data received\r\n");
      continue;
    }
    // no line detected
    if (payloadLength == 0)
    {
      printf("No line detected!\r\n");
      continue;
    }

    uint8_t numberOfLines = receivedData[7] / 6;

    if (numberOfLines == 1)
    {
      printf("1 line\r\n");
      // turn right or left

      // max y = 51
      // max x = 78

      uint8_t halfX = 39;
      uint8_t halfY = 25;

      uint8_t new_arr[6];
      for (int i = 0; i < 6; i++)
      {
        new_arr[i] = receivedData[i + 8];
      }

      Vector *v1 = (Vector *)new_arr;
      v1->print();

      // check if it's on the left or right side
      if (v1->m_x0 > halfX)
      {
        // right side
        printf("RIGHT SIDE\r\n");
      }
      else
      {
        // left side
        printf("LEFT SIDE\r\n");
      }
    }
    else if (numberOfLines == 2)
    {
      printf("2 lines\r\n");
      uint8_t new_arr[6];

      for (int i = 0; i < 6; i++)
      {
        new_arr[i] = receivedData[i + 8];
      }
      Vector *v1 = (Vector *)new_arr;
      v1->print();

      for (int i = 0; i < 6; i++)
      {
        new_arr[i] = receivedData[i + 14];
      }
      Vector *v2 = (Vector *)new_arr;
      v2->print();
    }
    else
    {
      printf("a lot\r\n");
    }

    // uint8_t new_arr[6];
    // for (int i = 0; i < 6; i++)
    // {
    //   new_arr[i] = receivedData[i + 8];
    // }

    // Vector *v1 = (Vector *)new_arr;
    // v1->print();

    // for (int i = 0; i < 6; i++)
    // {
    //   new_arr[i] = receivedData[i + 14];
    // }
    // Vector *v2 = (Vector *)new_arr;
    // v2->print();

    wait(1);
  }
}

/* get fps */
// uint8_t data[] = {174, 193, 24, 0}; // get fps
// i2c.write(addr, (const char *)data, sizeof(data));

// uint8_t receivedData[10];
// // Receive data over I2C
// i2c.read(addr, (char *)receivedData, sizeof(receivedData));

// int32_t value;
// memcpy(&value, &receivedData[sizeof(receivedData) - 4], sizeof(value));

// printf("%d FPS\r\n", value);