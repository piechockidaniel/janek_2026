#include "UltrasonicArray.h"

UltrasonicArray::UltrasonicArray(uint8_t trigger,
                                 uint8_t echoFront,
                                 uint8_t echoBack,
                                 uint8_t echoLeft,
                                 uint8_t echoRight)
    : trigger_(trigger) {
    echoPins_[0] = echoFront;
    echoPins_[1] = echoBack;
    echoPins_[2] = echoLeft;
    echoPins_[3] = echoRight;
    latest_ = {robot::Distances::OUT_OF_RANGE, robot::Distances::OUT_OF_RANGE,
               robot::Distances::OUT_OF_RANGE, robot::Distances::OUT_OF_RANGE};
}

void UltrasonicArray::begin() {
    pinMode(trigger_, OUTPUT);
    digitalWrite(trigger_, LOW);
    for (auto p : echoPins_) pinMode(p, INPUT);
}

bool UltrasonicArray::tick() {
    // Fire trigger pulse: 10 us per HC-SR04 datasheet.
    digitalWrite(trigger_, LOW);
    delayMicroseconds(2);
    digitalWrite(trigger_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger_, LOW);

    const uint8_t idx = nextIndex_;
    const uint16_t cm = measureCm(echoPins_[idx]);

    // Store into the correct field based on rotation index.
    switch (idx) {
        case 0: latest_.front = cm; break;
        case 1: latest_.back  = cm; break;
        case 2: latest_.left  = cm; break;
        case 3: latest_.right = cm; break;
    }
    nextIndex_ = (idx + 1) & 0x03;

    // Update lastSampleMs_ on EVERY tick — the watchdog cares whether the
    // polling loop is alive, not whether any particular sensor sees something.
    // Returning OUT_OF_RANGE is still a valid response (just means "nothing in
    // front of this sensor"), and the loop continues normally.
    lastSampleMs_ = millis();
    return cm != robot::Distances::OUT_OF_RANGE;
}

uint16_t UltrasonicArray::measureCm(uint8_t echoPin) const {
    const unsigned long pulseUs = pulseIn(echoPin, HIGH, ECHO_TIMEOUT_US);
    if (pulseUs == 0) return robot::Distances::OUT_OF_RANGE;
    // Speed of sound ~343 m/s -> round trip: us / 58 == cm.
    const unsigned long cm = pulseUs / 58UL;
    if (cm == 0 || cm >= robot::Distances::OUT_OF_RANGE) {
        return robot::Distances::OUT_OF_RANGE;
    }
    return static_cast<uint16_t>(cm);
}
