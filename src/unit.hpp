#ifndef GDEXTENSION_UNIT_H
#define GDEXTENSION_UNIT_H

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {
class NavigationAgent3D;
class Object;
}  // namespace godot

using godot::CharacterBody3D;
using godot::PackedStringArray;
using godot::String;
using godot::Vector3;

class Interactable;

class Unit : public CharacterBody3D {
  GDCLASS(Unit, CharacterBody3D)

 protected:
  static void _bind_methods();

 public:
  enum class OrderType {
    NONE,
    MOVE,
    ATTACK,
    INTERACT,
  };

  Unit();
  ~Unit();

  void _ready() override;
  void _physics_process(double delta) override;
  PackedStringArray _get_configuration_warnings() const override;

  void issue_move_order(const Vector3& position);
  void issue_attack_order(Unit* target);
  void issue_interact_order(Interactable* target);
  void stop_order();

  void set_desired_location(const Vector3& location);
  Vector3 get_desired_location() const;

  void set_speed(float new_speed);
  float get_speed() const;

  void set_auto_attack_range(float new_range);
  float get_auto_attack_range() const;

  void set_attack_buffer_range(float new_buffer);
  float get_attack_buffer_range() const;

  void set_faction_id(int32_t new_faction_id);
  int32_t get_faction_id() const;

  godot::NavigationAgent3D* get_navigation_agent() const;

 private:
  void _cache_navigation_agent();
  godot::NavigationAgent3D* _find_navigation_agent() const;

  void _set_order(OrderType new_order, godot::Object* new_target);
  void _apply_navigation_target_distance();
  void _face_horizontal_direction(const Vector3& direction);

  void _clear_order_targets();

  godot::NavigationAgent3D* navigation_agent = nullptr;
  Vector3 desired_location = Vector3(0, 0, 0);

  OrderType current_order = OrderType::NONE;
  godot::Object* current_order_target = nullptr;
  Unit* attack_target = nullptr;
  Interactable* interact_target = nullptr;

  float speed = 5.0f;
  float rotation_speed = 10.0f;  // How fast the unit rotates to face direction
  float auto_attack_range = 2.5f;
  float attack_buffer_range = 0.5f;  // Hysteresis buffer for resuming chase
  int32_t faction_id = 0;

  bool is_ready = false;
  int32_t frame_count = 0;
};

#endif  // GDEXTENSION_UNIT_H
