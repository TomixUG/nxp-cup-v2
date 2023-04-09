#include "mbed.h"
#include "Shield.h"

#include "TSL1401CL.h"

// servo
#include "Servo.h"
Servo myservo(PTD0);

// cam
TSL1401CL cam(PTC7, PTC5, PTC0);
// motors
PwmOut MA1(PTC1);
PwmOut MB1(PTC3);
PwmOut MA2(PTC2);
PwmOut MB2(PTC4);
DigitalIn M_ERR(PTD3);
DigitalOut M_EN(PTD2);
// leds
DigitalOut d1(PTC8);
DigitalOut d2(PTC9);
DigitalOut d3(PTC10);
DigitalOut d4(PTC11);
// pots
AnalogIn pot1(PTB11);
AnalogIn pot2(PTB10);
// switches and buttons
DigitalIn swA(PTB20);
DigitalIn swB(PTB23);
DigitalIn sw1(PTB2);
DigitalIn sw2(PTB3);
DigitalIn sw3(PTB9);
DigitalIn sw4(PTB19);

Shield::Shield() {}

void Shield::init()
{
    // camera
    cam.setIntegrationTime(80);
    while (!cam.integrationReady())
    {
        printf("integration not ready");
    }
    // motors
    M_EN = 1;
    // MA1.period_us(1000);  MB1.period_us(1000);  MA2.period_us(1000);  MB2.period_us(1000);
}

int *Shield::getCamData()
{
    return cam.getData();
}

void Shield::setServo(float input)
{
    // servo range from 0.36 to 0.64
    // input range from -100 to 100
    //   if (input > 100) input = 100;
    //   if (input < -100) input = -100;
    // float k = (100 - (-100)) / (0.64 - 0.36); = 714.2857143
    // float q = (k * 0.64) - 100; = 357.1428572

    //   float result = (input + 357.1428572) / 714.2857143;

    //   printf("Calculated result: %g\r\n", result);
    // myservo = result;
    myservo = input;
}

void Shield::setMotors(float pwrL, float pwrR)
{
    // input valid check
    if (pwrL < -100.0)
        pwrL = -100.0;
    if (pwrL > 100.0)
        pwrL = 100.0;
    // input valid check
    if (pwrR < -100.0)
        pwrR = -100.0;
    if (pwrR > 100.0)
        pwrR = 100.0;

    if (pwrL < 0)
    {
        MA1 = pwrL / -100.0;
        MA2 = 0;
    }
    else if (pwrL > 0)
    {
        MA1 = 0;
        MA2 = pwrL / 100.0;
    }
    else
    {
        MA1 = 0;
        MA2 = 0;
    }

    if (pwrR < 0)
    {
        MB1 = 0;
        MB2 = pwrR / -100.0;
    }
    else if (pwrR > 0)
    {
        MB1 = pwrR / 100.0;
        MB2 = 0;
    }
    else
    {
        MB1 = 0;
        MB2 = 0;
    }
}

void Shield::setD1(bool on)
{
    d1 = on;
}
void Shield::setD2(bool on)
{
    d2 = on;
}
void Shield::setD3(bool on)
{
    d3 = on;
}
void Shield::setD4(bool on)
{
    d4 = on;
}

float Shield::getPot1()
{
    return pot1.read();
}
float Shield::getPot2()
{
    return pot2.read();
}

bool Shield::getSwA()
{
    return swA.read();
}
bool Shield::getSwB()
{
    return swB.read();
}
bool Shield::getSw1()
{
    return sw1.read();
}
bool Shield::getSw2()
{
    return sw2.read();
}
bool Shield::getSw3()
{
    return sw3.read();
}
bool Shield::getSw4()
{
    return sw4.read();
}