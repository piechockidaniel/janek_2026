#pragma once
#include <cstdint>

namespace robot {

// Normalized velocity command. linear in [-1, +1] (forward positive),
// angular in [-1, +1] (counter-clockwise positive when viewed from above).
struct Velocity {
    float linear;
    float angular;

    static constexpr Velocity stopped()        { return {0.0f,  0.0f}; }
    static constexpr Velocity forward(float v) { return {v,     0.0f}; }
    static constexpr Velocity spinLeft(float w){ return {0.0f, +w}; }
    static constexpr Velocity spinRight(float w){return {0.0f, -w}; }
};

// Signed PWM command per side. Sign carries direction (forward = positive).
// Magnitude in [0, maxPwm] where maxPwm <= 255.
struct MotorOutput {
    int16_t left;
    int16_t right;

    static constexpr MotorOutput stopped() { return {0, 0}; }
};

// Distances from the four HC-SR04 sensors, in centimeters.
// A value of UINT16_MAX (or anything >= 400) means "no echo / out of range".
struct Distances {
    uint16_t front;
    uint16_t back;
    uint16_t left;
    uint16_t right;

    static constexpr uint16_t OUT_OF_RANGE = 400;

    bool frontClear(uint16_t threshold) const { return front  > threshold; }
    bool backClear (uint16_t threshold) const { return back   > threshold; }
    bool leftClear (uint16_t threshold) const { return left   > threshold; }
    bool rightClear(uint16_t threshold) const { return right  > threshold; }
};

} // namespace robot
