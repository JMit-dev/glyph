// sprite_sheet.cpp — SpriteSheet::load() implementation.
//
// Parses Aseprite JSON (Hash or Array format).
// ordered_json is used for the Hash format so frames stay in export order.
#include <glyph/sprite_sheet.h>

#include <nlohmann/json.hpp>
#include <stb_image.h>

#include <cstdio>
#include <filesystem>
#include <fstream>

namespace glyph {

std::shared_ptr<SpriteSheet> SpriteSheet::load(const std::string& json_path) {
    std::ifstream f(json_path);
    if (!f.is_open()) {
        std::fprintf(stderr, "[glyph] SpriteSheet: cannot open %s\n", json_path.c_str());
        return nullptr;
    }

    nlohmann::ordered_json j;
    try {
        f >> j;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "[glyph] SpriteSheet: JSON parse error in %s: %s\n",
                     json_path.c_str(), e.what());
        return nullptr;
    }

    auto sheet = std::make_shared<SpriteSheet>();

    // --- Parse frames + per-frame durations ---
    // Aseprite hash format: "frames" is an object (keys = frame names, in export order).
    // Array format: "frames" is an array. Both are handled.
    struct FrameData { Rect rect; float dur_sec; };
    std::vector<FrameData> frame_data;

    const auto& frames_node = j["frames"];
    auto parse_frame = [&](const nlohmann::ordered_json& val) {
        const auto& fr = val["frame"];
        frame_data.push_back({
            { fr["x"].get<float>(), fr["y"].get<float>(),
              fr["w"].get<float>(), fr["h"].get<float>() },
            val.value("duration", 100.0f) / 1000.f
        });
        sheet->frames.push_back(frame_data.back().rect);
    };

    if (frames_node.is_object()) {
        for (const auto& [key, val] : frames_node.items())
            parse_frame(val);
    } else if (frames_node.is_array()) {
        for (const auto& val : frames_node)
            parse_frame(val);
    }

    // --- Parse frameTags into AnimClips ---
    if (j.contains("meta") && j["meta"].contains("frameTags")) {
        for (const auto& tag : j["meta"]["frameTags"]) {
            AnimClip clip;
            clip.name = tag["name"].get<std::string>();
            const int from = tag["from"].get<int>();
            const int to   = tag["to"].get<int>();
            for (int i = from; i <= to; ++i) {
                clip.frame_indices.push_back(i);
                clip.durations.push_back(
                    i < static_cast<int>(frame_data.size())
                        ? frame_data[i].dur_sec : 0.1f);
            }
            sheet->clips[clip.name] = std::move(clip);
        }
    }

    // --- Load companion PNG texture ---
    const std::string img_name =
        j.contains("meta") ? j["meta"].value("image", "") : "";
    if (img_name.empty()) {
        std::fprintf(stderr, "[glyph] SpriteSheet: no meta.image in %s\n", json_path.c_str());
        return nullptr;
    }

    const auto png_path =
        (std::filesystem::path(json_path).parent_path() / img_name).string();

    int w, h, ch;
    uint8_t* pixels = stbi_load(png_path.c_str(), &w, &h, &ch, 4);
    if (!pixels) {
        std::fprintf(stderr, "[glyph] SpriteSheet: cannot load texture %s\n", png_path.c_str());
        return nullptr;
    }

    auto tex = std::make_shared<Texture>();
    const bool ok = tex->create(w, h, pixels);
    stbi_image_free(pixels);
    if (!ok) {
        std::fprintf(stderr, "[glyph] SpriteSheet: texture upload failed for %s\n", png_path.c_str());
        return nullptr;
    }

    sheet->texture = std::move(tex);
    return sheet;
}

} // namespace glyph
