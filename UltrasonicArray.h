#pragma once
#include <Arduino.h>
#include "Geometry.h"

// Reads four HC-SR04 ultrasonic sensors sharing a single trigger line.
//
// Design notes:
//   - Shared trigger means all four sensors emit their 40 kHz ping at once.
//     This is safe ONLY when the four heads point in mutually different
//     directions (front/back/left/right). Cross-talk would otherwise corrupt
//     readings.
//   - tick() reads ONE sensor per call and rotates through all four. This
//     bounds the main-loop blocking time to one pulseIn() worst-case
//     (~30 ms) instead of four (~120 ms). Each individual distance is
//     refreshed roughly every 4 ticks.
//   - For >100 Hz update rates you would want pin-change interrupts.
//     This implementation prioritizes simplicity for a learning project.
class UltrasonicArray {
public:
    UltrasonicArray(uint8_t triggerPin,
                    uint8_t echoFront,
                    uint8_t echoBack,
                    uint8_t echoLeft,
                    uint8_t echoRight);

    void begin();

    // Read the next sensor in rotation. Call from main loop.
    // Returns true if this tick refreshed a reading.
    bool tick();

    // Snapshot of the most recent distances in centimeters.
    // OUT_OF_RANGE means no echo received within the timeout.
    robot::Distances distances() const { return latest_; }

    // millis() of the most recent successful read across all sensors.
    uint32_t lastSampleMillis() const { return lastSampleMs_; }

private:
    static constexpr uint32_t ECHO_TIMEOUT_US = 25000;  // ~4 meters max

    uint16_t measureCm(uint8_t echoPin) const;

    uint8_t          trigger_;
    uint8_t          echoPins_[4];   // front, back, left, right
    robot::Distances latest_;
    uint8_t          nextIndex_     = 0;
    uint32_t         lastSampleMs_  = 0;
};
