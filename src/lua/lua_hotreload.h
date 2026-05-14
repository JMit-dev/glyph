// lua_hotreload.h — internal hot-reload API (desktop only).
#pragma once

namespace glyph { struct LuaStateImpl; }

namespace glyph {

// Check all tracked Lua files for modification every 250 ms.
// Reloads changed entity-script modules and patches live entity self tables.
// Reloads changed global scripts (run_file paths) by re-executing them.
// No-op on web / mobile (guard in .cpp via GLYPH_PLATFORM_DESKTOP).
void check_hot_reload(LuaStateImpl& impl);

} // namespace glyph
