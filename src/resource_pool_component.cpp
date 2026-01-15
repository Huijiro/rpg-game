#include "resource_pool_component.hpp"

#include <algorithm>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

using godot::ClassDB;
using godot::D_METHOD;
using godot::Engine;
using godot::PropertyInfo;
using godot::UtilityFunctions;
using godot::Variant;

ResourcePoolComponent::ResourcePoolComponent() = default;

ResourcePoolComponent::~ResourcePoolComponent() = default;

void ResourcePoolComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_pool_id", "id"),
                       &ResourcePoolComponent::set_pool_id);
  ClassDB::bind_method(D_METHOD("get_pool_id"),
                       &ResourcePoolComponent::get_pool_id);
  ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "pool_id"), "set_pool_id",
               "get_pool_id");

  ClassDB::bind_method(D_METHOD("set_max_value", "value"),
                       &ResourcePoolComponent::set_max_value);
  ClassDB::bind_method(D_METHOD("get_max_value"),
                       &ResourcePoolComponent::get_max_value);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_value"), "set_max_value",
               "get_max_value");

  ClassDB::bind_method(D_METHOD("set_current_value", "value"),
                       &ResourcePoolComponent::set_current_value);
  ClassDB::bind_method(D_METHOD("get_current_value"),
                       &ResourcePoolComponent::get_current_value);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "current_value"),
               "set_current_value", "get_current_value");

  ClassDB::bind_method(D_METHOD("can_spend", "amount"),
                       &ResourcePoolComponent::can_spend);
  ClassDB::bind_method(D_METHOD("try_spend", "amount"),
                       &ResourcePoolComponent::try_spend);
  ClassDB::bind_method(D_METHOD("restore", "amount"),
                       &ResourcePoolComponent::restore);

  ADD_SIGNAL(godot::MethodInfo("value_changed",
                               PropertyInfo(Variant::FLOAT, "current"),
                               PropertyInfo(Variant::FLOAT, "max")));
}

void ResourcePoolComponent::set_pool_id(StringName id) {
  pool_id = id;
}

StringName ResourcePoolComponent::get_pool_id() const {
  return pool_id;
}

void ResourcePoolComponent::set_max_value(float value) {
  max_value = std::max(0.0f, value);
  if (current_value > max_value) {
    current_value = max_value;
  }
  emit_signal("value_changed", current_value, max_value);
}

float ResourcePoolComponent::get_max_value() const {
  return max_value;
}

void ResourcePoolComponent::set_current_value(float value) {
  current_value = std::clamp(value, 0.0f, max_value);
  emit_signal("value_changed", current_value, max_value);
}

float ResourcePoolComponent::get_current_value() const {
  return current_value;
}

bool ResourcePoolComponent::can_spend(float amount) const {
  return current_value >= amount && amount >= 0.0f;
}

bool ResourcePoolComponent::try_spend(float amount) {
  if (!can_spend(amount)) {
    return false;
  }

  current_value -= amount;
  emit_signal("value_changed", current_value, max_value);
  return true;
}

void ResourcePoolComponent::restore(float amount) {
  if (amount < 0.0f) {
    amount = 0.0f;
  }

  current_value = std::min(max_value, current_value + amount);
  emit_signal("value_changed", current_value, max_value);
}
