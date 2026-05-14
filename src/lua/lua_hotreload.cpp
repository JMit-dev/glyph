// lua_hotreload.cpp — file-polling hot reload for Lua entity scripts.
//
// Algorithm (spec §19.4):
//   Every 250 ms, for each tracked module:
//     If the file's mtime changed:
//       1. Evict from module_cache.
//       2. Reload via load_script_module (re-executes, re-caches).
//       3. Patch all live entity self tables: update metatable.__index to the
//          new class table so methods update in place while per-entity state
//          (position, speed, etc.) is preserved.
//   Also re-executes global scripts (loaded via run_file) when they change.
//
// Desktop only — disabled on web/mobile via GLYPH_PLATFORM_DESKTOP guard.
#include "lua_hotreload.h"
#include "lua_state_impl.h"

#include <glyph/components.h>   // Script

#ifdef GLYPH_PLATFORM_DESKTOP
#include <any>
#include <cstdio>
#include <filesystem>
#include <vector>
#endif

namespace glyph {

void check_hot_reload(LuaStateImpl& impl) {
#ifdef GLYPH_PLATFORM_DESKTOP
    using namespace std::chrono;

    const auto now     = steady_clock::now();
    const auto elapsed = duration_cast<milliseconds>(now - impl.last_reload_check).count();
    if (elapsed < LuaStateImpl::kReloadIntervalMs) return;
    impl.last_reload_check = now;

    // --- Entity script modules ---
    std::vector<std::string> changed_modules;
    for (auto& [name, cached_mtime] : impl.module_mtimes) {
        const std::string path = "scripts/entities/" + name + ".lua";
        std::error_code ec;
        const auto mtime = std::filesystem::last_write_time(path, ec);
        if (ec || mtime == cached_mtime) continue;
        cached_mtime = mtime;
        changed_modules.push_back(name);
    }

    for (const auto& name : changed_modules) {
        // Evict and reload.
        impl.module_cache.erase(name);
        sol::table new_tbl = impl.load_script_module(name);
        if (!new_tbl.valid()) continue;

        // Patch every live entity self table for this module.
        if (!impl.scene) continue;
        auto& reg = impl.scene->registry();
        auto view = reg.view<Script>();
        for (auto e : view) {
            auto& sc = view.get<Script>(e);
            if (sc.lua_module != name || !sc.self.has_value()) continue;
            sol::table& self = std::any_cast<sol::table&>(sc.self);
            sol::table mt = self[sol::metatable_key];
            if (mt.valid()) mt[sol::meta_function::index] = new_tbl;
        }

        std::printf("[glyph] Hot reload: scripts/entities/%s.lua\n", name.c_str());
    }

    // --- Global scripts (run_file paths) ---
    for (const auto& path : impl.global_scripts) {
        std::error_code ec;
        const auto mtime = std::filesystem::last_write_time(path, ec);
        if (ec) continue;
        auto& recorded = impl.global_mtimes[path];
        if (mtime == recorded) continue;
        recorded = mtime;

        auto result = impl.lua.safe_script_file(path, sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            std::fprintf(stderr, "[glyph] Hot reload error in %s: %s\n",
                         path.c_str(), err.what());
        } else {
            std::printf("[glyph] Hot reload: %s\n", path.c_str());
        }
    }
#else
    (void)impl;
#endif
}

} // namespace glyph
