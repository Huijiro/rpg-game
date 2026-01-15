#include "moba_camera.hpp"

#include <cmath>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using godot::ClassDB;
using godot::D_METHOD;
using godot::Engine;
using godot::PropertyInfo;
using godot::UtilityFunctions;
using godot::Variant;

MOBACamera::MOBACamera() {
  // Set better MOBA camera defaults
  distance = 10.0f;
  height = 12.0f;
  follow_speed = 15.0f;  // Higher speed = less smoothing, more direct following
  pitch_angle = 60.0f;   // Higher angle = more top-down view
}

MOBACamera::~MOBACamera() = default;

void MOBACamera::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_target", "target"),
                       &MOBACamera::set_target);
  ClassDB::bind_method(D_METHOD("get_target"), &MOBACamera::get_target);

  ClassDB::bind_method(D_METHOD("set_distance", "distance"),
                       &MOBACamera::set_distance);
  ClassDB::bind_method(D_METHOD("get_distance"), &MOBACamera::get_distance);

  ClassDB::bind_method(D_METHOD("set_height", "height"),
                       &MOBACamera::set_height);
  ClassDB::bind_method(D_METHOD("get_height"), &MOBACamera::get_height);

  ClassDB::bind_method(D_METHOD("set_follow_speed", "speed"),
                       &MOBACamera::set_follow_speed);
  ClassDB::bind_method(D_METHOD("get_follow_speed"),
                       &MOBACamera::get_follow_speed);

  ClassDB::bind_method(D_METHOD("set_pitch_angle", "angle"),
                       &MOBACamera::set_pitch_angle);
  ClassDB::bind_method(D_METHOD("get_pitch_angle"),
                       &MOBACamera::get_pitch_angle);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "target",
                            godot::PROPERTY_HINT_NODE_TYPE, "Node3D"),
               "set_target", "get_target");

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance"), "set_distance",
               "get_distance");

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height"), "set_height",
               "get_height");

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "follow_speed"), "set_follow_speed",
               "get_follow_speed");

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pitch_angle"), "set_pitch_angle",
               "get_pitch_angle");
}

void MOBACamera::_ready() {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  // Find Camera3D child if it exists
  camera = nullptr;
  for (int i = 0; i < get_child_count(); ++i) {
    Node* child = get_child(i);
    if (child != nullptr) {
      camera = Object::cast_to<Camera3D>(child);
      if (camera != nullptr) {
        break;
      }
    }
  }

  // If no camera found, create one
  if (camera == nullptr) {
    camera = memnew(Camera3D);
    add_child(camera);
  }

  // Snap immediately if a target is already set.
  if (target != nullptr && target->is_inside_tree()) {
    _update_camera_transform(0.0, true);
  }
}

void MOBACamera::_update_camera_transform(double delta, bool snap) {
  if (target == nullptr || camera == nullptr) {
    return;
  }

  if (!target->is_inside_tree()) {
    return;
  }

  // Calculate desired camera position based on target
  Vector3 target_pos = target->get_global_position();

  // Convert pitch angle to radians (pi/180)
  float pitch_rad = pitch_angle * 3.14159265f / 180.0f;

  // Calculate horizontal distance based on height and pitch
  float tan_pitch = std::tan(pitch_rad);
  float horizontal_dist =
      (std::abs(tan_pitch) > 0.0001f) ? height / tan_pitch : distance;

  // Position camera behind and above the target
  // We'll position it in the negative Z direction (behind)
  Vector3 camera_offset = Vector3(0, height, horizontal_dist);
  Vector3 desired_position = target_pos + camera_offset;

  Vector3 new_pos;
  if (snap) {
    new_pos = desired_position;
  } else {
    // Move camera directly to desired position - don't smooth horizontal
    // movement Only smooth vertical to prevent jerky height changes
    Vector3 current_pos = get_global_position();
    new_pos =
        Vector3(desired_position.x,  // Direct horizontal follow
                current_pos.y + (desired_position.y - current_pos.y) *
                                    follow_speed * delta,  // Smooth vertical
                desired_position.z  // Direct horizontal follow
        );
  }
  set_global_position(new_pos);

  // Look at the target with a slight offset upward so we see more of the world
  Vector3 look_target = target_pos + Vector3(0, 1.5f, 0);
  camera->look_at(look_target, Vector3(0, 1, 0));
}

void MOBACamera::_physics_process(double delta) {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  _update_camera_transform(delta, false);
}

void MOBACamera::set_target(Node3D* new_target) {
  target = new_target;

  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  if (is_inside_tree() && target != nullptr && target->is_inside_tree() &&
      camera != nullptr) {
    _update_camera_transform(0.0, true);
  }
}

Node3D* MOBACamera::get_target() const {
  return target;
}

void MOBACamera::set_distance(float new_distance) {
  distance = new_distance;
}

float MOBACamera::get_distance() const {
  return distance;
}

void MOBACamera::set_height(float new_height) {
  height = new_height;
}

float MOBACamera::get_height() const {
  return height;
}

void MOBACamera::set_follow_speed(float new_speed) {
  follow_speed = new_speed;
}

float MOBACamera::get_follow_speed() const {
  return follow_speed;
}

void MOBACamera::set_pitch_angle(float new_angle) {
  pitch_angle = new_angle;
}

float MOBACamera::get_pitch_angle() const {
  return pitch_angle;
}
