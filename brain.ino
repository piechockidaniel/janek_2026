// =============================================================================
//  ROBO BRAIN  -  Arduino UNO R4 WiFi (or compatible clone)
//
//  Responsibilities:
//    * Read 4 HC-SR04 ultrasonic sensors (staggered).
//    * Run the RobotState machine.
//    * Apply MotorMixer output through 2 TA6586 H-bridges.
//    * Drive mood faces on the onboard 12x8 LED matrix.
//    * Render telemetry on a 1.8" ST7735 TFT.
//    * Drive the SFM-27 buzzer with non-blocking patterns.
//    * Serve a control web UI over WiFi (via the onboard ESP32-S3).
//    * Enforce watchdog failsafe if commands or sensors stall.
// =============================================================================

#include <Arduino.h>
#include <WiFiS3.h>
#include <ArduinoJson.h>

#include "Geometry.h"
#include "MotorMixer.h"
#include "ObstacleAvoidance.h"
#include "RobotState.h"
#include "MoodSelector.h"

#include "MotorDriver.h"
#include "UltrasonicArray.h"
#include "BuzzerDriver.h"
#include "MoodMatrix.h"
#include "TftTelemetry.h"

#include "pins.h"
#include "web_ui.h"
#include "secrets.h"

using namespace robot;

// ---- Tunables ----
constexpr int16_t  MAX_PWM            = 200;   // out of 255 -> ~78% duty
constexpr uint16_t SENSOR_TICK_MS     = 25;    // ~10 Hz per sensor (4 rotated)
constexpr uint16_t TFT_UPDATE_MS      = 200;   // 5 Hz display refresh
constexpr uint16_t TELEMETRY_MS       = 100;   // for status endpoint cache
constexpr uint16_t BATT_DIVIDER_R1_KO = 10;    // top resistor of 2:1 divider
constexpr uint16_t BATT_DIVIDER_R2_KO = 10;    // bottom resistor
constexpr uint16_t SERIAL_BAUD        = 115200;

// ---- Subsystems ----
MotorDriver       motorL(RoboPins::MOTOR_L_FWD, RoboPins::MOTOR_L_REV);
MotorDriver       motorR(RoboPins::MOTOR_R_FWD, RoboPins::MOTOR_R_REV);
UltrasonicArray   sonars(RoboPins::ULTRA_TRIGGER,
                         RoboPins::ULTRA_ECHO_F, RoboPins::ULTRA_ECHO_B,
                         RoboPins::ULTRA_ECHO_L, RoboPins::ULTRA_ECHO_R);
BuzzerDriver      buzzer(RoboPins::BUZZER);
MoodMatrix        mood;
TftTelemetry      tft({RoboPins::TFT_CS, RoboPins::TFT_DC, RoboPins::TFT_RST});
ObstacleAvoidance avoidance;
RobotState        state(RobotState::Config{ /*commandTimeoutMs=*/3600000, /*sensorTimeoutMs=*/3600000 });
WiFiServer        server(80);

// ---- Runtime state ----
Velocity  desiredVelocity = Velocity::stopped();
MotorOutput lastMotorOut  = MotorOutput::stopped();
float       batteryVolts   = 0.0f;
uint32_t    lastSensorTickMs   = 0;
uint32_t    lastTftMs           = 0;
uint32_t    lastCommandMs       = 0;
uint32_t    bootMs              = 0;
bool        wifiUp              = false;
Mode        prevMode            = Mode::Idle;
FailsafeReason prevFailReason   = FailsafeReason::None;

// =============================================================================
//  Helpers
// =============================================================================

static float readBatteryVolts() {
    // RA4M1 ADC is 14-bit but defaults to 10-bit; we use the default 1023 range.
    // Reference = 5V. Divider = 2:1 -> Vbatt = Vadc * 2.
    const int raw = analogRead(RoboPins::BATT_SENSE);
    const float vAdc = (raw / 1023.0f) * 5.0f;
    const float ratio = (float)(BATT_DIVIDER_R1_KO + BATT_DIVIDER_R2_KO)
                      / (float)BATT_DIVIDER_R2_KO;
    return vAdc * ratio;
}

static BuzzerDriver::Pattern patternForMode(Mode m) {
    switch (m) {
        case Mode::Failsafe:    return BuzzerDriver::Pattern::SteadyBeep;
        case Mode::Alert:       return BuzzerDriver::Pattern::FastBeep;
        case Mode::Avoiding:    return BuzzerDriver::Pattern::SlowBeep;
        case Mode::Autonomous:  return BuzzerDriver::Pattern::DoubleChirp;
        default:                return BuzzerDriver::Pattern::Off;
    }
}

static void applyVelocity(const Velocity& v) {
    lastMotorOut = MotorMixer::mix(v, MAX_PWM);
    motorL.apply(lastMotorOut.left);
    motorR.apply(lastMotorOut.right);
}

static void emergencyStop() {
    motorL.coast();
    motorR.coast();
    lastMotorOut = MotorOutput::stopped();
    desiredVelocity = Velocity::stopped();
}

// =============================================================================
//  WiFi + HTTP
// =============================================================================

static void connectWifi() {
    Serial.print(F("WiFi: connecting to "));
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t deadline = millis() + 15000;
    while (WiFi.status() != WL_CONNECTED && millis() < deadline) {
        delay(250);
        Serial.print('.');
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print(F("WiFi: up, IP="));
        Serial.println(WiFi.localIP());
        wifiUp = true;
        server.begin();
    } else {
        Serial.println(F("WiFi: connection failed - running offline"));
        wifiUp = false;
    }
}

static void writeStatusJson(WiFiClient& client) {
    const uint32_t now = millis();
    JsonDocument doc;
    doc["mode"]       = toString(state.current());
    doc["failReason"] = toString(state.failsafeReason());
    doc["battery"]    = batteryVolts;
    Distances d = sonars.distances();
    doc["front"]   = d.front;
    doc["back"]    = d.back;
    doc["left"]    = d.left;
    doc["right"]   = d.right;
    doc["pwmL"]    = lastMotorOut.left;
    doc["pwmR"]    = lastMotorOut.right;
    doc["uptime"]  = (now - bootMs) / 1000;
    // Watchdog diagnostics - how stale are the inputs the brain is making
    // decisions from? Useful for figuring out which timeout is tripping.
    doc["msSinceSensor"] = now - sonars.lastSampleMillis();
    doc["msSinceCmd"]    = now - lastCommandMs;
    doc["sensorTimeoutMs"]  = state.config().sensorTimeoutMs;
    doc["commandTimeoutMs"] = state.config().commandTimeoutMs;

    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-Type: application/json"));
    client.println(F("Cache-Control: no-store"));
    client.println(F("Connection: close"));
    client.println();
    serializeJson(doc, client);
    client.println();
}

static void writeIndexHtml(WiFiClient& client) {
    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-Type: text/html; charset=utf-8"));
    client.println(F("Cache-Control: no-store"));
    client.println(F("Connection: close"));
    client.println();
    // Stream HTML with the camera URL substituted in.
    const char* p = WEB_UI_HTML;
    const char* placeholder = "__CAMERA_URL__";
    while (true) {
        const char* hit = strstr(p, placeholder);
        if (!hit) { client.print(p); break; }
        client.write((const uint8_t*)p, hit - p);
        client.print(F(CAMERA_STREAM_URL));
        p = hit + strlen(placeholder);
    }
}

static void write404(WiFiClient& client) {
    client.println(F("HTTP/1.1 404 Not Found"));
    client.println(F("Connection: close"));
    client.println();
}

// Parse "cmd=drive&lin=0.5&ang=-0.2" from the URL query.
static void handleCommand(const String& query) {
    String cmd;
    float lin = 0, ang = 0;

    int i = 0;
    while (i < (int)query.length()) {
        int amp = query.indexOf('&', i);
        if (amp < 0) amp = query.length();
        int eq = query.indexOf('=', i);
        if (eq < 0 || eq > amp) { i = amp + 1; continue; }
        String key = query.substring(i, eq);
        String val = query.substring(eq + 1, amp);
        if (key == "cmd") cmd = val;
        else if (key == "lin") lin = val.toFloat();
        else if (key == "ang") ang = val.toFloat();
        i = amp + 1;
    }

    Command c = Command::None;
    if      (cmd == "stop")    c = Command::Stop;
    else if (cmd == "drive")   { c = Command::Drive;   desiredVelocity = {lin, ang}; }
    else if (cmd == "explore") c = Command::Explore;
    else if (cmd == "reset")   c = Command::Reset;

    lastCommandMs = millis();

    RobotState::Inputs in {
        c,
        sonars.distances(),
        avoidance.panic(sonars.distances()),
        0,
        millis() - sonars.lastSampleMillis()
    };
    state.update(in);
}

static void serviceHttp() {
    WiFiClient client = server.available();
    if (!client) return;

    // Read request line: "GET /path?query HTTP/1.1"
    String req = client.readStringUntil('\n');
    // Drain remaining headers.
    while (client.available()) {
        String h = client.readStringUntil('\n');
        if (h.length() <= 1) break;
    }

    int sp1 = req.indexOf(' ');
    int sp2 = req.indexOf(' ', sp1 + 1);
    if (sp1 < 0 || sp2 < 0) { client.stop(); return; }
    String method = req.substring(0, sp1);
    String url    = req.substring(sp1 + 1, sp2);

    int q = url.indexOf('?');
    String path  = q < 0 ? url : url.substring(0, q);
    String query = q < 0 ? ""  : url.substring(q + 1);

    if (path == "/")              writeIndexHtml(client);
    else if (path == "/status")   writeStatusJson(client);
    else if (path == "/cmd")    { handleCommand(query); writeStatusJson(client); }
    else                          write404(client);

    client.stop();
}

// =============================================================================
//  setup / loop
// =============================================================================

void setup() {
    Serial.begin(SERIAL_BAUD);
    delay(200);
    Serial.println(F("\n=== Robo brain booting ==="));

    motorL.begin();
    motorR.begin();
    sonars.begin();
    buzzer.begin();
    mood.begin();
    tft.begin();

    bootMs        = millis();
    lastCommandMs = millis();

    connectWifi();

    Serial.println(F("Robo brain ready"));
}

void loop() {
    const uint32_t now = millis();

    // 1. Sensors - one per tick, rotated.
    if (now - lastSensorTickMs >= SENSOR_TICK_MS) {
        sonars.tick();
        lastSensorTickMs = now;
    }

    // 2. State machine.
    RobotState::Inputs in {
        Command::None,
        sonars.distances(),
        avoidance.panic(sonars.distances()),
        now - lastCommandMs,
        now - sonars.lastSampleMillis()
    };
    Mode mode = state.update(in);

    // ---- Diagnostic: log every mode change with surrounding context ----
    if (mode != prevMode || state.failsafeReason() != prevFailReason) {
        Serial.print(F("[t="));
        Serial.print(now);
        Serial.print(F("ms] mode "));
        Serial.print(toString(prevMode));
        Serial.print(F(" -> "));
        Serial.print(toString(mode));
        if (mode == Mode::Failsafe) {
            Serial.print(F(" (reason="));
            Serial.print(toString(state.failsafeReason()));
            Serial.print(F(")"));
        }
        Serial.print(F("  msSinceSensor="));
        Serial.print(now - sonars.lastSampleMillis());
        Serial.print(F(" msSinceCmd="));
        Serial.print(now - lastCommandMs);
        Serial.print(F(" front="));
        Serial.print(sonars.distances().front);
        Serial.print(F(" back="));
        Serial.print(sonars.distances().back);
        Serial.print(F(" left="));
        Serial.print(sonars.distances().left);
        Serial.print(F(" right="));
        Serial.println(sonars.distances().right);
        prevMode = mode;
        prevFailReason = state.failsafeReason();
    }

    // 3. Decide velocity based on mode.
    Velocity v;
    switch (mode) {
        case Mode::Idle:
        case Mode::Failsafe:
            v = Velocity::stopped();
            break;
        case Mode::ManualDrive:
        case Mode::Alert:
            v = avoidance.filter(desiredVelocity, sonars.distances());
            break;
        case Mode::Autonomous:
        case Mode::Avoiding:
            v = avoidance.chooseExplore(sonars.distances());
            break;
    }

    if (mode == Mode::Failsafe) {
        emergencyStop();
    } else {
        applyVelocity(v);
    }

    // 4. Buzzer + mood.
    buzzer.setPattern(patternForMode(mode));
    buzzer.tick();
    mood.show(MoodSelector::forMode(mode));

    // 5. Battery.
    batteryVolts = batteryVolts * 0.9f + readBatteryVolts() * 0.1f;  // simple EMA

    // 6. TFT - throttled.
    if (now - lastTftMs >= TFT_UPDATE_MS) {
        tft.update(mode, sonars.distances(), batteryVolts,
                   lastMotorOut.left, lastMotorOut.right, wifiUp);
        lastTftMs = now;
    }

    // 7. HTTP.
    if (wifiUp) serviceHttp();
}
