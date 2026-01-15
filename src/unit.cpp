#include "unit.hpp"

#include "attack_component.hpp"
#include "health_component.hpp"
#include "interactable.hpp"

#include <cmath>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/navigation_agent3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector3.hpp>

using godot::Basis;
using godot::ClassDB;
using godot::D_METHOD;
using godot::Engine;
using godot::MethodInfo;
using godot::NavigationAgent3D;
using godot::Node;
using godot::Object;
using godot::PackedStringArray;
using godot::PropertyInfo;
using godot::String;
using godot::StringName;
using godot::Transform3D;
using godot::UtilityFunctions;
using godot::Variant;
using godot::Vector3;

Unit::Unit() = default;

Unit::~Unit() = default;

void Unit::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_navigation_agent"),
                       &Unit::get_navigation_agent);

  ClassDB::bind_method(D_METHOD("issue_move_order", "position"),
                       &Unit::issue_move_order);
  ClassDB::bind_method(D_METHOD("issue_attack_order", "target"),
                       &Unit::issue_attack_order);
  ClassDB::bind_method(D_METHOD("issue_interact_order", "target"),
                       &Unit::issue_interact_order);
  ClassDB::bind_method(D_METHOD("stop_order"), &Unit::stop_order);

  // Backwards compatible API; prefer issue_move_order going forward.
  ClassDB::bind_method(D_METHOD("set_desired_location", "location"),
                       &Unit::set_desired_location);
  ClassDB::bind_method(D_METHOD("get_desired_location"),
                       &Unit::get_desired_location);
  ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "desired_location"),
               "set_desired_location", "get_desired_location");

  ClassDB::bind_method(D_METHOD("set_speed", "speed"), &Unit::set_speed);
  ClassDB::bind_method(D_METHOD("get_speed"), &Unit::get_speed);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

  ClassDB::bind_method(D_METHOD("set_auto_attack_range", "range"),
                       &Unit::set_auto_attack_range);
  ClassDB::bind_method(D_METHOD("get_auto_attack_range"),
                       &Unit::get_auto_attack_range);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "auto_attack_range"),
               "set_auto_attack_range", "get_auto_attack_range");

  ClassDB::bind_method(D_METHOD("set_attack_buffer_range", "buffer"),
                       &Unit::set_attack_buffer_range);
  ClassDB::bind_method(D_METHOD("get_attack_buffer_range"),
                       &Unit::get_attack_buffer_range);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "attack_buffer_range"),
               "set_attack_buffer_range", "get_attack_buffer_range");

  ClassDB::bind_method(D_METHOD("set_faction_id", "faction_id"),
                       &Unit::set_faction_id);
  ClassDB::bind_method(D_METHOD("get_faction_id"), &Unit::get_faction_id);
  ADD_PROPERTY(PropertyInfo(Variant::INT, "faction_id"), "set_faction_id",
               "get_faction_id");

  ADD_SIGNAL(MethodInfo("order_changed",
                        PropertyInfo(Variant::INT, "previous_order"),
                        PropertyInfo(Variant::INT, "new_order"),
                        PropertyInfo(Variant::OBJECT, "target")));
}

void Unit::_ready() {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }
  _cache_navigation_agent();
  frame_count = 0;
}

void Unit::_physics_process(double delta) {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  // Safety check - both this node and navigation agent must be in tree to
  // function
  if (!is_inside_tree()) {
    return;
  }

  if (navigation_agent == nullptr) {
    _cache_navigation_agent();
  }
  if (navigation_agent == nullptr) {
    return;
  }

  // Ensure navigation agent is in tree before using it
  if (!navigation_agent->is_inside_tree()) {
    frame_count = 0;  // Reset counter
    return;
  }

  // Wait several frames after entering tree before using navigation
  if (!is_ready) {
    frame_count++;
    if (frame_count < 3) {
      return;
    }
    is_ready = true;
    _apply_navigation_target_distance();
  }

  if (current_order == OrderType::ATTACK) {
    if (attack_target == nullptr || !attack_target->is_inside_tree()) {
      stop_order();
    } else {
      // Check if target is dead
      HealthComponent* target_health = attack_target->get_health_component();
      if (target_health != nullptr && target_health->is_dead()) {
        stop_order();
      } else {
        const Vector3 target_pos = attack_target->get_global_position();
        desired_location = target_pos;

        // Hard stop when within attack range, but keep facing target.
        Vector3 current_position = get_global_position();
        Vector3 to_target = target_pos - current_position;
        to_target.y = 0.0f;
        const float distance_to_target = to_target.length();

        // Get attack range from AttackComponent
        AttackComponent* attack_comp = get_attack_component();
        float effective_attack_range = auto_attack_range;
        if (attack_comp != nullptr) {
          effective_attack_range = attack_comp->get_attack_range();
        }

        // Hysteresis: stop when within range, but resume only when far enough
        // away. This prevents jitter from continuous start/stop cycles.
        const float resume_distance =
            effective_attack_range + attack_buffer_range;
        if (distance_to_target <= effective_attack_range) {
          if (distance_to_target > 0.001f) {
            _face_horizontal_direction(to_target / distance_to_target);
          }

          // Keep nav target updated so we resume chasing if target moves.
          Vector3 current_target = navigation_agent->get_target_position();
          if (!current_target.is_equal_approx(desired_location)) {
            navigation_agent->set_target_position(desired_location);
          }

          // Attempt attack
          if (attack_comp != nullptr) {
            attack_comp->try_fire_at(attack_target, delta);
          } else {
            UtilityFunctions::push_error(
                "[Unit] ATTACK order requires AttackComponent");
            stop_order();
            move_and_slide();
            return;
          }

          // Stop horizontal movement but keep vertical velocity (gravity).
          set_velocity(Vector3(0, get_velocity().y, 0));
          move_and_slide();
          return;
        } else if (distance_to_target <= resume_distance) {
          // In the buffer zone: keep moving to create hysteresis.
          // This prevents the unit from oscillating around attack range.
          if (distance_to_target > 0.001f) {
            _face_horizontal_direction(to_target / distance_to_target);
          }

          Vector3 current_target = navigation_agent->get_target_position();
          if (!current_target.is_equal_approx(desired_location)) {
            navigation_agent->set_target_position(desired_location);
          }
        }
      }
    }
  } else if (current_order == OrderType::INTERACT) {
    if (interact_target == nullptr || !interact_target->is_inside_tree()) {
      stop_order();
    } else {
      desired_location = interact_target->get_global_position();
    }
  }

  Vector3 current_target = navigation_agent->get_target_position();

  // Only update if target position has changed
  if (!current_target.is_equal_approx(desired_location)) {
    navigation_agent->set_target_position(desired_location);
  }

  Vector3 current_position = get_global_position();
  Vector3 next_position = navigation_agent->get_next_path_position();
  Vector3 displacement = next_position - current_position;
  float distance = displacement.length();

  // If we're very close to the next position, we've reached the destination
  if (distance < 0.1f) {
    set_velocity(Vector3(0, get_velocity().y, 0));
    move_and_slide();
    return;
  }

  if (distance > 0.001f) {
    Vector3 direction = displacement / distance;
    Vector3 velocity = direction * speed;
    velocity.y = get_velocity().y;
    set_velocity(velocity);

    // Rotate unit to face movement direction instantly (Y axis only)
    _face_horizontal_direction(direction);
  } else {
    set_velocity(Vector3(0, get_velocity().y, 0));
  }
  move_and_slide();
}

void Unit::issue_move_order(const Vector3& position) {
  _clear_order_targets();
  _set_order(OrderType::MOVE, nullptr);
  desired_location = position;

  if (navigation_agent != nullptr && is_ready) {
    _apply_navigation_target_distance();
    navigation_agent->set_target_position(desired_location);
  }
}

void Unit::issue_attack_order(Unit* target) {
  _clear_order_targets();
  attack_target = target;
  _set_order(OrderType::ATTACK, target);

  if (navigation_agent != nullptr && is_ready && attack_target != nullptr &&
      attack_target->is_inside_tree()) {
    desired_location = attack_target->get_global_position();
    _apply_navigation_target_distance();
    navigation_agent->set_target_position(desired_location);
  }
}

void Unit::issue_interact_order(Interactable* target) {
  _clear_order_targets();
  interact_target = target;
  _set_order(OrderType::INTERACT, target);

  if (navigation_agent != nullptr && is_ready && interact_target != nullptr &&
      interact_target->is_inside_tree()) {
    desired_location = interact_target->get_global_position();
    _apply_navigation_target_distance();
    navigation_agent->set_target_position(desired_location);
  }
}

void Unit::stop_order() {
  _clear_order_targets();
  _set_order(OrderType::NONE, nullptr);

  // Stop horizontal movement but keep vertical velocity (gravity).
  set_velocity(Vector3(0, get_velocity().y, 0));
}

void Unit::set_desired_location(const Vector3& location) {
  issue_move_order(location);
}

Vector3 Unit::get_desired_location() const {
  return desired_location;
}

void Unit::set_speed(float new_speed) {
  speed = new_speed;
}

float Unit::get_speed() const {
  return speed;
}

void Unit::set_auto_attack_range(float new_range) {
  auto_attack_range = new_range;

  if (current_order == OrderType::ATTACK && navigation_agent != nullptr &&
      is_ready) {
    _apply_navigation_target_distance();
  }
}

float Unit::get_auto_attack_range() const {
  return auto_attack_range;
}

void Unit::set_attack_buffer_range(float new_buffer) {
  attack_buffer_range = new_buffer;
  if (attack_buffer_range < 0.0f) {
    attack_buffer_range = 0.0f;
  }
}

float Unit::get_attack_buffer_range() const {
  return attack_buffer_range;
}

void Unit::set_faction_id(int32_t new_faction_id) {
  faction_id = new_faction_id;
}

int32_t Unit::get_faction_id() const {
  return faction_id;
}

void Unit::_set_order(OrderType new_order, godot::Object* new_target) {
  OrderType previous_order = current_order;
  godot::Object* previous_target = current_order_target;

  current_order = new_order;
  current_order_target = new_target;

  if (previous_order != current_order ||
      previous_target != current_order_target) {
    emit_signal("order_changed", static_cast<int>(previous_order),
                static_cast<int>(current_order), current_order_target);
  }
}

void Unit::_apply_navigation_target_distance() {
  if (navigation_agent == nullptr || !is_ready) {
    return;
  }

  switch (current_order) {
    case OrderType::ATTACK:
      navigation_agent->set_target_desired_distance(auto_attack_range);
      break;
    case OrderType::MOVE:
    case OrderType::INTERACT:
    case OrderType::NONE:
    default:
      navigation_agent->set_target_desired_distance(0.0f);
      break;
  }
}

void Unit::_face_horizontal_direction(const Vector3& direction) {
  Vector3 horizontal_direction =
      Vector3(direction.x, 0, direction.z).normalized();
  if (horizontal_direction.length() <= 0.001f) {
    return;
  }

  float target_angle =
      std::atan2(-horizontal_direction.x, -horizontal_direction.z);

  float cos_a = std::cos(target_angle);
  float sin_a = std::sin(target_angle);
  Vector3 new_forward = Vector3(-sin_a, 0, -cos_a);
  Vector3 new_right = Vector3(cos_a, 0, -sin_a);
  Vector3 new_up = Vector3(0, 1, 0);

  Basis new_basis = Basis();
  new_basis.set_column(0, new_right);
  new_basis.set_column(1, new_up);
  new_basis.set_column(2, -new_forward);
  set_transform(Transform3D(new_basis, get_transform().origin));
}

void Unit::_clear_order_targets() {
  attack_target = nullptr;
  interact_target = nullptr;
}

PackedStringArray Unit::_get_configuration_warnings() const {
  if (_find_navigation_agent() == nullptr) {
    PackedStringArray warnings;
    warnings.push_back(String("Unit requires a NavigationAgent3D child node."));
    return warnings;
  }
  return PackedStringArray();
}

NavigationAgent3D* Unit::get_navigation_agent() const {
  return navigation_agent;
}

void Unit::_cache_navigation_agent() {
  navigation_agent = _find_navigation_agent();
}

NavigationAgent3D* Unit::_find_navigation_agent() const {
  const int32_t total_children = get_child_count();
  for (int32_t i = 0; i < total_children; ++i) {
    Node* child = get_child(i);
    if (child == nullptr) {
      continue;
    }
    if (auto agent = Object::cast_to<NavigationAgent3D>(child)) {
      return agent;
    }
  }
  return nullptr;
}

Node* Unit::get_component_by_class(const StringName& class_name) const {
  const int32_t total_children = get_child_count();
  for (int32_t i = 0; i < total_children; ++i) {
    Node* child = get_child(i);
    if (child == nullptr) {
      continue;
    }
    if (child->get_class() == class_name) {
      return child;
    }
  }
  return nullptr;
}

HealthComponent* Unit::get_health_component() const {
  Node* component = get_component_by_class("HealthComponent");
  return Object::cast_to<HealthComponent>(component);
}

AttackComponent* Unit::get_attack_component() const {
  Node* component = get_component_by_class("AttackComponent");
  return Object::cast_to<AttackComponent>(component);
}
