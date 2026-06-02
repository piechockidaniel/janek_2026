#pragma once
#include <cstdint>
#include "Geometry.h"

namespace robot {

enum class Mode : uint8_t {
    Idle,            // motors off, awaiting commands
    ManualDrive,     // following operator velocity commands
    Autonomous,      // exploring with obstacle avoidance
    Avoiding,        // autonomous + currently maneuvering around an obstacle
    Alert,           // operator commanded motion, but obstacle blocks it
    Failsafe         // watchdog tripped or hard error - motors locked off
};

// When mode_ is Failsafe, this enum records WHY the watchdog tripped,
// to aid debugging without a serial monitor attached.
enum class FailsafeReason : uint8_t {
    None,            // not in failsafe, or cleared by reset
    SensorTimeout,   // millisSinceLastSensor exceeded cfg_.sensorTimeoutMs
    CommandTimeout   // moving manually and millisSinceLastCommand exceeded cfg_.commandTimeoutMs
};

// Inputs to the state machine that can come from the operator (web UI).
enum class Command : uint8_t {
    None,
    Stop,            // -> Idle
    Drive,           // -> ManualDrive (carries velocity payload separately)
    Explore,         // -> Autonomous
    Reset            // -> Idle (clears Failsafe)
};

// State machine. Pure function of (currentMode, command, sensors, time).
// No timers, no globals - feed it deltaMillis from outside.
class RobotState {
public:
    struct Inputs {
        Command   command;
        Distances distances;
        bool      panic;
        uint32_t  millisSinceLastCommand;
        uint32_t  millisSinceLastSensor;
    };

    struct Config {
        // If no command arrives within this many ms while moving, fail safe.
        uint32_t commandTimeoutMs = 1000;
        // If sensor reads stop arriving for this long, fail safe.
        uint32_t sensorTimeoutMs  = 500;
    };

    RobotState() : cfg_() {}
    explicit RobotState(const Config& cfg) : cfg_(cfg) {}

    // Advance the state machine. Returns the new mode.
    Mode update(const Inputs& in);

    Mode current() const { return mode_; }

    // The reason for the most recent Failsafe transition. Cleared by Reset.
    FailsafeReason failsafeReason() const { return failsafeReason_; }

    // For tests / inspection
    const Config& config() const { return cfg_; }

private:
    Mode           mode_           = Mode::Idle;
    FailsafeReason failsafeReason_ = FailsafeReason::None;
    Config         cfg_;
};

const char* toString(Mode m);
const char* toString(Command c);
const char* toString(FailsafeReason r);

} // namespace robot
