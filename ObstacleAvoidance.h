#pragma once
#include "Geometry.h"

namespace robot {

// Pure-function obstacle avoidance policy.
//
// Given the four sensor distances and the operator's desired velocity,
// returns the velocity that should actually be applied. The policy is
// conservative: it only restricts motion, never amplifies or initiates it.
//
// Behaviour:
//   - If desired motion is forward and front < stopThreshold -> stop.
//   - If desired motion is forward and front < slowThreshold -> scale down.
//   - If desired motion is backward and back < stopThreshold -> stop.
//   - Sideways clearance does not block motion but biases turning away
//     from obstacles when in autonomous explore mode (see chooseExplore()).
class ObstacleAvoidance {
public:
    struct Config {
        uint16_t stopThresholdCm  = 15;
        uint16_t slowThresholdCm  = 40;
        uint16_t panicThresholdCm = 10;  // triggers buzzer alert
    };

    ObstacleAvoidance() : cfg_() {}
    explicit ObstacleAvoidance(const Config& cfg) : cfg_(cfg) {}

    // Filter an operator-driven velocity through the safety policy.
    Velocity filter(const Velocity& desired, const Distances& d) const;

    // Choose a velocity autonomously, based only on sensor data.
    // Returns forward when path is clear, turns toward the most-open side
    // when blocked, backs up when boxed in.
    Velocity chooseExplore(const Distances& d) const;

    // Returns true when any forward-facing sensor is below panic threshold.
    bool panic(const Distances& d) const;

    const Config& config() const { return cfg_; }

private:
    Config cfg_;
};

} // namespace robot
