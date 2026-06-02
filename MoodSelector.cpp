#include "MoodSelector.h"

namespace robot {

Mood MoodSelector::forMode(Mode m) {
    switch (m) {
        case Mode::Idle:        return Mood::Sleepy;
        case Mode::ManualDrive: return Mood::Happy;
        case Mode::Autonomous:  return Mood::Focused;
        case Mode::Avoiding:    return Mood::Surprised;
        case Mode::Alert:       return Mood::Concerned;
        case Mode::Failsafe:    return Mood::Sad;
    }
    return Mood::Sleepy;
}

const char* MoodSelector::toString(Mood mood) {
    switch (mood) {
        case Mood::Sleepy:    return "Sleepy";
        case Mood::Happy:     return "Happy";
        case Mood::Focused:   return "Focused";
        case Mood::Surprised: return "Surprised";
        case Mood::Concerned: return "Concerned";
        case Mood::Sad:       return "Sad";
    }
    return "?";
}

} // namespace robot
