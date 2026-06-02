#include "MotorDriver.h"

MotorDriver::MotorDriver(uint8_t pinForward, uint8_t pinReverse)
    : pinFwd_(pinForward), pinRev_(pinReverse) {}

void MotorDriver::begin() {
    pinMode(pinFwd_, OUTPUT);
    pinMode(pinRev_, OUTPUT);
    digitalWrite(pinFwd_, LOW);
    digitalWrite(pinRev_, LOW);
}

void MotorDriver::apply(int16_t signedPwm) {
    // Clamp to safe PWM range.
    if (signedPwm >  255) signedPwm =  255;
    if (signedPwm < -255) signedPwm = -255;

    if (signedPwm == 0) {
        coast();
    } else if (signedPwm > 0) {
        analogWrite(pinFwd_, signedPwm);
        digitalWrite(pinRev_, LOW);
    } else {
        digitalWrite(pinFwd_, LOW);
        analogWrite(pinRev_, -signedPwm);
    }
    lastApplied_ = signedPwm;
}

void MotorDriver::coast() {
    digitalWrite(pinFwd_, LOW);
    digitalWrite(pinRev_, LOW);
    lastApplied_ = 0;
}
