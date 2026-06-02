#include "BuzzerDriver.h"

BuzzerDriver::BuzzerDriver(uint8_t pin) : pin_(pin) {}

void BuzzerDriver::begin() {
    pinMode(pin_, OUTPUT);
    applyState(false);
    nextEdgeMs_ = millis();
}

void BuzzerDriver::setPattern(Pattern p) {
    if (p == pattern_) return;
    pattern_ = p;
    phase_   = 0;
    nextEdgeMs_ = millis();
    if (p == Pattern::Off)         { applyState(false); }
    if (p == Pattern::SteadyBeep)  { applyState(true);  }
}

void BuzzerDriver::tick() {
    const uint32_t now = millis();
    if ((int32_t)(now - nextEdgeMs_) < 0) return;

    switch (pattern_) {
        case Pattern::Off:
        case Pattern::SteadyBeep:
            // Static states - nothing to do.
            return;

        case Pattern::FastBeep:
            applyState(!state_);
            nextEdgeMs_ = now + 80;
            return;

        case Pattern::SlowBeep:
            applyState(!state_);
            nextEdgeMs_ = now + (state_ ? 200 : 400);
            return;

        case Pattern::DoubleChirp:
            // Phase machine: chirp on, chirp off, chirp on, long silence.
            switch (phase_) {
                case 0: applyState(true);  nextEdgeMs_ = now + 60;   phase_ = 1; break;
                case 1: applyState(false); nextEdgeMs_ = now + 80;   phase_ = 2; break;
                case 2: applyState(true);  nextEdgeMs_ = now + 60;   phase_ = 3; break;
                case 3: applyState(false); nextEdgeMs_ = now + 1000; phase_ = 0; break;
                default: phase_ = 0;
            }
            return;
    }
}

void BuzzerDriver::applyState(bool on) {
    state_ = on;
    digitalWrite(pin_, on ? HIGH : LOW);
}
