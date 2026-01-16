#ifndef GDEXTENSION_MOVEMENT_COMPONENT_H
#define GDEXTENSION_MOVEMENT_COMPONENT_H

#include <godot_cpp/classes/navigation_agent3d.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "unit_order.hpp"

using godot::NavigationAgent3D;
using godot::PackedStringArray;
using godot::Vector3;

// Forward declaration
class Unit;

class MovementComponent : public NavigationAgent3D {
  GDCLASS(MovementComponent, NavigationAgent3D)

 protected:
  static void _bind_methods();

  float speed = 5.0f;
  float rotation_speed = 10.0f;
  bool is_ready = false;
  int32_t frame_count = 0;
  bool was_moving = false;  // Track previous movement state for signal emission

  // Private helper methods
  void _face_horizontal_direction(const Vector3& direction);
  void _apply_navigation_target_distance(OrderType order);
  void _on_owner_unit_died(godot::Object* source);

 public:
  MovementComponent();
  ~MovementComponent();

  void _ready() override;

  // Properties
  void set_speed(float new_speed);
  float get_speed() const;

  void set_rotation_speed(float new_rotation_speed);
  float get_rotation_speed() const;

  // Core movement processing
  // Returns horizontal velocity (Y component is always 0)
  Vector3 process_movement(double delta,
                           const Vector3& target_location,
                           OrderType order);

  // Utility
  bool is_at_destination() const;

  // Get owner Unit for context (replaces get_component_by_class logic)
  Unit* get_owner_unit() const;
};

#endif  // GDEXTENSION_MOVEMENT_COMPONENT_H
