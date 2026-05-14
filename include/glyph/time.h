// time.h — frame timing and fixed-timestep accumulator.
//
// Implements the "Fix Your Timestep" pattern (Glenn Fiedler, 2006):
//   - Variable render rate; physics/logic at a fixed 60 Hz step.
//   - Accumulator capped at kMaxAccum to prevent spiral-of-death.
//   - fixed_alpha() gives a sub-step interpolation factor for smooth rendering.
//
// Access via Game::time() inside lifecycle hooks.
#pragma once

#include <cstdint>

namespace glyph {

class Time {
public:
    // Fixed-update constants accessible to game code.
    static constexpr float kFixedDt  = 1.f / 60.f;  // 60 Hz physics step
    static constexpr float kMaxAccum = 0.25f;         // spiral-of-death cap (~4 steps max allowed behind)

    // --- Per-frame queries ---

    // Time-scaled, capped frame delta (seconds). Use this in on_update.
    float    delta()       const { return delta_; }

    // Unscaled wall-clock time since the engine started (seconds).
    double   elapsed()     const { return elapsed_; }

    uint64_t frame_count() const { return frame_; }

    // Exponentially smoothed frame rate.
    float    fps()         const { return fps_; }

    // Interpolation factor in [0, 1] between the last two fixed steps.
    // Use for rendering interpolated physics positions:
    //   render_pos = lerp(prev_pos, curr_pos, time().fixed_alpha())
    float    fixed_alpha() const { return alpha_; }

    float    time_scale()  const { return scale_; }
    void     set_time_scale(float s) { scale_ = s > 0.f ? s : 0.f; }

    // --- Engine-internal ---

    // Advance by raw_dt seconds (before capping and scaling).
    // Call once per frame before the fixed-update loop.
    void tick(float raw_dt);

    // Consume one fixed step from the accumulator.
    // Returns true if a step was available; call in a while loop:
    //   while (time.step_fixed()) game->on_fixed_update(Time::kFixedDt);
    // Sets fixed_alpha() when it returns false (loop ends).
    bool step_fixed();

private:
    float    delta_       = 0.f;
    double   elapsed_     = 0.0;
    uint64_t frame_       = 0;
    float    fps_         = 0.f;
    float    scale_       = 1.f;
    float    accumulator_ = 0.f;
    float    alpha_       = 0.f;
};

} // namespace glyph
