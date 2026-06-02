// All Arduino pin assignments live here so wiring changes touch one file.
// Reference: docs/wiring.md
//
// NOTE: namespace is named RoboPins (not "pins") to avoid a collision with
// a symbol exported by the Renesas RA Arduino core for the UNO R4.
#pragma once
#include <Arduino.h>

namespace RoboPins {

// ---- Drivetrain (2x TA6586) ----
constexpr uint8_t MOTOR_L_FWD = 3;   // PWM
constexpr uint8_t MOTOR_L_REV = 5;   // PWM
constexpr uint8_t MOTOR_R_FWD = 6;   // PWM
constexpr uint8_t MOTOR_R_REV = 9;   // PWM

// ---- HC-SR04 array (shared trigger, four echoes) ----
constexpr uint8_t ULTRA_TRIGGER  = 2;
constexpr uint8_t ULTRA_ECHO_F   = 4;
constexpr uint8_t ULTRA_ECHO_B   = 7;
constexpr uint8_t ULTRA_ECHO_L   = 12;
constexpr uint8_t ULTRA_ECHO_R   = A1;

// ---- TFT 1.8" ST7735 (hardware SPI: SCK=D13, MOSI=D11) ----
constexpr uint8_t TFT_DC  = 8;
constexpr uint8_t TFT_CS  = 10;
constexpr uint8_t TFT_RST = A3;

// ---- Buzzer SFM-27 ----
constexpr uint8_t BUZZER  = A2;

// ---- Analog sensing ----
constexpr uint8_t BATT_SENSE = A0;   // through 2:1 divider

} // namespace RoboPins
