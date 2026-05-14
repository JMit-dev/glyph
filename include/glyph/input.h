// input.h — keyboard, mouse, and action-binding input system.
// All methods must be called from the main thread.
// Access via Game::input() inside lifecycle hooks.
#pragma once

#include <glyph/math.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace glyph {

// Values match SDL_Scancode so no translation table is needed internally.
enum class Key : int {
    Unknown = 0,
    A = 4,  B = 5,  C = 6,  D = 7,  E = 8,  F = 9,  G = 10,
    H = 11, I = 12, J = 13, K = 14, L = 15, M = 16, N = 17,
    O = 18, P = 19, Q = 20, R = 21, S = 22, T = 23, U = 24,
    V = 25, W = 26, X = 27, Y = 28, Z = 29,
    Num1 = 30, Num2 = 31, Num3 = 32, Num4 = 33, Num5 = 34,
    Num6 = 35, Num7 = 36, Num8 = 37, Num9 = 38, Num0 = 39,
    Return = 40, Escape = 41, Backspace = 42, Tab = 43, Space = 44,
    F1 = 58, F2 = 59, F3 = 60, F4 = 61,  F5 = 62,  F6 = 63,
    F7 = 64, F8 = 65, F9 = 66, F10 = 67, F11 = 68, F12 = 69,
    Right = 79, Left = 80, Down = 81, Up = 82,
    LCtrl = 224, LShift = 225, LAlt = 226,
    RCtrl = 228, RShift = 229, RAlt = 230,
};

// Values match SDL_BUTTON_* constants.
enum class MouseButton : int { Left = 1, Middle = 2, Right = 3, X1 = 4, X2 = 5 };

// Stub enums — gamepad input wired up when SDL gamepad events are added.
enum class GamepadButton : int { A, B, X, Y, LB, RB, Start, Select, LStick, RStick };
enum class GamepadAxis   : int { LeftX, LeftY, RightX, RightY, LTrigger, RTrigger };

class Input {
public:
    Input();  // sets up default action and axis bindings

    // --- Keyboard ---
    bool key_down    (Key k) const;
    bool key_pressed (Key k) const;   // true only on the frame the key went down
    bool key_released(Key k) const;   // true only on the frame the key went up

    // --- Mouse ---
    vec2 mouse_position() const { return mouse_pos_; }
    bool mouse_down    (MouseButton b) const;
    bool mouse_pressed (MouseButton b) const;
    bool mouse_released(MouseButton b) const;
    vec2 mouse_wheel   () const { return mouse_wheel_; }  // scroll delta this frame

    // --- Named action bindings ---
    // Multiple keys can be bound to one action; any of them triggers it.
    void bind_action(const std::string& name, Key k);
    // Axis: neg key produces -1, pos key produces +1. Multiple pairs are OR'd and clamped.
    void bind_axis  (const std::string& name, Key neg, Key pos);

    bool  action_down   (const std::string& name) const;
    bool  action_pressed(const std::string& name) const;
    float axis_value    (const std::string& name) const;

    // --- Engine-internal: called from main_entry.cpp ---
    void begin_frame();                         // swap prev/current; reset wheel
    void set_key          (int scancode, bool down);
    void set_mouse_button (int button,   bool down);
    void set_mouse_position(float x, float y);
    void add_mouse_wheel  (float dx, float dy);

private:
    static constexpr int kKeyCount = 512;
    static constexpr int kBtnCount = 6;

    bool key_cur_[kKeyCount] = {};
    bool key_prv_[kKeyCount] = {};
    bool btn_cur_[kBtnCount] = {};
    bool btn_prv_[kBtnCount] = {};
    vec2 mouse_pos_   {0, 0};
    vec2 mouse_wheel_ {0, 0};

    struct ActionDef { std::vector<Key> keys; };
    struct AxisDef   {
        struct Pair { Key neg = Key::Unknown; Key pos = Key::Unknown; };
        std::vector<Pair> pairs;
    };

    std::unordered_map<std::string, ActionDef> actions_;
    std::unordered_map<std::string, AxisDef>   axes_;
};

} // namespace glyph
