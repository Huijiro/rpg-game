#include "test_movement.hpp"

#include "unit.hpp"

#include <cmath>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/string.hpp>

using godot::ClassDB;
using godot::D_METHOD;
using godot::Engine;
using godot::Node;
using godot::Object;
using godot::PackedStringArray;
using godot::PropertyInfo;
using godot::RandomNumberGenerator;
using godot::String;
using godot::Variant;
using godot::Vector3;

TestMovement::TestMovement() = default;

TestMovement::~TestMovement() = default;

void TestMovement::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_enabled", "enabled"),
                       &TestMovement::set_enabled);
  ClassDB::bind_method(D_METHOD("get_enabled"), &TestMovement::get_enabled);
  ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled",
               "get_enabled");

  ClassDB::bind_method(D_METHOD("set_interval_seconds", "seconds"),
                       &TestMovement::set_interval_seconds);
  ClassDB::bind_method(D_METHOD("get_interval_seconds"),
                       &TestMovement::get_interval_seconds);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "interval_seconds"),
               "set_interval_seconds", "get_interval_seconds");

  ClassDB::bind_method(D_METHOD("set_wander_radius", "radius"),
                       &TestMovement::set_wander_radius);
  ClassDB::bind_method(D_METHOD("get_wander_radius"),
                       &TestMovement::get_wander_radius);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "wander_radius"),
               "set_wander_radius", "get_wander_radius");

  ClassDB::bind_method(D_METHOD("reset_origin"), &TestMovement::reset_origin);
  ClassDB::bind_method(D_METHOD("wander_once"), &TestMovement::wander_once);
}

void TestMovement::_ready() {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  set_physics_process(true);

  rng.instantiate();
  rng->randomize();

  reset_origin();
  time_until_next = interval_seconds;
}

void TestMovement::_physics_process(double delta) {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  if (!enabled) {
    return;
  }

  _ensure_origin();

  if (!has_origin || interval_seconds <= 0.0) {
    return;
  }

  time_until_next -= delta;
  if (time_until_next > 0.0) {
    return;
  }

  wander_once();
  time_until_next += interval_seconds;
}

PackedStringArray TestMovement::_get_configuration_warnings() const {
  if (_get_unit() == nullptr) {
    PackedStringArray warnings;
    warnings.push_back(
        String("TestMovement should be a child of a Unit node."));
    return warnings;
  }

  return PackedStringArray();
}

void TestMovement::set_enabled(bool new_enabled) {
  enabled = new_enabled;
}

bool TestMovement::get_enabled() const {
  return enabled;
}

void TestMovement::set_interval_seconds(double seconds) {
  interval_seconds = seconds;
  if (interval_seconds < 0.0) {
    interval_seconds = 0.0;
  }

  if (time_until_next > interval_seconds) {
    time_until_next = interval_seconds;
  }
}

double TestMovement::get_interval_seconds() const {
  return interval_seconds;
}

void TestMovement::set_wander_radius(double radius) {
  wander_radius = radius;
  if (wander_radius < 0.0) {
    wander_radius = 0.0;
  }
}

double TestMovement::get_wander_radius() const {
  return wander_radius;
}

void TestMovement::reset_origin() {
  Unit* unit = _get_unit();
  if (unit == nullptr || !unit->is_inside_tree()) {
    has_origin = false;
    return;
  }

  origin_position = unit->get_global_position();
  has_origin = true;
}

void TestMovement::wander_once() {
  Unit* unit = _get_unit();
  if (unit == nullptr || !unit->is_inside_tree()) {
    return;
  }

  if (!has_origin) {
    reset_origin();
    if (!has_origin) {
      return;
    }
  }

  if (wander_radius <= 0.0) {
    return;
  }

  if (rng.is_null()) {
    rng.instantiate();
    rng->randomize();
  }

  const double angle = rng->randf_range(0.0, 2.0 * Math_PI);
  const double distance = std::sqrt(rng->randf()) * wander_radius;

  Vector3 offset =
      Vector3(std::cos(angle) * distance, 0.0, std::sin(angle) * distance);

  Vector3 target = origin_position + offset;
  target.y = origin_position.y;

  unit->issue_move_order(target);
}

Unit* TestMovement::_get_unit() const {
  Node* parent = get_parent();
  if (parent == nullptr) {
    return nullptr;
  }

  return Object::cast_to<Unit>(parent);
}

void TestMovement::_ensure_origin() {
  if (has_origin) {
    return;
  }

  Unit* unit = _get_unit();
  if (unit == nullptr || !unit->is_inside_tree()) {
    return;
  }

  origin_position = unit->get_global_position();
  has_origin = true;
}
