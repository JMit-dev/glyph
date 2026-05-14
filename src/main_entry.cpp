// main_entry.cpp — engine entry point.
//
// Uses SDL3's callback-based main pattern (SDL_MAIN_USE_CALLBACKS). Events
// are dispatched via SDL_AppEvent before each SDL_AppIterate call, so the
// Input state updated by SDL_AppEvent is always visible to on_update().
//
// Input::begin_frame() runs at the END of each iterate, after on_render and
// the buffer swap. This ensures the previous-frame snapshot used for
// key_pressed / key_released is correct on the next frame.

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "platform/window.h"

#include <glyph/app.h>
#include <glyph/input.h>
#include <glyph/renderer.h>

#include <cstdio>
#include <memory>

extern "C" glyph::Game* glyph_create_game();

// AppState is forward-declared as a friend in Game so it can set the service
// pointers (input_, etc.) without exposing a public setter.
struct AppState {
    std::unique_ptr<glyph::Game> game;
    glyph::Window                window;
    glyph::Renderer              renderer;
    glyph::Input                 input;
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

    // Wire engine services into the Game base class.
    state->game->engine_set_input(&state->input);

    state->game->on_start();
    state->prev_ticks_ms = SDL_GetTicks();

    *appstate = state.release();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    auto* state = static_cast<AppState*>(appstate);

    const uint64_t now_ms = SDL_GetTicks();
    const float    dt     = static_cast<float>(now_ms - state->prev_ticks_ms) / 1000.0f;
    state->prev_ticks_ms  = now_ms;

    state->game->on_update(dt);

    state->renderer.begin_frame();
    state->game->on_render(state->renderer);
    state->renderer.end_frame();
    state->window.swap_buffers();

    // begin_frame after the swap: copies current→previous for key_pressed /
    // key_released detection, and resets the scroll-wheel delta.
    state->input.begin_frame();

    return state->window.should_close() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    auto* state = static_cast<AppState*>(appstate);

    switch (event->type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        state->window.request_close();
        return SDL_APP_SUCCESS;

    case SDL_EVENT_KEY_DOWN:
        state->input.set_key(static_cast<int>(event->key.scancode), true);
        break;
    case SDL_EVENT_KEY_UP:
        state->input.set_key(static_cast<int>(event->key.scancode), false);
        break;

    case SDL_EVENT_MOUSE_MOTION:
        state->input.set_mouse_position(event->motion.x, event->motion.y);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        state->input.set_mouse_button(static_cast<int>(event->button.button), true);
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        state->input.set_mouse_button(static_cast<int>(event->button.button), false);
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        state->input.add_mouse_wheel(event->wheel.x, event->wheel.y);
        break;

    default: break;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/) {
    auto* state = static_cast<AppState*>(appstate);
    if (state) {
        state->game->on_shutdown();
        state->renderer.shutdown();
        state->window.destroy();
        delete state;
    }
    SDL_Quit();
}
