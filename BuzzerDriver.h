#pragma once
#include <Arduino.h>

// Drives the SFM-27 active buzzer with non-blocking patterns.
//
// The SFM-27 has its own internal oscillator; we only toggle the supply.
// HIGH = sound, LOW = silence. No tone()/PWM needed.
//
// All patterns are non-blocking: call tick() each main-loop iteration
// and the driver toggles the pin at the right moments.
class BuzzerDriver {
public:
    enum class Pattern : uint8_t {
        Off,
        SteadyBeep,   // continuous tone (for failsafe)
        FastBeep,     // 80 ms on / 80 ms off  (panic / very close)
        SlowBeep,     // 200 ms on / 400 ms off (slow zone warning)
        DoubleChirp   // two short chirps then 1 s silence (autonomous heartbeat)
    };

    explicit BuzzerDriver(uint8_t pin);

    void begin();
    void setPattern(Pattern p);
    void tick();
    Pattern pattern() const { return pattern_; }

private:
    uint8_t  pin_;
    Pattern  pattern_ = Pattern::Off;
    bool     state_   = false;
    uint32_t nextEdgeMs_ = 0;
    uint8_t  phase_     = 0;  // for multi-step patterns

    void applyState(bool on);
};
