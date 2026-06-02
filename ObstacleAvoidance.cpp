#include "ObstacleAvoidance.h"

namespace robot {

Velocity ObstacleAvoidance::filter(const Velocity& desired, const Distances& d) const {
    Velocity out = desired;

    // Block forward motion if the path ahead is blocked.
    if (desired.linear > 0.0f) {
        if (d.front <= cfg_.stopThresholdCm) {
            out.linear = 0.0f;
        } else if (d.front < cfg_.slowThresholdCm) {
            // Linear taper between stop and slow thresholds.
            const float range = static_cast<float>(cfg_.slowThresholdCm - cfg_.stopThresholdCm);
            const float over  = static_cast<float>(d.front - cfg_.stopThresholdCm);
            const float scale = range > 0.0f ? over / range : 1.0f;
            out.linear *= scale;
        }
    }

    // Block reverse motion if obstacle behind.
    if (desired.linear < 0.0f && d.back <= cfg_.stopThresholdCm) {
        out.linear = 0.0f;
    }

    return out;
}

Velocity ObstacleAvoidance::chooseExplore(const Distances& d) const {
    const bool frontClear = d.front > cfg_.slowThresholdCm;
    const bool backClear  = d.back  > cfg_.slowThresholdCm;

    if (frontClear) {
        // Path ahead is open - cruise forward, gently steering toward
        // whichever side has more room (avoids skimming walls).
        const float leftRoom  = static_cast<float>(d.left);
        const float rightRoom = static_cast<float>(d.right);
        const float bias = (leftRoom - rightRoom) / 200.0f; // small correction
        const float angular = bias > 0.3f ? 0.3f : (bias < -0.3f ? -0.3f : bias);
        return {0.6f, angular};
    }

    // Front blocked. Pick the side with the most room and turn in place.
    if (d.left > d.right) {
        return Velocity::spinLeft(0.7f);
    }
    if (d.right > d.left) {
        return Velocity::spinRight(0.7f);
    }

    // Boxed in left/right too. Back up if there is room behind.
    if (backClear) {
        return {-0.5f, 0.0f};
    }

    // Fully cornered. Stop and wait for human help.
    return Velocity::stopped();
}

bool ObstacleAvoidance::panic(const Distances& d) const {
    return d.front <= cfg_.panicThresholdCm || d.back <= cfg_.panicThresholdCm;
}

} // namespace robot
