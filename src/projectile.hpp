#ifndef GDEXTENSION_PROJECTILE_H
#define GDEXTENSION_PROJECTILE_H

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

using godot::Node3D;
using godot::Vector3;

class Unit;

class Projectile : public Node3D {
  GDCLASS(Projectile, Node3D)

 protected:
  static void _bind_methods();

  Unit* attacker = nullptr;
  Unit* target = nullptr;
  float damage = 0.0f;
  float speed = 20.0f;
  float hit_radius = 0.5f;  // "Close enough" distance

  Vector3 direction = Vector3(0, 0, 0);
  double travel_distance = 0.0;

 public:
  Projectile();
  ~Projectile();

  void _physics_process(double delta) override;

  // Setup projectile with attacker, target, damage, and speed
  void setup(Unit* attacker_unit,
             Unit* target_unit,
             float damage_amount,
             float travel_speed);

  void set_hit_radius(float radius);
  float get_hit_radius() const;
};

#endif  // GDEXTENSION_PROJECTILE_H
