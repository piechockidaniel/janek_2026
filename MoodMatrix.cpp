#include "MoodMatrix.h"

namespace {

// Each face is 8 rows * 12 columns of LEDs.
// Stored as 8 uint16_t rows, only the low 12 bits used.
// MSB of each row is column 0 (leftmost LED).
// Bit 0xFF000 = column 0; bit 0x00001 = column 11.

constexpr uint32_t makeFrame(uint16_t r0, uint16_t r1, uint16_t r2, uint16_t r3,
                             uint16_t r4, uint16_t r5, uint16_t r6, uint16_t r7) {
    (void)r0; (void)r1; (void)r2; (void)r3; (void)r4; (void)r5; (void)r6; (void)r7;
    return 0;  // unused - we use the uint32_t[3] form below
}

// ArduinoLEDMatrix expects an array of 3 uint32_t = 96 bits = 8x12.
// Packing: bits left-to-right, top-to-bottom. We define each frame as
// a clear 12x8 grid with 0 = LED off, 1 = LED on, then encode.
//
// To keep this readable we build frames at compile time from rows.
struct Frame {
    uint32_t data[3];
};

constexpr Frame frameFromRows(const uint16_t rows[8]) {
    Frame f{{0, 0, 0}};
    // 96 bits total; bit index 0 = top-left.
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 12; ++c) {
            if ((rows[r] >> (11 - c)) & 1) {
                int idx = r * 12 + c;
                int word = idx / 32;
                int bit  = 31 - (idx % 32);
                f.data[word] |= (1UL << bit);
            }
        }
    }
    return f;
}

// 12-bit row literals - read top-to-bottom.

// Sleepy: closed eyes "- -" and small mouth
const uint16_t SLEEPY[8] = {
    0b000000000000,
    0b000000000000,
    0b011000011000,  // closed eyelids (dashes)
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b001111110000,  // small flat mouth
    0b000000000000,
};

// Happy: round eyes, big smile
const uint16_t HAPPY[8] = {
    0b000000000000,
    0b001100001100,
    0b001100001100,
    0b000000000000,
    0b000000000000,
    0b100000000010,
    0b010000000100,
    0b001111111000,
};

// Focused: narrowed eyes, neutral mouth
const uint16_t FOCUSED[8] = {
    0b000000000000,
    0b000000000000,
    0b011100011100,
    0b011100011100,
    0b000000000000,
    0b000000000000,
    0b001111111000,
    0b000000000000,
};

// Surprised: wide eyes, O mouth
const uint16_t SURPRISED[8] = {
    0b011100011100,
    0b011100011100,
    0b011100011100,
    0b000000000000,
    0b000000000000,
    0b000110110000,
    0b001001001000,
    0b000110110000,
};

// Concerned: angled eyes, wavy mouth
const uint16_t CONCERNED[8] = {
    0b010000000010,
    0b001000000100,
    0b000110011000,
    0b000000000000,
    0b000000000000,
    0b000000000000,
    0b001100110000,
    0b010011001100,
};

// Sad: x-eyes, frown
const uint16_t SAD[8] = {
    0b000000000000,
    0b010100010100,
    0b001000001000,
    0b010100010100,
    0b000000000000,
    0b000111111000,
    0b010000000010,
    0b100000000001,
};

const Frame FRAME_SLEEPY    = frameFromRows(SLEEPY);
const Frame FRAME_HAPPY     = frameFromRows(HAPPY);
const Frame FRAME_FOCUSED   = frameFromRows(FOCUSED);
const Frame FRAME_SURPRISED = frameFromRows(SURPRISED);
const Frame FRAME_CONCERNED = frameFromRows(CONCERNED);
const Frame FRAME_SAD       = frameFromRows(SAD);

const uint32_t* frameFor(robot::Mood m) {
    switch (m) {
        case robot::Mood::Sleepy:    return FRAME_SLEEPY.data;
        case robot::Mood::Happy:     return FRAME_HAPPY.data;
        case robot::Mood::Focused:   return FRAME_FOCUSED.data;
        case robot::Mood::Surprised: return FRAME_SURPRISED.data;
        case robot::Mood::Concerned: return FRAME_CONCERNED.data;
        case robot::Mood::Sad:       return FRAME_SAD.data;
    }
    return FRAME_SLEEPY.data;
}

} // namespace

MoodMatrix::MoodMatrix() = default;

void MoodMatrix::begin() {
    matrix_.begin();
    show(robot::Mood::Sleepy);
    initialized_ = true;
}

void MoodMatrix::show(robot::Mood mood) {
    if (initialized_ && mood == current_) return;  // no-op if unchanged
    current_ = mood;
    matrix_.loadFrame(frameFor(mood));
}
