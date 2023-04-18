#include <mbed.h>
#include <cmath>
#include "Serial.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <tuple>

#include "Shield.h"

// struct Vector
// {
//   void print()
//   {
//     char buf[64];
//     sprintf(buf, "vector: (%d %d) (%d %d) index: %d flags %d", m_x0, m_y0, m_x1, m_y1, m_index, m_flags);
//     printf("%s \r\n", buf);
//   }
//   uint8_t m_x0;
//   uint8_t m_y0;
//   uint8_t m_x1;
//   uint8_t m_y1;
//   uint8_t m_index;
//   uint8_t m_flags;
// };

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

std::pair<bool, int> detect_edge(const std::vector<int> &diff, int threshold)
{
  auto max_iter = std::max_element(diff.begin(), diff.end());
  bool found = *max_iter > threshold;
  int index = found ? std::distance(diff.begin(), max_iter) : 0;
  return {found, index};
}

std::tuple<bool, bool, int, int> detect_edges(const std::vector<int> &pixels)
{
  const int n = pixels.size();

  // calculate differences
  std::vector<int> diff(n - 1);
  for (int i = 0; i < n - 1; ++i)
  {
    diff[i] = std::abs(pixels[i + 1] - pixels[i]);
  }

  int mid_point = n / 2;
  std::vector<int> diff_left(diff.begin(), diff.begin() + mid_point);
  std::vector<int> diff_right(diff.begin() + mid_point, diff.end());

  // int threshold = std::accumulate(diff.begin(), diff.end(), 0) / diff.size();
  auto max_diff = std::max_element(diff.begin(), diff.end()); // max diff
  // no lane detection:
  // auto max_raw = std::max_element(pixels.begin(), pixels.end()); // max from raw data
  // printf("%f\r\n", (double)*max_diff / (double)*max_raw);

  int threshold = *max_diff * 0.8; // calculate the threshold from the biggest difference

  auto [line_left, line_left_idx] = detect_edge(diff_left, threshold);
  auto [line_right, line_right_idx] = detect_edge(diff_right, threshold);

  if (line_right)
  {
    line_right_idx += mid_point;
  }

  return {line_left, line_right, line_left_idx, line_right_idx};
}

Shield shield;

// const uint8_t searchWhere = 25; // the Y value of finding the point
const int centerConst = 62;

int main()
{
  shield.init();

  if (shield.getSw4() == true)
  {
    /* browser viewer */
    while (1)
    {

      int *val = shield.getCamData();

      int diff[127];
      for (int i = 1; i < 127; ++i)
      {
        diff[i] = abs(val[i] - val[i + 1]);
      }

      for (int i = 1; i < 127; i++)
      {
        printf("%d;", diff[i]);
      }
      printf("\r\n");
    }
  }

  while (1)
  {
    // averaging
    int howManyTimes = 10;
    int total = 0;
    for (int i = 0; i < howManyTimes; i++)
    {
      int *val = shield.getCamData();
      // Convert the C-style array to a std::vector<int>
      std::vector<int> pixels(val, val + 128);
      auto [line_left, line_right, line_left_idx, line_right_idx] = detect_edges(pixels);

      // printf("Line on left side: %s, Index: %d\r\n", line_left ? "true" : "false", line_left_idx);
      // printf("Line on right side: %s, Index: %d\r\n", line_right ? "true" : "false", line_right_idx);

      // if we lose one of the 2 lines, calculate one
      if (line_right == false)
      {
        // line_right_idx = 127;
        line_right_idx = line_left_idx + 80; // h
      }
      // h
      if (line_left == false)
      {
        line_left_idx = line_right_idx - 80;
      }

      float centerPoint = ((line_left_idx + line_right_idx) / 2);
      total += centerPoint;
    }

    float avgCenterPoint = total / howManyTimes;

    float error = avgCenterPoint - centerConst;
    float result = map(error, 15, -15, 0.36, 0.64);

    float servoResult = round(result * 100.0) / 100.0;
    // printf("Servo: %f        %f\r\n", servoResult, avgCenterPoint);
    shield.setServo(servoResult);

    if (shield.getSw1() == true)
    {
      shield.setMotors(10, 10);
    }
    else
    {
      shield.setMotors(0, 0);
    }
  }

  // while (1)
  // {
  //   float resultTotal = 0;
  //   int howManyAverage = 5;
  //   for (int i = 0; i < howManyAverage; i++)
  //   {
  //     // no line detected
  //     // if (payloadLength == 0)
  //     // {
  //     //   printf("No line detected!\r\n");
  //     //   i -= 1;
  //     //   continue;
  //     // }

  //     int rightSideAmount = 0;
  //     int rightSideTotal = 0;

  //     int leftSideAmount = 0;
  //     int leftSideTotal = 0;

  //     for (int i = 0; i < numberOfVectors; i++)
  //     {
  //       // Do something with vectors[i]
  //       // vectors[i].print();
  //       if (vectors[i].m_x0 > centerConst)
  //       {
  //         // right side
  //         // printf("right side\r\n");
  //         rightSideTotal += vectors[i].m_x0;
  //         rightSideTotal += vectors[i].m_x1;

  //         rightSideAmount += 2;
  //       }
  //       else
  //       {
  //         // left side
  //         // printf("left side\r\n");
  //         leftSideTotal += vectors[i].m_x0;
  //         leftSideTotal += vectors[i].m_x1;

  //         leftSideAmount += 2;
  //       }
  //     }

  //     if (rightSideAmount == 0)
  //     {
  //       // turn right
  //       rightSideTotal = 78;
  //       rightSideAmount = 1;
  //     }

  //     // turn corrections TODO: junk code hhh
  //     if ((rightSideTotal / rightSideAmount) < 40)
  //     {
  //       rightSideTotal = 40;
  //       rightSideAmount = 1;
  //     }

  //     if ((leftSideTotal / leftSideAmount) > (78 - 50))
  //     {
  //       leftSideTotal = (78 - 50);
  //       leftSideAmount = 1;
  //     }

  //     float centerPoint = ((rightSideTotal / rightSideAmount) + (leftSideTotal / leftSideAmount)) / 2;
  //     // printf("Center: %f\r\n", centerPoint);

  //     // calculate the error and map it to servo
  //     float x = centerPoint - centerConst;
  //     float result = map(x, 15, -15, 0.36, 0.64);

  //     resultTotal += result;
  //   }

  //   float averagedResult = resultTotal / howManyAverage;

  //   float servoResult = round(averagedResult * 100.0) / 100.0;
  //   // printf("Servo: %f \r\n", servoResult);
  //   shield.setServo(servoResult);

  //   if (shield.getSw1() == true)
  //   {
  //     shield.setMotors(20, 20);
  //   }
  //   else
  //   {

  //     shield.setMotors(0, 0);
  //   }

  //   // wait(0.5);
  // }
}
