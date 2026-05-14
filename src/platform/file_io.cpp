// file_io.cpp — desktop file I/O implementation.
#include "file_io.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace glyph {

std::vector<uint8_t> FileIO::read_binary(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

std::string FileIO::read_text(const std::string& path) {
    std::ifstream f(path);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool FileIO::exists(const std::string& path) {
    return std::filesystem::exists(path);
}

} // namespace glyph
