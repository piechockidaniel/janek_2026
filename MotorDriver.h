#pragma once
#include <Arduino.h>
#include "Geometry.h"

// Drives one TA6586 H-bridge channel (one motor pair on the chassis).
//
// TA6586 logic:
//   IN1=1  IN2=0  -> forward
//   IN1=0  IN2=1  -> reverse
//   IN1=0  IN2=0  -> coast (high impedance)
//   IN1=1  IN2=1  -> active brake (avoid: chip-specific behavior)
//
// We apply PWM to whichever pin is "active" for the current direction
// and hold the other LOW. This gives clean speed control with predictable
// coasting at zero throttle.
class MotorDriver {
public:
    // pinForward / pinReverse must BOTH be PWM-capable.
    // On Arduino UNO R4 Minima/WiFi those are D3, D5, D6, D9, D10, D11.
    MotorDriver(uint8_t pinForward, uint8_t pinReverse);

    // Initialize pins to OUTPUT LOW. Call once in setup().
    void begin();

    // Apply a signed PWM command in the range [-255..255].
    // Positive = forward, negative = reverse, 0 = coast.
    // Values are clamped automatically; caller may pass already-clamped
    // output from MotorMixer.
    void apply(int16_t signedPwm);

    // Immediately drive both pins LOW. Use from failsafe paths.
    void coast();

private:
    uint8_t pinFwd_;
    uint8_t pinRev_;
    int16_t lastApplied_ = 0;
};
