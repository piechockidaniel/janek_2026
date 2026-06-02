#include "MotorMixer.h"

namespace robot {

namespace {
    constexpr float clampf(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }
    constexpr float absf(float v) { return v < 0 ? -v : v; }
}

MotorOutput MotorMixer::mix(const Velocity& v, int16_t maxPwm) {
    // 1. Clamp inputs to [-1, +1]. Garbage in must not yield garbage out.
    const float linear  = clampf(v.linear,  -1.0f, 1.0f);
    const float angular = clampf(v.angular, -1.0f, 1.0f);

    // 2. Differential mix.
    //    Positive angular -> CCW rotation (left wheels slow, right wheels fast).
    float left  = linear - angular;
    float right = linear + angular;

    // 3. Normalize so the larger magnitude side rides at exactly 1.0 (or less).
    //    Without this, a request for full-forward + full-turn would saturate
    //    one side while leaving headroom on the other, losing the turn.
    const float maxMag = absf(left) > absf(right) ? absf(left) : absf(right);
    if (maxMag > 1.0f) {
        left  /= maxMag;
        right /= maxMag;
    }

    // 4. Scale to PWM range. Cast preserves sign for direction.
    return {
        static_cast<int16_t>(left  * static_cast<float>(maxPwm)),
        static_cast<int16_t>(right * static_cast<float>(maxPwm))
    };
}

} // namespace robot
