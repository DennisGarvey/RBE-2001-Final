#include <Arduino.h>
#include "ClampMotor.h"
#include <Romi32U4.h>

unsigned const int LUTTimeout = 1000,
          LUTThreshold = 8;
int lastUpdatedPosition = 0,
    lastTarget = -1;
unsigned long lastUpdatedTime = 0;

ClampMotor::ClampMotor()
{
}

void ClampMotor::setup()
{
    servo.attach();
    pinMode(18, INPUT);
}

long ClampMotor::getPosition()
{
    return analogRead(0);
}

void ClampMotor::reset()
{
    servo.detach();
    lastUpdatedPosition = getPosition();
    lastUpdatedTime = millis();
}

void ClampMotor::setEffortWithDeadband(int effort)
{
    if(effort != 0) {
        effort = constrain(effort, -600, 600);
        effort = effort * ((600.0f - Deadband) / 600.0f);
        if(effort > 0) effort += Deadband;
        else           effort -= Deadband;
    }
    setEffort(effort);
}


void ClampMotor::setEffort(int effort)
{
    if(effort == 0) servo.detach();
    else {
        effort = constrain(effort, -600, 600);
        servo.writeMicroseconds(effort + neutral);
    }
}

byte ClampMotor::moveTo(int target)
{
    if(target != lastTarget) {
        lastTarget = target;
        lastUpdatedPosition = getPosition();
        lastUpdatedTime = millis();
    }

    target = constrain(target, 0, ADCMAX);
    int pos = getPosition();
    int deltaP = target - pos;
    /*Serial.print(deltaP);
    Serial.print("\t");
    Serial.println(deltaP * kp);*/
    if(abs(deltaP) < tolerance || (deltaP * kp > 0 && pos == 0) || (deltaP * kp < 0 && pos > 1020)) {
        setEffort(0);
        return 1;
    } else if(abs(lastUpdatedPosition - pos) > LUTThreshold) {
        lastUpdatedPosition = pos;
        if(deltaP > 0) lastUpdatedPosition -= LUTThreshold / 2;
        else lastUpdatedPosition += LUTThreshold / 2;
        lastUpdatedTime = millis();
        /*Serial.print(lastUpdatedPosition);
        Serial.print("\t");
        Serial.print(target);
        Serial.print("\t");
        Serial.println(pos);*/
    }
    else if(lastUpdatedTime + LUTTimeout < millis()) {
        //Serial.print("Hit at ");
        //Serial.println(pos);
        return 2;
    }
    setEffortWithDeadband(deltaP * kp);
    return 0;
}