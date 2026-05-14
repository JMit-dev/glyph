// main_entry.cpp — engine entry point.
//
// Uses SDL3's callback-based main pattern. SDL calls SDL_AppEvent for each
// pending event before calling SDL_AppIterate, so Input state updated in
// SDL_AppEvent is always visible to on_update().
//
// Frame loop order (per iterate):
//   time.tick()                  — advance timing, accumulate fixed steps
//   while time.step_fixed()      — drain fixed-update budget (capped at 0.25 s)
//     game->on_fixed_update(dt)
//   game->on_update(dt)          — variable-rate game logic
//   render + swap                — draw, present
//   input.begin_frame()          — advance key pressed/released snapshots

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "platform/window.h"

#include <glyph/app.h>
#include <glyph/input.h>
#include <glyph/renderer.h>
#include <glyph/time.h>

#include <cstdio>
#include <memory>

extern "C" glyph::Game* glyph_create_game();

struct AppState {
    std::unique_ptr<glyph::Game> game;
    glyph::Window                window;
    glyph::Renderer              renderer;
    glyph::Input                 input;
    glyph::Time                  time;
    uint64_t                     prev_ticks_ms = 0;
};

SDL_AppResult SDL_AppInit(void** appstate, int /*argc*/, char* /*argv*/[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "[glyph] SDL_Init failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    auto state = std::make_unique<AppState>();

    state->game.reset(glyph_create_game());
    if (!state->game) {
        std::fprintf(stderr, "[glyph] glyph_create_game returned null\n");
        return SDL_APP_FAILURE;
    }

    const glyph::AppConfig config = state->game->configure();
    if (!state->window.create(config)) return SDL_APP_FAILURE;

    const glyph::ivec2 drawable = state->window.drawable_size();
    if (!state->renderer.init(drawable.x, drawable.y)) {
        std::fprintf(stderr, "[glyph] Renderer::init failed\n");
        return SDL_APP_FAILURE;
    }

    state->game->engine_set_input(&state->input);
    state->game->engine_set_time (&state->time);

    state->game->on_start();
    state->prev_ticks_ms = SDL_GetTicks();

    *appstate = state.release();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    auto* s = static_cast<AppState*>(appstate);

    // Raw frame time; Time::tick() caps and scales it.
    const uint64_t now_ms = SDL_GetTicks();
    const float raw_dt = static_cast<float>(now_ms - s->prev_ticks_ms) / 1000.f;
    s->prev_ticks_ms = now_ms;

    s->time.tick(raw_dt);

    // Fixed-rate updates — at most kMaxAccum / kFixedDt per frame.
    while (s->time.step_fixed()) {
        s->game->on_fixed_update(glyph::Time::kFixedDt);
    }

    // Variable-rate update — dt is already capped and time-scaled.
    s->game->on_update(s->time.delta());

    s->renderer.begin_frame();
    s->game->on_render(s->renderer);
    s->renderer.end_frame();
    s->window.swap_buffers();

    s->input.begin_frame();

    return s->window.should_close() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    auto* s = static_cast<AppState*>(appstate);

    switch (event->type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        s->window.request_close();
        return SDL_APP_SUCCESS;

    case SDL_EVENT_KEY_DOWN:
        s->input.set_key(static_cast<int>(event->key.scancode), true);
        break;
    case SDL_EVENT_KEY_UP:
        s->input.set_key(static_cast<int>(event->key.scancode), false);
        break;

    case SDL_EVENT_MOUSE_MOTION:
        s->input.set_mouse_position(event->motion.x, event->motion.y);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        s->input.set_mouse_button(static_cast<int>(event->button.button), true);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        s->input.set_mouse_button(static_cast<int>(event->button.button), false);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        s->input.add_mouse_wheel(event->wheel.x, event->wheel.y);
        break;

    default: break;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/) {
    auto* s = static_cast<AppState*>(appstate);
    if (s) {
        s->game->on_shutdown();
        s->renderer.shutdown();
        s->window.destroy();
        delete s;
    }
    SDL_Quit();
}
