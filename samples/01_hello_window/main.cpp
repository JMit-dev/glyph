// 01_hello_window — opens a blank window and prints a couple of lifecycle
// messages. Verifies that the engine's startup/shutdown plumbing works.
#include <glyph/glyph.h>

#include <cstdio>

class HelloWindow : public glyph::Game {
public:
    glyph::AppConfig configure() override {
        glyph::AppConfig c;
        c.title  = "Glyph: Hello Window";
        c.width  = 800;
        c.height = 600;
        return c;
    }

    void on_start() override {
        std::printf("[hello_window] window opened — close it to exit\n");
    }

    void on_update(float dt) override {
        // Track elapsed seconds without doing anything fancy.
        elapsed_ += dt;
    }

    void on_shutdown() override {
        std::printf("[hello_window] shutting down after %.2f seconds\n", elapsed_);
    }

private:
    float elapsed_ = 0.0f;
};

GLYPH_MAIN(HelloWindow)
