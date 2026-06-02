#pragma once
#include "Geometry.h"

namespace robot {

// Skid-steer (tank-style) differential drive mixer.
//
// Converts a normalized (linear, angular) velocity command into signed PWM
// values for the left and right motor pairs. Output is normalized so that
// neither side ever exceeds maxPwm in magnitude, preserving the *ratio*
// between the two sides (critical for predictable turning).
class MotorMixer {
public:
    // maxPwm caps the absolute PWM value applied to a motor.
    // We default to 200/255 (~78%) to stay safely below the motors'
    // ~6V rating when fed from a 7.4V 2S 18650 pack.
    static constexpr int16_t DEFAULT_MAX_PWM = 200;

    static MotorOutput mix(const Velocity& v, int16_t maxPwm = DEFAULT_MAX_PWM);
};

} // namespace robot
