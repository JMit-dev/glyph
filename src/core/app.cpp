// app.cpp — Game service accessor implementations.
// The pointers are set by AppState (main_entry.cpp) before on_start().
#include <glyph/app.h>
#include <glyph/input.h>

namespace glyph {

Input& Game::input() { return *input_; }

} // namespace glyph
