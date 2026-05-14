// lua_state_impl.h — LuaStateImpl definition (internal to src/lua/ only).
#pragma once

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <sol/sol.hpp>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#include <glyph/lua_state.h>

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace glyph {

class Audio;
class Input;
class Resources;
class Scene;
class Time;

struct LuaStateImpl {
    sol::state lua;
    Scene*     scene     = nullptr;
    Resources* resources = nullptr;
    Audio*     audio     = nullptr;
    Input*     input     = nullptr;
    Time*      time      = nullptr;
    float      dt        = 0.f;

    // Module cache: module_name → class table returned by the Lua file.
    std::unordered_map<std::string, sol::table>               module_cache;
    // Modification times for hot reload (entity script modules).
    std::unordered_map<std::string, std::filesystem::file_time_type> module_mtimes;
    // Paths registered via run_file() and their last seen mtimes.
    std::vector<std::string>                                  global_scripts;
    std::unordered_map<std::string, std::filesystem::file_time_type> global_mtimes;

    // Hot reload timer — checked every 250 ms.
    std::chrono::steady_clock::time_point last_reload_check =
        std::chrono::steady_clock::now();
    static constexpr int kReloadIntervalMs = 250;

    // Load a Lua module from scripts/entities/<name>.lua.
    // Returns an invalid table on failure; result is cached after first load.
    sol::table load_script_module(const std::string& name) {
        auto it = module_cache.find(name);
        if (it != module_cache.end()) return it->second;

        const std::string path = "scripts/entities/" + name + ".lua";
        auto result = lua.safe_script_file(path, sol::script_pass_on_error);
        if (!result.valid()) {
            sol::error err = result;
            std::fprintf(stderr, "[glyph] Script: cannot load '%s': %s\n",
                         path.c_str(), err.what());
            return sol::table{};
        }
        sol::table tbl = result;
        if (!tbl.valid()) {
            std::fprintf(stderr, "[glyph] Script: '%s' did not return a table\n",
                         path.c_str());
            return sol::table{};
        }
        module_cache[name] = tbl;

        // Record mtime so hot reload can detect future changes.
        std::error_code ec;
        module_mtimes[name] = std::filesystem::last_write_time(path, ec);

        return tbl;
    }
};

// Defined in lua_bindings.cpp.
void register_lua_bindings(LuaStateImpl& impl);

} // namespace glyph
