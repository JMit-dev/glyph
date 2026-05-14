// app.cpp — Game service accessor implementations.
// Pointers are set by AppState (main_entry.cpp) before on_start().
#include <glyph/app.h>
#include <glyph/audio.h>
#include <glyph/input.h>
#include <glyph/time.h>

namespace glyph {

Audio& Game::audio() { return *audio_; }
Input& Game::input() { return *input_; }
Time&  Game::time()  { return *time_;  }

} // namespace glyph
