#include "movement_component.hpp"

#include <cmath>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "health_component.hpp"
#include "unit.hpp"

using godot::Basis;
using godot::Callable;
using godot::ClassDB;
using godot::D_METHOD;
using godot::Node;
using godot::PropertyInfo;
using godot::StringName;
using godot::Transform3D;
using godot::Variant;
using godot::Vector3;

MovementComponent::MovementComponent() = default;

MovementComponent::~MovementComponent() = default;

void MovementComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_speed", "speed"),
                       &MovementComponent::set_speed);
  ClassDB::bind_method(D_METHOD("get_speed"), &MovementComponent::get_speed);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

  ClassDB::bind_method(D_METHOD("set_rotation_speed", "speed"),
                       &MovementComponent::set_rotation_speed);
  ClassDB::bind_method(D_METHOD("get_rotation_speed"),
                       &MovementComponent::get_rotation_speed);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rotation_speed"),
               "set_rotation_speed", "get_rotation_speed");

  ClassDB::bind_method(D_METHOD("is_at_destination"),
                       &MovementComponent::is_at_destination);

  // Bind signal callback method
  ClassDB::bind_method(D_METHOD("_on_owner_unit_died", "source"),
                       &MovementComponent::_on_owner_unit_died);

  // Add signals
  ADD_SIGNAL(godot::MethodInfo("movement_started"));
  ADD_SIGNAL(godot::MethodInfo("movement_stopped"));
}

void MovementComponent::_ready() {
  frame_count = 0;
  is_ready = false;

  // Connect to owner unit's health component death signal
  Unit* owner = get_owner_unit();
  if (owner != nullptr) {
    HealthComponent* health_comp = owner->get_health_component();
    if (health_comp != nullptr) {
      health_comp->connect(StringName("died"),
                           Callable(this, StringName("_on_owner_unit_died")));
    }
  }
}

void MovementComponent::set_speed(float new_speed) {
  speed = new_speed;
}

float MovementComponent::get_speed() const {
  return speed;
}

void MovementComponent::set_rotation_speed(float new_rotation_speed) {
  rotation_speed = new_rotation_speed;
}

float MovementComponent::get_rotation_speed() const {
  return rotation_speed;
}

Vector3 MovementComponent::process_movement(double delta,
                                            const Vector3& target_location,
                                            OrderType order) {
  // Safety checks
  Unit* owner = get_owner_unit();
  if (owner == nullptr || !owner->is_inside_tree()) {
    return Vector3(0, 0, 0);
  }

  // If owner unit is dead, don't move
  // Check if owner is valid and check its health status
  HealthComponent* health_comp = owner->get_health_component();
  if (health_comp != nullptr && health_comp->is_dead()) {
    return Vector3(0, 0, 0);
  }

  // Ensure this component (which IS the NavigationAgent3D) is in tree before
  // using it
  // Also check if we've been queued for deletion
  if (!is_inside_tree()) {
    frame_count = 0;
    return Vector3(0, 0, 0);
  }

  // Don't process if we're queued for deletion
  if (is_queued_for_deletion()) {
    return Vector3(0, 0, 0);
  }

  // Wait several frames after entering tree before using navigation
  if (!is_ready) {
    frame_count++;
    if (frame_count < 3) {
      return Vector3(0, 0, 0);
    }
    is_ready = true;
    _apply_navigation_target_distance(order);
  }

  // Update target distance based on order type
  _apply_navigation_target_distance(order);

  // Update navigation target position
  Vector3 current_target = get_target_position();
  if (!current_target.is_equal_approx(target_location)) {
    set_target_position(target_location);
  }

  // Get next path position and calculate velocity
  Vector3 current_position = owner->get_global_position();
  Vector3 next_position = get_next_path_position();
  Vector3 displacement = next_position - current_position;
  float distance = displacement.length();

  Vector3 velocity = Vector3(0, 0, 0);

  // Calculate direction to target for rotation
  Vector3 direction = Vector3(0, 0, 0);
  if (distance > 0.001f) {
    direction = displacement / distance;
    velocity = direction * speed;
  } else if (distance >= 0.0f) {
    // Calculate direction to the actual target (for rotation when near
    // destination)
    Vector3 to_target = target_location - current_position;
    to_target.y = 0.0f;
    float target_distance = to_target.length();
    if (target_distance > 0.001f) {
      direction = to_target / target_distance;
    }
  }

  // Always rotate to face the direction (whether moving or stopped)
  if (distance >= 0.001f || direction.length() > 0.001f) {
    _face_horizontal_direction(direction);
  }

  // Emit movement signals based on velocity state
  bool is_moving = (velocity.length() > 0.01f);
  if (is_moving && !was_moving) {
    emit_signal(StringName("movement_started"));
  } else if (!is_moving && was_moving) {
    emit_signal(StringName("movement_stopped"));
  }
  was_moving = is_moving;

  return velocity;
}

void MovementComponent::_face_horizontal_direction(const Vector3& direction) {
  Unit* owner = get_owner_unit();
  if (owner == nullptr || !owner->is_inside_tree()) {
    return;
  }

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
  owner->set_transform(Transform3D(new_basis, owner->get_transform().origin));
}

void MovementComponent::_apply_navigation_target_distance(OrderType order) {
  if (!is_ready || !is_inside_tree()) {
    return;
  }

  switch (order) {
    case OrderType::ATTACK: {
      Unit* owner = get_owner_unit();
      if (owner != nullptr && owner->is_inside_tree()) {
        // Get attack range from unit's attack component
        float attack_range = 2.5f;  // default
        // Note: We can't directly access AttackComponent here to avoid circular
        // dependency, but the Unit will handle attack range logic in its own
        // physics_process. This just sets a reasonable default.
        set_target_desired_distance(attack_range);
      }
      break;
    }
    case OrderType::MOVE:
    case OrderType::INTERACT:
    case OrderType::NONE:
    default:
      set_target_desired_distance(0.0f);
      break;
  }
}

bool MovementComponent::is_at_destination() const {
  // Cast away const since is_navigation_finished() isn't const but we just
  // query state
  return const_cast<MovementComponent*>(this)->is_navigation_finished();
}

Unit* MovementComponent::get_owner_unit() const {
  // Check if we're still in the tree - if not, parent might be invalid
  if (!is_inside_tree()) {
    return nullptr;
  }

  Node* parent = get_parent();
  if (parent == nullptr) {
    return nullptr;
  }
  return Object::cast_to<Unit>(parent);
}

void MovementComponent::_on_owner_unit_died(godot::Object* source) {
  // When the owner unit dies, queue this movement component for deletion
  // Dead units should not have a movement component
  if (is_inside_tree()) {
    queue_free();
  }
}
