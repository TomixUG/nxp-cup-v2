#include <mbed.h>
#include <cmath>
#include "Serial.h"

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

Vector vectors[30];

const uint8_t searchWhere = 25; // the Y value of finding the point
const int centerConst = 34;

int main()
{
  // wait(5); // wait for the camera to boot up

  shield.init();
  i2c.frequency(100000);

  // uint8_t data[] = {0xae, 0xc1, 0x0e, 0x00}; // get version
  // // uint8_t data[] = {174, 193, 24, 0}; // get fps

  if (shield.getSw4() == true)
  {
    uint8_t data[] = {174, 193, 22, 2, 1, 1}; // lamp
    i2c.write(addr, (const char *)data, sizeof(data));

    uint8_t receivedData[10];
    i2c.read(addr, (char *)receivedData, sizeof(receivedData));
    for (size_t i = 0; i < sizeof(receivedData); i++)
    {
      printf("%02x ", receivedData[i]);
    }
    printf("\n");
  }

  while (1)
  {
    float resultTotal = 0;
    int howManyAverage = 5;
    for (int i = 0; i < howManyAverage; i++)
    {
      uint8_t data[] = {174, 193, 48, 2, 1, 1}; // get main features
      i2c.write(addr, (const char *)data, sizeof(data));
      uint8_t receivedData[40];
      i2c.read(addr, (char *)receivedData, sizeof(receivedData));
      // for (size_t i = 0; i < sizeof(receivedData); i++)
      // {
      //   printf("%02x ", receivedData[i]);
      // }
      // printf("\r\n");

      // 6 bytes per line

      uint8_t packetType = receivedData[2];
      uint8_t payloadLength = receivedData[3];

      if (packetType != 49)
      {
        // printf("Invalid data received\r\n");
        i -= 1;
        continue;
      }
      // no line detected
      if (payloadLength == 0)
      {
        // printf("No line detected!\r\n");
        shield.setServo(0.5);
        i -= 1;
        continue;
      }

      uint8_t numberOfVectors = receivedData[7] / 6;
      for (int i = 0; i < numberOfVectors; i++)
      {
        int offset = i * 6 + 8; // Calculate the byte offset for this line in the payload
        vectors[i] = getLine(receivedData, offset);
      }

      int rightSideAmount = 0;
      int rightSideTotal = 0;

      int leftSideAmount = 0;
      int leftSideTotal = 0;

      for (int i = 0; i < numberOfVectors; i++)
      {
        // Do something with vectors[i]
        // vectors[i].print();
        if (vectors[i].m_x0 > centerConst)
        {
          // right side
          // printf("right side\r\n");
          rightSideTotal += vectors[i].m_x0;
          rightSideTotal += vectors[i].m_x1;

          rightSideAmount += 2;
        }
        else
        {
          // left side
          // printf("left side\r\n");
          leftSideTotal += vectors[i].m_x0;
          leftSideTotal += vectors[i].m_x1;

          leftSideAmount += 2;
        }
      }

      if (rightSideAmount == 0)
      {
        // turn right
        rightSideTotal = 78;
        rightSideAmount = 1;
      }

      // turn corrections TODO: junk code hhh
      if ((rightSideTotal / rightSideAmount) < 46)
      {
        rightSideTotal = 35;
        rightSideAmount = 1;
      }

      if ((leftSideTotal / leftSideAmount) > (78 - 50))
      {
        leftSideTotal = (78 - 50);
        leftSideAmount = 1;
      }

      float centerPoint = ((rightSideTotal / rightSideAmount) + (leftSideTotal / leftSideAmount)) / 2;
      // printf("Center: %f\r\n", centerPoint);

      // calculate the error and map it to servo
      float x = centerPoint - centerConst;
      float result = map(x, 15, -15, 0.36, 0.64);

      resultTotal += result;
    }

    float averagedResult = resultTotal / howManyAverage;

    float servoResult = round(averagedResult * 100.0) / 100.0;
    // printf("Servo: %f \r\n", servoResult);
    shield.setServo(servoResult);

    if (shield.getSw1() == true)
    {
      // if (servoResult < 0.4)

      // {
      //   shield.setMotors(18, 18);
      // }
      // else if (servoResult > 0.54)
      // {
      //   shield.setMotors(18, 18);
      // }
      // else
      // {
      shield.setMotors(20, 20);
      // }
    }
    else
    {

      shield.setMotors(0, 0);
    }

    // wait(0.5);
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