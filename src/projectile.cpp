#include "projectile.hpp"

#include <cmath>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "health_component.hpp"
#include "unit.hpp"

using godot::ClassDB;
using godot::D_METHOD;
using godot::Engine;
using godot::Object;
using godot::PropertyInfo;
using godot::UtilityFunctions;
using godot::Variant;

Projectile::Projectile() = default;

Projectile::~Projectile() = default;

void Projectile::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_hit_radius", "radius"),
                       &Projectile::set_hit_radius);
  ClassDB::bind_method(D_METHOD("get_hit_radius"), &Projectile::get_hit_radius);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "hit_radius"), "set_hit_radius",
               "get_hit_radius");
}

void Projectile::_physics_process(double delta) {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  if (target == nullptr || !target->is_inside_tree()) {
    queue_free();
    return;
  }

  Vector3 current_pos = get_global_position();
  Vector3 target_pos = target->get_global_position();

  // Recompute direction each frame (target might be moving)
  Vector3 to_target = target_pos - current_pos;
  float distance_to_target = to_target.length();

  // Check if we've arrived (close enough)
  if (distance_to_target <= hit_radius) {
    // Check if target is still alive
    HealthComponent* target_health = Object::cast_to<HealthComponent>(
        target->get_component_by_class("HealthComponent"));

    if (target_health != nullptr && !target_health->is_dead()) {
      // Apply damage
      if (attacker != nullptr) {
        UtilityFunctions::print("[Projectile] " + attacker->get_name() +
                                "'s projectile hit " + target->get_name() +
                                " for " + godot::String::num(damage) +
                                " damage");
      }
      target_health->apply_damage(damage, attacker);
    } else if (target_health != nullptr && target_health->is_dead()) {
      if (attacker != nullptr) {
        UtilityFunctions::print("[Projectile] " + attacker->get_name() +
                                "'s projectile reached " + target->get_name() +
                                " but target was already dead");
      }
    }

    queue_free();
    return;
  }

  // Move towards target
  if (distance_to_target > 0.001f) {
    direction = to_target / distance_to_target;
    Vector3 velocity = direction * speed;
    set_global_position(current_pos + velocity * static_cast<float>(delta));
    travel_distance += speed * delta;
  }
}

void Projectile::setup(Unit* attacker_unit,
                       Unit* target_unit,
                       float damage_amount,
                       float travel_speed) {
  attacker = attacker_unit;
  target = target_unit;
  damage = damage_amount;
  speed = travel_speed;

  if (target != nullptr) {
    Vector3 start_pos = attacker_unit != nullptr
                            ? attacker_unit->get_global_position()
                            : get_global_position();
    Vector3 target_pos = target_unit->get_global_position();
    Vector3 to_target = target_pos - start_pos;
    float distance = to_target.length();

    if (distance > 0.001f) {
      direction = to_target / distance;
    }
  }
}

void Projectile::set_hit_radius(float radius) {
  hit_radius = std::max(0.0f, radius);
}

float Projectile::get_hit_radius() const {
  return hit_radius;
}
