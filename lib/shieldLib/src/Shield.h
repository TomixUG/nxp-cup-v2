#pragma once

class Shield
{
public:
    Shield();
    void init();
    void setServo(float input);
    void setMotors(float pwrL, float pwrR);

    void setD1(bool on);
    void setD2(bool on);
    void setD3(bool on);
    void setD4(bool on);

    float getPot1();
    float getPot2();

    bool getSwA();
    bool getSwB();
    bool getSw1();
    bool getSw2();
    bool getSw3();
    bool getSw4();
};