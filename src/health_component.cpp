#include "health_component.hpp"

#include <algorithm>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include "unit.hpp"

using godot::ClassDB;
using godot::D_METHOD;
using godot::PropertyInfo;
using godot::UtilityFunctions;
using godot::Variant;

HealthComponent::HealthComponent() = default;

HealthComponent::~HealthComponent() = default;

void HealthComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_max_health", "value"),
                       &HealthComponent::set_max_health);
  ClassDB::bind_method(D_METHOD("get_max_health"),
                       &HealthComponent::get_max_health);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_health"), "set_max_health",
               "get_max_health");

  ClassDB::bind_method(D_METHOD("set_current_health", "value"),
                       &HealthComponent::set_current_health);
  ClassDB::bind_method(D_METHOD("get_current_health"),
                       &HealthComponent::get_current_health);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "current_health"),
               "set_current_health", "get_current_health");

  ClassDB::bind_method(D_METHOD("apply_damage", "amount", "source"),
                       &HealthComponent::apply_damage, nullptr);
  ClassDB::bind_method(D_METHOD("heal", "amount"), &HealthComponent::heal);
  ClassDB::bind_method(D_METHOD("is_dead"), &HealthComponent::is_dead);

  ADD_SIGNAL(godot::MethodInfo("health_changed",
                               PropertyInfo(Variant::FLOAT, "current"),
                               PropertyInfo(Variant::FLOAT, "max")));
  ADD_SIGNAL(
      godot::MethodInfo("died", PropertyInfo(Variant::OBJECT, "source")));
  ADD_SIGNAL(godot::MethodInfo("damage_taken",
                               PropertyInfo(Variant::FLOAT, "amount")));
}

void HealthComponent::set_max_health(float value) {
  max_health = std::max(0.0f, value);
  if (current_health > max_health) {
    current_health = max_health;
  }
  emit_signal("health_changed", current_health, max_health);
}

float HealthComponent::get_max_health() const {
  return max_health;
}

void HealthComponent::set_current_health(float value) {
  current_health = std::clamp(value, 0.0f, max_health);
  emit_signal("health_changed", current_health, max_health);

  if (current_health <= 0.0f) {
    emit_signal("died", nullptr);
  }
}

float HealthComponent::get_current_health() const {
  return current_health;
}

bool HealthComponent::apply_damage(float amount, godot::Object* source) {
  if (amount < 0.0f) {
    amount = 0.0f;
  }

  current_health = std::max(0.0f, current_health - amount);
  emit_signal("damage_taken", amount);
  emit_signal("health_changed", current_health, max_health);

  // Log damage
  if (owner_unit != nullptr) {
    UtilityFunctions::print(
        "[HealthComponent] " + owner_unit->get_name() + " took " +
        godot::String::num(amount) +
        " damage. HP: " + godot::String::num(current_health) + "/" +
        godot::String::num(max_health));
  } else {
    UtilityFunctions::print(
        "[HealthComponent] Took " + godot::String::num(amount) +
        " damage. HP: " + godot::String::num(current_health) + "/" +
        godot::String::num(max_health));
  }

  if (current_health <= 0.0f) {
    if (owner_unit != nullptr) {
      UtilityFunctions::print("[HealthComponent] " + owner_unit->get_name() +
                              " died!");
    } else {
      UtilityFunctions::print("[HealthComponent] Unit died!");
    }
    emit_signal("died", source);
    return true;  // Unit died
  }

  return false;  // Unit survived
}

void HealthComponent::heal(float amount) {
  if (amount < 0.0f) {
    amount = 0.0f;
  }

  current_health = std::min(max_health, current_health + amount);
  emit_signal("health_changed", current_health, max_health);
}

bool HealthComponent::is_dead() const {
  return current_health <= 0.0f;
}
