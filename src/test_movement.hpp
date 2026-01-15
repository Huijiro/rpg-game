#ifndef GDEXTENSION_TEST_MOVEMENT_H
#define GDEXTENSION_TEST_MOVEMENT_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/vector3.hpp>

class Unit;

using godot::Node;
using godot::PackedStringArray;
using godot::Vector3;

class TestMovement : public Node {
  GDCLASS(TestMovement, Node)

 protected:
  static void _bind_methods();

 public:
  TestMovement();
  ~TestMovement();

  void _ready() override;
  void _physics_process(double delta) override;
  PackedStringArray _get_configuration_warnings() const override;

  void set_enabled(bool new_enabled);
  bool get_enabled() const;

  void set_interval_seconds(double seconds);
  double get_interval_seconds() const;

  void set_wander_radius(double radius);
  double get_wander_radius() const;

  void reset_origin();
  void wander_once();

 private:
  Unit* _get_unit() const;
  void _ensure_origin();

  bool enabled = true;
  double interval_seconds = 5.0;
  double wander_radius = 5.0;

  bool has_origin = false;
  Vector3 origin_position;
  double time_until_next = 0.0;

  godot::Ref<godot::RandomNumberGenerator> rng;
};

#endif  // GDEXTENSION_TEST_MOVEMENT_H
