// file_io.h — platform-agnostic file reading.
// Internal to the engine; game code accesses files through Resources.
// On web/mobile these helpers will route through SDL's virtual filesystem
// (AAssetManager on Android, Emscripten's preloaded FS on web) — for now
// a straight std::ifstream implementation covers desktop.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace glyph {

class FileIO {
public:
    // Read the entire file as raw bytes. Returns empty on error.
    static std::vector<uint8_t> read_binary(const std::string& path);

    // Read the entire file as a UTF-8 string. Returns empty on error.
    static std::string read_text(const std::string& path);

    static bool exists(const std::string& path);
};

} // namespace glyph
