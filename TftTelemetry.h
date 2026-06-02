#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "Geometry.h"
#include "RobotState.h"

// Renders robot telemetry on a 128x160 ST7735 1.8" SPI display.
//
// Only changed regions are redrawn each frame -> no flicker, no full
// repaint of the screen, and the main loop stays responsive even
// while the screen is "updating".
class TftTelemetry {
public:
    struct Pins {
        uint8_t cs;
        uint8_t dc;
        uint8_t rst;
    };

    explicit TftTelemetry(Pins pins);

    void begin();
    void renderInitial();
    void update(robot::Mode mode,
                const robot::Distances& d,
                float batteryVolts,
                int16_t leftPwm,
                int16_t rightPwm,
                bool wifiConnected);

private:
    Adafruit_ST7735 tft_;

    // Cached values so we only redraw when something changes.
    robot::Mode lastMode_       = robot::Mode::Idle;
    uint16_t    lastFront_      = 0xFFFF;
    uint16_t    lastBack_       = 0xFFFF;
    uint16_t    lastLeft_       = 0xFFFF;
    uint16_t    lastRight_      = 0xFFFF;
    int16_t     lastBattCenti_  = -9999;  // in centi-volts to avoid float compare
    int16_t     lastLeftPwm_    = 0;
    int16_t     lastRightPwm_   = 0;
    bool        lastWifi_       = false;
    bool        firstRender_    = true;

    void drawLabels();
    void drawValue(int16_t x, int16_t y, uint16_t value, uint16_t color);
    void drawMode(robot::Mode m);
    void drawBattery(float volts);
    void drawWifi(bool connected);
};
