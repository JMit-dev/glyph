# Audio

**Header:** `#include <glyph/audio.h>`

Thin wrapper over [miniaudio](https://miniaud.io). The audio device runs on its own thread internally — do not call any Audio methods from background threads.

Access via `audio()` inside any `Game` lifecycle hook. Obtain `Sound` and `Music` via `Resources`.

---

## Quick start

```cpp
void on_start() override {
    res_.set_root("assets");
    shoot_sfx_ = res_.sound("shoot.wav");
    bgm_       = res_.music("theme.ogg");

    audio().play_music(bgm_, 1.0f);   // fade in over 1 second
}

void on_update(float dt) override {
    if (input().action_pressed("fire"))
        audio().play(shoot_sfx_, 0.8f);   // volume 0.8, fire-and-forget
}

void on_shutdown() override {
    audio().stop_music();
    shoot_sfx_.reset();
    bgm_.reset();
    res_.unload_unused();
}
```

---

## Types

### `SoundHandle`

Returned by `Audio::play()`. Discard for fire-and-forget; keep it to stop or adjust volume later.

```cpp
glyph::SoundHandle h = audio().play(snd);
audio().set_volume(h, 0.5f);
audio().stop(h);
```

A handle is silently invalidated when the sound finishes — calling stop/set_volume on a finished handle is safe.

### `Sound`

A short audio clip decoded fully into memory for low-latency, multi-instance playback. Obtained from `Resources::sound()`. Supported formats: WAV, OGG, MP3, FLAC (via miniaudio).

### `Music`

A long audio track streamed from disk. Obtained from `Resources::music()`. Automatically looped when played via `Audio::play_music()`.

---

## Audio methods

### `play(sound, volume, pitch, looped) → SoundHandle`

Play a sound effect. Decodes the file to memory (`MA_SOUND_FLAG_DECODE`) on first play.

| Parameter | Default | Description |
|---|---|---|
| `volume` | `1.0` | Linear gain [0, 1] |
| `pitch` | `1.0` | Playback speed scalar |
| `looped` | `false` | Loop indefinitely |

### `stop(SoundHandle)`
Stop and release the sound instance immediately.

### `set_volume(SoundHandle, float)`
Adjust volume of a playing sound.

### `play_music(music, fade_in_seconds)`
Stream a music track. Replaces any currently playing music. Pass `fade_in_seconds > 0` for a smooth start.

### `stop_music(fade_out_seconds)`
Stop the music. `fade_out_seconds = 0` stops immediately; `> 0` fades out over that duration (music continues until next `play_music` or shutdown).

### `master_volume(float)`
Global volume scalar applied to all sounds and music. Range `[0, 1]`.

---

## Resources integration

```cpp
glyph::Resources res_;
std::shared_ptr<glyph::Sound> shoot_;
std::shared_ptr<glyph::Music> bgm_;

void on_start() override {
    res_.set_root("assets");
    shoot_ = res_.sound("sfx/shoot.wav");
    bgm_   = res_.music("music/theme.ogg");
}
```

`Resources::sound()` and `Resources::music()` cache by resolved path and return `shared_ptr` handles.

---

## Thread safety

All `Audio` methods must be called from the **main thread**. miniaudio manages its own audio callback thread internally; you never need to touch it.
