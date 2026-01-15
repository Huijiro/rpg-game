#include "input_manager.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include "interactable.hpp"
#include "unit.hpp"

using godot::ClassDB;
using godot::D_METHOD;
using godot::Dictionary;
using godot::Engine;
using godot::InputEventMouseButton;
using godot::MOUSE_BUTTON_RIGHT;
using godot::Node;
using godot::Node3D;
using godot::PhysicsRayQueryParameters3D;
using godot::PropertyInfo;
using godot::String;
using godot::UtilityFunctions;
using godot::Variant;
using godot::Vector2;

InputManager::InputManager() = default;

InputManager::~InputManager() {
  if (click_marker != nullptr) {
    click_marker->queue_free();
    click_marker = nullptr;
  }
}

void InputManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_controlled_unit", "unit"),
                       &InputManager::set_controlled_unit);
  ClassDB::bind_method(D_METHOD("get_controlled_unit"),
                       &InputManager::get_controlled_unit);

  ClassDB::bind_method(D_METHOD("set_camera", "camera"),
                       &InputManager::set_camera);
  ClassDB::bind_method(D_METHOD("get_camera"), &InputManager::get_camera);

  ClassDB::bind_method(D_METHOD("set_raycast_distance", "distance"),
                       &InputManager::set_raycast_distance);
  ClassDB::bind_method(D_METHOD("get_raycast_distance"),
                       &InputManager::get_raycast_distance);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "camera",
                            godot::PROPERTY_HINT_NODE_TYPE, "Camera3D"),
               "set_camera", "get_camera");

  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "raycast_distance"),
               "set_raycast_distance", "get_raycast_distance");

  ClassDB::bind_method(D_METHOD("set_click_indicator_scene", "scene"),
                       &InputManager::set_click_indicator_scene);
  ClassDB::bind_method(D_METHOD("get_click_indicator_scene"),
                       &InputManager::get_click_indicator_scene);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "click_indicator_scene",
                            godot::PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
               "set_click_indicator_scene", "get_click_indicator_scene");
}

void InputManager::_ready() {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  // Auto-find camera if not set
  if (camera == nullptr) {
    camera = Object::cast_to<Camera3D>(get_viewport()->get_camera_3d());
  }

  // Auto-find Unit if not set - search parent
  if (controlled_unit == nullptr) {
    Node* parent = get_parent();
    if (parent != nullptr) {
      controlled_unit = Object::cast_to<Unit>(parent);
    }
  }
}

void InputManager::_input(const Ref<InputEvent>& event) {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  if (controlled_unit == nullptr) {
    return;
  }

  // Check for right mouse button click
  auto mouse_event = Object::cast_to<InputEventMouseButton>(event.ptr());
  if (mouse_event == nullptr) {
    return;
  }

  if (mouse_event->get_button_index() != MOUSE_BUTTON_RIGHT) {
    return;
  }

  if (!mouse_event->is_pressed()) {
    return;
  }

  Vector3 click_position;
  godot::Object* clicked_object = nullptr;
  if (_try_raycast(click_position, clicked_object)) {
    if (auto clicked_unit = Object::cast_to<Unit>(clicked_object)) {
      if (clicked_unit == controlled_unit) {
        // Ignore right-clicks on the main unit itself.
        get_viewport()->set_input_as_handled();
        return;
      }

      // Allies: do nothing.
      if (clicked_unit->get_faction_id() == controlled_unit->get_faction_id()) {
        get_viewport()->set_input_as_handled();
        return;
      }

      // Enemies: issue ATTACK order (approach until in range).
      controlled_unit->issue_attack_order(clicked_unit);
      UtilityFunctions::print("[InputManager] Issued ATTACK order on: " +
                              String(clicked_unit->get_name()));
      get_viewport()->set_input_as_handled();
      return;
    }

    if (auto clicked_interactable =
            Object::cast_to<Interactable>(clicked_object)) {
      UtilityFunctions::print("[InputManager] Clicked Interactable: " +
                              String(clicked_interactable->get_name()));
      get_viewport()->set_input_as_handled();
      return;
    }

    // Default: treat as terrain/world click.
    controlled_unit->issue_move_order(click_position);
    _show_click_marker(click_position);
    get_viewport()->set_input_as_handled();
  }
}

void InputManager::_process(double delta) {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  _update_click_marker(delta);
}

void InputManager::set_controlled_unit(Unit* unit) {
  controlled_unit = unit;
}

Unit* InputManager::get_controlled_unit() const {
  return controlled_unit;
}

void InputManager::set_camera(Camera3D* cam) {
  camera = cam;
}

Camera3D* InputManager::get_camera() const {
  return camera;
}

void InputManager::set_raycast_distance(float distance) {
  raycast_distance = distance;
}

float InputManager::get_raycast_distance() const {
  return raycast_distance;
}

void InputManager::set_click_indicator_scene(const Ref<PackedScene>& scene) {
  click_indicator_scene = scene;
}

Ref<PackedScene> InputManager::get_click_indicator_scene() const {
  return click_indicator_scene;
}

bool InputManager::_try_raycast(Vector3& out_position,
                                godot::Object*& out_collider) {
  out_collider = nullptr;

  if (camera == nullptr) {
    return false;
  }

  // Get the current physics space from the camera (which is a Node3D)
  Node3D* camera_node = Object::cast_to<Node3D>(camera);
  if (camera_node == nullptr) {
    return false;
  }

  auto world = camera_node->get_world_3d();
  if (world == nullptr) {
    return false;
  }

  physics_state = world->get_direct_space_state();
  if (physics_state == nullptr) {
    return false;
  }

  // Get mouse position and convert to world space
  auto viewport = get_viewport();
  if (viewport == nullptr) {
    return false;
  }

  Vector2 mouse_pos = viewport->get_mouse_position();

  // Create ray from camera through mouse position
  Vector3 ray_from = camera->project_ray_origin(mouse_pos);
  Vector3 ray_normal = camera->project_ray_normal(mouse_pos);
  Vector3 ray_to = ray_from + (ray_normal * raycast_distance);

  // Setup raycast query
  Ref<PhysicsRayQueryParameters3D> query =
      PhysicsRayQueryParameters3D::create(ray_from, ray_to);
  query->set_collide_with_bodies(true);
  query->set_collide_with_areas(true);

  // Avoid hitting the player's own unit.
  if (controlled_unit != nullptr && controlled_unit->is_inside_tree()) {
    godot::Array exclude;
    exclude.push_back(controlled_unit->get_rid());
    query->set_exclude(exclude);
  }

  // Execute raycast
  Dictionary result = physics_state->intersect_ray(query);

  if (result.is_empty()) {
    out_collider = nullptr;
    return false;
  }

  out_position = result["position"];
  out_collider = result["collider"];
  return true;
}

void InputManager::_show_click_marker(const Vector3& position) {
  // Clean up old marker if it exists
  if (click_marker != nullptr) {
    click_marker->queue_free();
  }

  // Check if scene is set
  if (click_indicator_scene.is_null()) {
    return;  // No scene configured
  }

  // Instantiate the click indicator scene
  Node* marker_instance = click_indicator_scene->instantiate();
  auto marker = Object::cast_to<Node3D>(marker_instance);
  if (marker == nullptr) {
    return;  // Scene root is not a Node3D
  }

  // Add to scene FIRST, then set position
  Node* parent = get_parent();
  if (parent != nullptr) {
    parent->add_child(marker);
  }

  // Position at click location (after adding to tree)
  Vector3 marker_pos = position;
  marker_pos.y += 0.5f;
  marker->set_global_position(marker_pos);

  click_marker = marker;
  marker_active = true;
  marker_fade_timer = 0.0f;
}

void InputManager::_update_click_marker(double delta) {
  if (click_marker == nullptr) {
    return;
  }

  if (!marker_active) {
    return;
  }

  marker_fade_timer += delta;

  // Fade out the marker
  if (marker_fade_timer < marker_fade_duration) {
    float progress = marker_fade_timer / marker_fade_duration;
    float alpha = 1.0f - progress;  // Fade from 1 to 0

    if (marker_material.is_valid()) {
      godot::Color color = marker_material->get_albedo();
      color.a = alpha;
      marker_material->set_albedo(color);
    }
  } else {
    // Remove marker after fade is complete
    click_marker->queue_free();
    click_marker = nullptr;
    marker_material.unref();
    marker_active = false;
    marker_fade_timer = 0.0f;
  }
}
