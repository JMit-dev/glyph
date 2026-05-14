// scene.h — Scene and Entity.
//
// Entity is a value-typed handle (Scene* + entt::entity). Scene owns the
// entt::registry. Entity template methods are defined after Scene so they
// can call scene_->registry() with the complete type.
//
// scene.h intentionally does not include components.h — that would create
// a circular dependency (Transform stores Entity). Components are visible
// to scene.cpp where systems iterate them.
#pragma once

#include <glyph/math.h>

#include <entt/entt.hpp>

#include <functional>
#include <string>

namespace glyph {

class Renderer;   // forward — scene.h doesn't pull in the full renderer chain
class Scene;      // forward — needed by Entity declaration

// ---------------------------------------------------------------------------
// Entity — value-typed handle over entt::entity + Scene pointer.
// Pass by value; default-constructed entity is invalid (safe to test with valid()).
// ---------------------------------------------------------------------------
class Entity {
public:
    Entity() = default;
    Entity(Scene* s, entt::entity id) : scene_(s), id_(id) {}

    bool valid() const;
    void destroy();

    // --- Component access (defined after Scene below) ---

    template<class T, class... Args> T& add(Args&&... args);
    template<class T>                T& get();
    template<class T>                T* try_get();
    template<class T>                bool has() const;
    template<class T>                void remove();

    // Raw EnTT id — use for interop with the registry escape hatch.
    entt::entity id() const { return id_; }

    bool operator==(const Entity& o) const { return scene_ == o.scene_ && id_ == o.id_; }
    bool operator!=(const Entity& o) const { return !(*this == o); }

private:
    Scene*       scene_ = nullptr;
    entt::entity id_    = entt::null;
};

// ---------------------------------------------------------------------------
// Scene — owns the EnTT registry and provides a friendly entity API.
// ---------------------------------------------------------------------------
// Callback fired by CollisionSystem for each overlapping BoxCollider pair.
using CollisionCallback = std::function<void(Entity, Entity)>;

class Scene {
public:
    // Create an anonymous entity.
    Entity create_entity();

    // Create an entity with a Name component already attached.
    Entity create_entity(const std::string& name);

    // Find the first entity with a matching Name component. Returns invalid
    // entity if not found.
    Entity find(const std::string& name);

    // Iterate every entity that has all of Comps. fn receives (Entity, Comps&...).
    template<class... Comps, class Fn>
    void each(Fn&& fn);

    // Run built-in systems (Lifetime, Movement, Collision, etc.).
    void run_systems(float dt);

    // Register a callback invoked once per overlapping BoxCollider pair per frame.
    // Replaces any previously registered callback. Pass nullptr to clear.
    void on_collision(CollisionCallback cb) { collision_cb_ = std::move(cb); }

    // Render all Sprite+Transform entities.
    void render(Renderer& r);

    // Destroy all entities and components.
    void clear();

    // Raw registry access — escape hatch for advanced use.
    entt::registry& registry() { return registry_; }

private:
    entt::registry   registry_;
    CollisionCallback collision_cb_;
};

// ---------------------------------------------------------------------------
// Entity template method bodies — defined here after Scene is complete.
// ---------------------------------------------------------------------------

inline bool Entity::valid() const {
    return scene_ != nullptr && scene_->registry().valid(id_);
}

inline void Entity::destroy() {
    if (valid()) { scene_->registry().destroy(id_); id_ = entt::null; }
}

template<class T, class... Args>
T& Entity::add(Args&&... args) {
    return scene_->registry().emplace<T>(id_, std::forward<Args>(args)...);
}

template<class T>
T& Entity::get() { return scene_->registry().get<T>(id_); }

template<class T>
T* Entity::try_get() { return scene_->registry().try_get<T>(id_); }

template<class T>
bool Entity::has() const { return scene_->registry().all_of<T>(id_); }

template<class T>
void Entity::remove() { scene_->registry().remove<T>(id_); }

// ---------------------------------------------------------------------------
// Scene template method bodies.
// ---------------------------------------------------------------------------

template<class... Comps, class Fn>
void Scene::each(Fn&& fn) {
    registry_.view<Comps...>().each(std::forward<Fn>(fn));
}

} // namespace glyph
