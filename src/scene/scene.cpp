// scene.cpp — Scene non-template method implementations.
#include <glyph/scene.h>
#include <glyph/components.h>
#include <glyph/renderer.h>

namespace glyph {

Entity Scene::create_entity() {
    return {this, registry_.create()};
}

Entity Scene::create_entity(const std::string& name) {
    Entity e = create_entity();
    e.add<Name>(Name{name});
    return e;
}

Entity Scene::find(const std::string& name) {
    auto view = registry_.view<Name>();
    for (auto e : view) {
        if (view.get<Name>(e).value == name)
            return {this, e};
    }
    return {};
}

void Scene::run_systems(float /*dt*/) {
    // Built-in systems (Lifetime, Movement, Animator, Collision) implemented in phase 11.
}

void Scene::render(Renderer& /*r*/) {
    // Sprite + Transform rendering implemented in phase 11.
}

void Scene::clear() {
    registry_.clear();
}

} // namespace glyph
