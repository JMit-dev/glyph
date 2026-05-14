// time.cpp — Time implementation.
#include <glyph/time.h>

#include <algorithm>  // std::min, std::clamp

namespace glyph {

void Time::tick(float raw_dt) {
    // Cap raw_dt so a single frozen frame can't flood the fixed-update loop.
    raw_dt = std::min(raw_dt, kMaxAccum);

    // Scaled delta exposed to on_update.
    delta_ = raw_dt * scale_;

    // Accumulate for fixed steps (also capped to guard against scale > 1).
    accumulator_ = std::min(accumulator_ + delta_, kMaxAccum);

    // Unscaled wall time.
    elapsed_ += static_cast<double>(raw_dt);

    // Smoothed FPS (exponential moving average, ~20-frame window).
    if (raw_dt > 0.f) {
        const float inst = 1.f / raw_dt;
        fps_ = (frame_ == 0) ? inst : fps_ * 0.95f + inst * 0.05f;
    }

    ++frame_;
}

bool Time::step_fixed() {
    if (accumulator_ >= kFixedDt) {
        accumulator_ -= kFixedDt;
        return true;
    }
    // Loop ended — set alpha for rendering interpolation.
    alpha_ = accumulator_ / kFixedDt;
    return false;
}

} // namespace glyph
