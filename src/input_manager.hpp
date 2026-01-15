#ifndef GDEXTENSION_INPUT_MANAGER_H
#define GDEXTENSION_INPUT_MANAGER_H

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {
class Object;
class PhysicsDirectSpaceState3D;
class Node3D;
}  // namespace godot

using godot::Camera3D;
using godot::Color;
using godot::InputEvent;
using godot::Node;
using godot::PackedScene;
using godot::Ref;
using godot::ResourceLoader;
using godot::StandardMaterial3D;
using godot::String;
using godot::StringName;
using godot::Vector3;

class Unit;

class InputManager : public Node {
  GDCLASS(InputManager, Node)

 protected:
  static void _bind_methods();

 public:
  InputManager();
  ~InputManager();

  void _ready() override;
  void _input(const Ref<InputEvent>& event) override;
  void _process(double delta) override;

  void set_controlled_unit(Unit* unit);
  Unit* get_controlled_unit() const;

  void set_camera(Camera3D* camera);
  Camera3D* get_camera() const;

  void set_raycast_distance(float distance);
  float get_raycast_distance() const;

  void set_click_indicator_scene(const Ref<PackedScene>& scene);
  Ref<PackedScene> get_click_indicator_scene() const;

 private:
  // Helper methods
  bool _try_raycast(Vector3& out_position, godot::Object*& out_collider);
  void _show_click_marker(const Vector3& position);
  void _update_click_marker(double delta);

  // Member variables
  Unit* controlled_unit = nullptr;
  Camera3D* camera = nullptr;
  godot::PhysicsDirectSpaceState3D* physics_state = nullptr;
  float raycast_distance = 1000.0f;

  // Visual feedback
  godot::Node3D* click_marker = nullptr;
  Ref<StandardMaterial3D> marker_material = nullptr;
  Vector3 marker_target_position = Vector3(0, 0, 0);
  float marker_fade_timer = 0.0f;
  float marker_fade_duration = 2.0f;
  bool marker_active = false;
  Ref<PackedScene> click_indicator_scene = nullptr;
};

#endif  // GDEXTENSION_INPUT_MANAGER_H
