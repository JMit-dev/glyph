// main_entry.cpp — engine entry point.
//
// Uses SDL3's callback-based main pattern (SDL_MAIN_USE_CALLBACKS). This
// works the same on desktop, Web (Emscripten), iOS, and Android without
// needing platform-specific main loops — SDL handles the loop scheduling
// behind the scenes (e.g. via emscripten_set_main_loop on the web).
//
// The user provides a Game subclass via GLYPH_MAIN(), which defines
// glyph_create_game(). We resolve that symbol at link time.

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "platform/window.h"

#include <glyph/app.h>

#include <cstdio>
#include <memory>

// Defined by the GLYPH_MAIN(GameClass) macro in user code.
extern "C" glyph::Game* glyph_create_game();

namespace glyph {

struct AppState {
    std::unique_ptr<Game> game;
    Window                window;
    uint64_t              prev_ticks_ms = 0;
};

} // namespace glyph

SDL_AppResult SDL_AppInit(void** appstate, int /*argc*/, char* /*argv*/[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "[glyph] SDL_Init failed: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    auto state  = std::make_unique<glyph::AppState>();
    state->game.reset(glyph_create_game());
    if (!state->game) {
        std::fprintf(stderr, "[glyph] glyph_create_game returned null\n");
        return SDL_APP_FAILURE;
    }

    const glyph::AppConfig config = state->game->configure();
    if (!state->window.create(config)) {
        return SDL_APP_FAILURE;
    }

    state->game->on_start();
    state->prev_ticks_ms = SDL_GetTicks();

    *appstate = state.release();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    auto* state = static_cast<glyph::AppState*>(appstate);

    const uint64_t now_ms = SDL_GetTicks();
    const float    dt     = static_cast<float>(now_ms - state->prev_ticks_ms) / 1000.0f;
    state->prev_ticks_ms  = now_ms;

    state->game->on_update(dt);

    // Phase 1 has no renderer yet; just yield a frame so we don't hog CPU.
    SDL_Delay(16);

    return state->window.should_close() ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    auto* state = static_cast<glyph::AppState*>(appstate);

    switch (event->type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        state->window.request_close();
        return SDL_APP_SUCCESS;
    default:
        break;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult /*result*/) {
    auto* state = static_cast<glyph::AppState*>(appstate);
    if (state) {
        state->game->on_shutdown();
        state->window.destroy();
        delete state;
    }
    SDL_Quit();
}
