#pragma once
#include <cstdint>
#include "RobotState.h"

namespace robot {

// Visual moods the LED matrix can render. The actual pixel data lives in
// the MoodMatrix hardware driver; this enum is the contract between them.
enum class Mood : uint8_t {
    Sleepy,    // ---  ---
    Happy,     // ^_^
    Focused,   // -_-
    Surprised, // o_o
    Concerned, // >_<
    Sad        // x_x
};

// Pure mapping from state to mood. Tested in isolation in native env.
class MoodSelector {
public:
    static Mood forMode(Mode m);
    static const char* toString(Mood mood);
};

} // namespace robot
