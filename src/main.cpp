#include <mbed.h>
#include <cmath>
#include "Serial.h"

#include "pixy2.h"
#include "Shield.h"

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
Vector getLine(const uint8_t *receivedData, int index)
{
  Vector v;
  v.m_x0 = receivedData[index + 0];
  v.m_y0 = receivedData[index + 1];
  v.m_x1 = receivedData[index + 2];
  v.m_y1 = receivedData[index + 3];
  v.m_index = receivedData[index + 4];
  v.m_flags = receivedData[index + 5];
  return v;
}
float map(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

Shield shield;

// Configure the I2C pins
I2C i2c(D14, D15); // sda, scl
const int addr = 0x54 << 1;

const uint8_t searchWhere = 25; // the Y value of finding the point

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

      Vector v1 = getLine(receivedData, 8);
      v1.print();

      // check if it's on the left or right side
      if (v1.m_x0 > halfX)
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

      Vector v1 = getLine(receivedData, 8);
      v1.print();

      Vector v2 = getLine(receivedData, 14);
      v2.print();

      // calculate midpoint

      // calculate X location on the line on fixed height
      float p1 = v1.m_x0 + (searchWhere - v1.m_y0) * (v1.m_x1 - v1.m_x0) / (float)(v1.m_y1 - v1.m_y0);
      float p2 = v2.m_x0 + (searchWhere - v2.m_y0) * (v2.m_x1 - v2.m_x0) / (float)(v2.m_y1 - v2.m_y0);

      int steerPoint = round((p1 + p2) / 2);

      float result = map(steerPoint, 0, 78, 0.36, 0.64);

      shield.setServo(result);

      printf("1The point loc: %f\r\n", p1);
      printf("2The point loc: %f\r\n", p2);
      printf("Steer point: %d\r\n", steerPoint);
      printf("Servo: %f\r\n", result);
    }
    else
    {
      printf("a lot\r\n");
    }

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