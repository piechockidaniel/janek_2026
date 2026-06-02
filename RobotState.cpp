#include "RobotState.h"

namespace robot {

Mode RobotState::update(const Inputs& in) {
    // ---- Highest priority: failsafe conditions ----
    // Sensor channel is the most critical: if we have no idea what's around
    // us we cannot safely move. Trip immediately.
    // if (in.millisSinceLastSensor > cfg_.sensorTimeoutMs) {
    //     mode_ = Mode::Failsafe;
    //     failsafeReason_ = FailsafeReason::SensorTimeout;
    //     return mode_;
    // }

    // Command timeout only matters when we are actively moving under operator
    // control. Autonomous mode is self-driving so it is exempt.
    const bool needsCommand =
        (mode_ == Mode::ManualDrive || mode_ == Mode::Alert);
    if (needsCommand && in.millisSinceLastCommand > cfg_.commandTimeoutMs) {
        mode_ = Mode::Failsafe;
        failsafeReason_ = FailsafeReason::CommandTimeout;
        return mode_;
    }

    // ---- Operator-initiated transitions ----
    switch (in.command) {
        case Command::Reset:
            // Reset only clears Failsafe; from any other state it's a no-op.
            if (mode_ == Mode::Failsafe) {
                mode_ = Mode::Idle;
                failsafeReason_ = FailsafeReason::None;
            }
            return mode_;

        case Command::Stop:
            mode_ = Mode::Idle;
            return mode_;

        case Command::Drive:
            if (mode_ != Mode::Failsafe) {
                mode_ = in.panic ? Mode::Alert : Mode::ManualDrive;
            }
            return mode_;

        case Command::Explore:
            if (mode_ != Mode::Failsafe) {
                mode_ = in.panic ? Mode::Avoiding : Mode::Autonomous;
            }
            return mode_;

        case Command::None:
        default:
            break; // fall through to sensor-driven refinement
    }

    // ---- Sensor-driven refinement (no new command) ----
    switch (mode_) {
        case Mode::ManualDrive:
            if (in.panic) mode_ = Mode::Alert;
            break;
        case Mode::Alert:
            if (!in.panic) mode_ = Mode::ManualDrive;
            break;
        case Mode::Autonomous:
            if (in.panic) mode_ = Mode::Avoiding;
            break;
        case Mode::Avoiding:
            if (!in.panic) mode_ = Mode::Autonomous;
            break;
        case Mode::Idle:
        case Mode::Failsafe:
            // Stable states - no auto-transition.
            break;
    }
    return mode_;
}

const char* toString(Mode m) {
    switch (m) {
        case Mode::Idle:        return "Idle";
        case Mode::ManualDrive: return "ManualDrive";
        case Mode::Autonomous:  return "Autonomous";
        case Mode::Avoiding:    return "Avoiding";
        case Mode::Alert:       return "Alert";
        case Mode::Failsafe:    return "Failsafe";
    }
    return "?";
}

const char* toString(Command c) {
    switch (c) {
        case Command::None:    return "None";
        case Command::Stop:    return "Stop";
        case Command::Drive:   return "Drive";
        case Command::Explore: return "Explore";
        case Command::Reset:   return "Reset";
    }
    return "?";
}

const char* toString(FailsafeReason r) {
    switch (r) {
        case FailsafeReason::None:           return "None";
        case FailsafeReason::SensorTimeout:  return "SensorTimeout";
        case FailsafeReason::CommandTimeout: return "CommandTimeout";
    }
    return "?";
}

} // namespace robot
