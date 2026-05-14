// audio.h — sound effects and music playback.
// Thin wrapper over miniaudio. No miniaudio types in the public API.
// Access via Game::audio() inside lifecycle hooks.
//
// Sound — fully decoded into memory; supports many simultaneous instances.
// Music — streamed from disk; one track at a time.
#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace glyph {

// Returned by Audio::play(). Discard for fire-and-forget; keep to stop or
// adjust volume. Invalidated automatically when the sound finishes.
struct SoundHandle {
    uint32_t id = 0;
    bool valid() const { return id != 0; }
};

// A short audio clip decoded fully into memory at load time.
// Obtained from Resources::sound().
class Sound {
public:
    const std::string& path() const { return path_; }
private:
    friend class Resources;
    explicit Sound(std::string p) : path_(std::move(p)) {}
    std::string path_;
};

// A long audio track streamed from disk during playback.
// Obtained from Resources::music().
class Music {
public:
    const std::string& path() const { return path_; }
private:
    friend class Resources;
    explicit Music(std::string p) : path_(std::move(p)) {}
    std::string path_;
};

class Audio {
public:
    Audio();
    ~Audio();

    // Initialise the audio device. Returns false if no device is available;
    // the engine continues without sound rather than crashing.
    bool init();
    void shutdown();

    // Play a sound effect. Returns a handle for optional control.
    SoundHandle play(const std::shared_ptr<Sound>& snd,
                     float volume = 1.f,
                     float pitch  = 1.f,
                     bool  looped = false);

    void stop      (SoundHandle h);
    void set_volume(SoundHandle h, float volume);

    // Stream a music track. Replaces any currently playing music.
    void play_music(const std::shared_ptr<Music>& mus, float fade_in_seconds  = 0.f);
    void stop_music(float fade_out_seconds = 0.f);

    // Global volume scalar [0, 1].
    void master_volume(float vol);

    // Engine-internal: release ma_sound instances that have finished.
    void update();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace glyph
