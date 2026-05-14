// input.cpp — Input implementation.
#include <glyph/input.h>

#include <algorithm>  // std::clamp
#include <cstring>    // memcpy

namespace glyph {

Input::Input() {
    // Default bindings from spec §13.
    // move_x / move_y support both WASD and arrow keys.
    bind_axis("move_x", Key::A,    Key::D);
    bind_axis("move_x", Key::Left, Key::Right);
    bind_axis("move_y", Key::W,    Key::S);
    bind_axis("move_y", Key::Up,   Key::Down);

    bind_action("confirm", Key::Space);
    bind_action("confirm", Key::Return);
    bind_action("cancel",  Key::Escape);
}

// --- Keyboard ---

bool Input::key_down(Key k) const {
    int s = static_cast<int>(k);
    return s > 0 && s < kKeyCount && key_cur_[s];
}

bool Input::key_pressed(Key k) const {
    int s = static_cast<int>(k);
    return s > 0 && s < kKeyCount && key_cur_[s] && !key_prv_[s];
}

bool Input::key_released(Key k) const {
    int s = static_cast<int>(k);
    return s > 0 && s < kKeyCount && !key_cur_[s] && key_prv_[s];
}

// --- Mouse ---

bool Input::mouse_down(MouseButton b) const {
    int i = static_cast<int>(b);
    return i > 0 && i < kBtnCount && btn_cur_[i];
}

bool Input::mouse_pressed(MouseButton b) const {
    int i = static_cast<int>(b);
    return i > 0 && i < kBtnCount && btn_cur_[i] && !btn_prv_[i];
}

bool Input::mouse_released(MouseButton b) const {
    int i = static_cast<int>(b);
    return i > 0 && i < kBtnCount && !btn_cur_[i] && btn_prv_[i];
}

// --- Bindings ---

void Input::bind_action(const std::string& name, Key k) {
    actions_[name].keys.push_back(k);
}

void Input::bind_axis(const std::string& name, Key neg, Key pos) {
    axes_[name].pairs.push_back({neg, pos});
}

bool Input::action_down(const std::string& name) const {
    auto it = actions_.find(name);
    if (it == actions_.end()) return false;
    for (Key k : it->second.keys) if (key_down(k)) return true;
    return false;
}

bool Input::action_pressed(const std::string& name) const {
    auto it = actions_.find(name);
    if (it == actions_.end()) return false;
    for (Key k : it->second.keys) if (key_pressed(k)) return true;
    return false;
}

float Input::axis_value(const std::string& name) const {
    auto it = axes_.find(name);
    if (it == axes_.end()) return 0.f;
    float v = 0.f;
    for (const auto& p : it->second.pairs) {
        if (key_down(p.pos)) v += 1.f;
        if (key_down(p.neg)) v -= 1.f;
    }
    return std::clamp(v, -1.f, 1.f);
}

// --- Engine-internal updates ---

void Input::begin_frame() {
    std::memcpy(key_prv_, key_cur_, sizeof(key_cur_));
    std::memcpy(btn_prv_, btn_cur_, sizeof(btn_cur_));
    mouse_wheel_ = {0.f, 0.f};
}

void Input::set_key(int scancode, bool down) {
    if (scancode > 0 && scancode < kKeyCount) key_cur_[scancode] = down;
}

void Input::set_mouse_button(int button, bool down) {
    if (button > 0 && button < kBtnCount) btn_cur_[button] = down;
}

void Input::set_mouse_position(float x, float y) {
    mouse_pos_ = {x, y};
}

void Input::add_mouse_wheel(float dx, float dy) {
    mouse_wheel_.x += dx;
    mouse_wheel_.y += dy;
}

} // namespace glyph
