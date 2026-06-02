#pragma once
#include <Arduino.h>
#include <Arduino_LED_Matrix.h>
#include "MoodSelector.h"

// Renders a Mood as an 8x12 pixel face on the UNO R4 WiFi's onboard
// LED matrix. The matrix is internally connected via charlieplexing on
// the RA4M1 - no GPIO is consumed.
//
// The faces are intentionally simple so a kid can recognize them at a
// glance. Each is a 12-byte array where bytes 0..7 are rows top to
// bottom, bits 0..11 left to right (only 12 LSBs used).
class MoodMatrix {
public:
    MoodMatrix();
    void begin();
    void show(robot::Mood mood);
    robot::Mood current() const { return current_; }

private:
    ArduinoLEDMatrix matrix_;
    robot::Mood      current_ = robot::Mood::Sleepy;
    bool             initialized_ = false;
};
