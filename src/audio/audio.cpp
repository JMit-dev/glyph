// audio.cpp — Audio implementation via miniaudio.
#include <glyph/audio.h>

#include <miniaudio.h>

#include <cstdio>
#include <unordered_map>

namespace glyph {

// Stops and uninitialises a ma_sound when the unique_ptr is destroyed.
struct MaSoundDeleter {
    void operator()(ma_sound* s) const {
        ma_sound_stop(s);
        ma_sound_uninit(s);
        delete s;
    }
};
using MaSoundPtr = std::unique_ptr<ma_sound, MaSoundDeleter>;

struct Audio::Impl {
    ma_engine engine{};
    bool      initialized = false;

    std::unordered_map<uint32_t, MaSoundPtr> sounds;
    MaSoundPtr                               music;
    uint32_t                                 next_id = 1;
};

Audio::Audio()  = default;
Audio::~Audio() { shutdown(); }

bool Audio::init() {
    impl_ = std::make_unique<Impl>();

    if (ma_engine_init(nullptr, &impl_->engine) != MA_SUCCESS) {
        std::fprintf(stderr, "[glyph] Audio: ma_engine_init failed — running without sound\n");
        impl_.reset();
        return false;
    }

    impl_->initialized = true;
    return true;
}

void Audio::shutdown() {
    if (!impl_) return;
    if (impl_->initialized) {
        impl_->sounds.clear();  // MaSoundDeleter stops + uninits each sound
        impl_->music.reset();
        ma_engine_uninit(&impl_->engine);
        impl_->initialized = false;
    }
    impl_.reset();
}

SoundHandle Audio::play(const std::shared_ptr<Sound>& snd,
                        float volume, float pitch, bool looped) {
    if (!impl_ || !impl_->initialized || !snd) return {};

    auto ms = MaSoundPtr(new ma_sound);

    // MA_SOUND_FLAG_DECODE: decode the file to memory for low-latency playback.
    constexpr ma_uint32 kFlags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;
    if (ma_sound_init_from_file(&impl_->engine, snd->path().c_str(),
                                kFlags, nullptr, nullptr, ms.get()) != MA_SUCCESS) {
        std::fprintf(stderr, "[glyph] Audio::play: cannot load '%s'\n", snd->path().c_str());
        return {};
    }

    ma_sound_set_volume (ms.get(), volume);
    ma_sound_set_pitch  (ms.get(), pitch);
    ma_sound_set_looping(ms.get(), static_cast<ma_bool32>(looped));
    ma_sound_start      (ms.get());

    const uint32_t id = impl_->next_id++;
    impl_->sounds[id] = std::move(ms);
    return SoundHandle{id};
}

void Audio::stop(SoundHandle h) {
    if (!impl_ || !h.valid()) return;
    impl_->sounds.erase(h.id);   // deleter stops + uninits
}

void Audio::set_volume(SoundHandle h, float volume) {
    if (!impl_ || !h.valid()) return;
    auto it = impl_->sounds.find(h.id);
    if (it != impl_->sounds.end())
        ma_sound_set_volume(it->second.get(), volume);
}

void Audio::play_music(const std::shared_ptr<Music>& mus, float fade_in_seconds) {
    if (!impl_ || !impl_->initialized || !mus) return;

    impl_->music.reset();   // stop previous track immediately

    auto ms = MaSoundPtr(new ma_sound);

    constexpr ma_uint32 kFlags = MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION;
    if (ma_sound_init_from_file(&impl_->engine, mus->path().c_str(),
                                kFlags, nullptr, nullptr, ms.get()) != MA_SUCCESS) {
        std::fprintf(stderr, "[glyph] Audio::play_music: cannot load '%s'\n",
                     mus->path().c_str());
        return;
    }

    ma_sound_set_looping(ms.get(), MA_TRUE);

    if (fade_in_seconds > 0.f) {
        const auto ms_dur = static_cast<ma_uint64>(fade_in_seconds * 1000.f);
        ma_sound_set_fade_in_milliseconds(ms.get(), 0.f, 1.f, ms_dur);
    }

    ma_sound_start(ms.get());
    impl_->music = std::move(ms);
}

void Audio::stop_music(float fade_out_seconds) {
    if (!impl_ || !impl_->music) return;

    if (fade_out_seconds > 0.f) {
        // Apply a fade; the track keeps playing (and decaying) until the next
        // play_music() call or shutdown(). Use fade_out = 0 for an instant stop.
        const auto ms_dur = static_cast<ma_uint64>(fade_out_seconds * 1000.f);
        ma_sound_set_fade_in_milliseconds(impl_->music.get(), -1.f, 0.f, ms_dur);
    } else {
        impl_->music.reset();
    }
}

void Audio::master_volume(float vol) {
    if (!impl_ || !impl_->initialized) return;
    ma_engine_set_volume(&impl_->engine, vol);
}

void Audio::update() {
    if (!impl_) return;
    // Recycle sound slots that have finished playing.
    for (auto it = impl_->sounds.begin(); it != impl_->sounds.end(); ) {
        it = ma_sound_at_end(it->second.get())
           ? impl_->sounds.erase(it)
           : std::next(it);
    }
}

} // namespace glyph
