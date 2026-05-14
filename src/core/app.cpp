// app.cpp — Game service accessor implementations.
// Pointers are set by AppState (main_entry.cpp) before on_start().
#include <glyph/app.h>
#include <glyph/audio.h>
#include <glyph/input.h>
#include <glyph/lua_state.h>
#include <glyph/resources.h>
#include <glyph/scene.h>
#include <glyph/time.h>

namespace glyph {

Audio&     Game::audio()     { return *audio_;     }
Input&     Game::input()     { return *input_;     }
LuaState&  Game::lua()       { return *lua_;       }
Resources& Game::resources() { return *resources_; }
Scene&     Game::scene()     { return *scene_;     }
Time&      Game::time()      { return *time_;      }

} // namespace glyph
