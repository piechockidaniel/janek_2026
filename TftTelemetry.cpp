#include "TftTelemetry.h"
#include <SPI.h>

namespace {
constexpr uint16_t BG     = 0x0000;  // black
constexpr uint16_t FG     = 0xFFFF;  // white
constexpr uint16_t DIM    = 0x52AA;  // gray
constexpr uint16_t OK     = 0x07E0;  // green
constexpr uint16_t WARN   = 0xFD20;  // orange
constexpr uint16_t ALERT  = 0xF800;  // red
constexpr uint16_t INFO   = 0x07FF;  // cyan

uint16_t colorForDistance(uint16_t cm) {
    if (cm <= 15)  return ALERT;
    if (cm <= 40)  return WARN;
    return OK;
}

uint16_t colorForMode(robot::Mode m) {
    switch (m) {
        case robot::Mode::Idle:        return DIM;
        case robot::Mode::ManualDrive: return OK;
        case robot::Mode::Autonomous:  return INFO;
        case robot::Mode::Avoiding:    return WARN;
        case robot::Mode::Alert:       return WARN;
        case robot::Mode::Failsafe:    return ALERT;
    }
    return FG;
}
}

TftTelemetry::TftTelemetry(Pins pins)
    : tft_(pins.cs, pins.dc, pins.rst) {}

void TftTelemetry::begin() {
    tft_.initR(INITR_BLACKTAB);  // V1.1 ST7735 module
    tft_.setRotation(1);          // landscape: 160 wide, 128 tall
    tft_.fillScreen(BG);
    renderInitial();
}

void TftTelemetry::renderInitial() {
    tft_.fillScreen(BG);
    drawLabels();
    firstRender_ = true;
}

void TftTelemetry::drawLabels() {
    tft_.setTextColor(DIM, BG);
    tft_.setTextSize(1);

    tft_.setCursor(2, 2);    tft_.print("MODE");
    tft_.setCursor(110, 2);  tft_.print("WIFI");

    tft_.setCursor(2, 28);   tft_.print("FRONT");
    tft_.setCursor(2, 44);   tft_.print("BACK ");
    tft_.setCursor(2, 60);   tft_.print("LEFT ");
    tft_.setCursor(2, 76);   tft_.print("RIGHT");

    tft_.setCursor(2, 100);  tft_.print("BATT");
    tft_.setCursor(80, 100); tft_.print("L");
    tft_.setCursor(110, 100);tft_.print("R");

    tft_.drawFastHLine(0, 22, 160, DIM);
    tft_.drawFastHLine(0, 96, 160, DIM);
}

void TftTelemetry::drawMode(robot::Mode m) {
    tft_.fillRect(40, 0, 60, 12, BG);
    tft_.setTextColor(colorForMode(m), BG);
    tft_.setTextSize(1);
    tft_.setCursor(40, 2);
    tft_.print(toString(m));
}

void TftTelemetry::drawValue(int16_t x, int16_t y, uint16_t value, uint16_t color) {
    tft_.fillRect(x, y, 60, 12, BG);
    tft_.setTextColor(color, BG);
    tft_.setTextSize(1);
    tft_.setCursor(x, y);
    if (value >= robot::Distances::OUT_OF_RANGE) {
        tft_.print("---");
    } else {
        tft_.print(value);
        tft_.print(" cm");
    }
}

void TftTelemetry::drawBattery(float volts) {
    tft_.fillRect(30, 100, 50, 12, BG);
    uint16_t color = volts < 6.4f ? ALERT : (volts < 7.0f ? WARN : OK);
    tft_.setTextColor(color, BG);
    tft_.setTextSize(1);
    tft_.setCursor(30, 100);
    tft_.print(volts, 2);
    tft_.print("V");
}

void TftTelemetry::drawWifi(bool connected) {
    tft_.fillRect(140, 0, 20, 12, BG);
    tft_.setTextColor(connected ? OK : ALERT, BG);
    tft_.setTextSize(1);
    tft_.setCursor(140, 2);
    tft_.print(connected ? "ON " : "OFF");
}

void TftTelemetry::update(robot::Mode mode,
                          const robot::Distances& d,
                          float batteryVolts,
                          int16_t leftPwm,
                          int16_t rightPwm,
                          bool wifiConnected) {
    if (firstRender_ || mode != lastMode_) {
        drawMode(mode); lastMode_ = mode;
    }
    if (firstRender_ || d.front != lastFront_) {
        drawValue(50, 28, d.front, colorForDistance(d.front)); lastFront_ = d.front;
    }
    if (firstRender_ || d.back  != lastBack_) {
        drawValue(50, 44, d.back,  colorForDistance(d.back));  lastBack_  = d.back;
    }
    if (firstRender_ || d.left  != lastLeft_) {
        drawValue(50, 60, d.left,  colorForDistance(d.left));  lastLeft_  = d.left;
    }
    if (firstRender_ || d.right != lastRight_) {
        drawValue(50, 76, d.right, colorForDistance(d.right)); lastRight_ = d.right;
    }

    const int16_t battCenti = (int16_t)(batteryVolts * 100.0f);
    if (firstRender_ || battCenti != lastBattCenti_) {
        drawBattery(batteryVolts); lastBattCenti_ = battCenti;
    }

    if (firstRender_ || leftPwm != lastLeftPwm_) {
        tft_.fillRect(82, 112, 26, 10, BG);
        tft_.setTextColor(FG, BG);
        tft_.setCursor(82, 112);
        tft_.print(leftPwm);
        lastLeftPwm_ = leftPwm;
    }
    if (firstRender_ || rightPwm != lastRightPwm_) {
        tft_.fillRect(112, 112, 26, 10, BG);
        tft_.setTextColor(FG, BG);
        tft_.setCursor(112, 112);
        tft_.print(rightPwm);
        lastRightPwm_ = rightPwm;
    }

    if (firstRender_ || wifiConnected != lastWifi_) {
        drawWifi(wifiConnected); lastWifi_ = wifiConnected;
    }

    firstRender_ = false;
}
