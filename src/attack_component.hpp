#ifndef GDEXTENSION_ATTACK_COMPONENT_H
#define GDEXTENSION_ATTACK_COMPONENT_H

#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/property_info.hpp>

#include "unit_component.hpp"

using godot::List;
using godot::PackedScene;
using godot::PropertyInfo;
using godot::Ref;

enum class AttackDelivery { MELEE, PROJECTILE };

class AttackComponent : public UnitComponent {
  GDCLASS(AttackComponent, UnitComponent)

 protected:
  static void _bind_methods();

  // Attack stats
  float base_attack_time = 1.7f;  // BAT
  float attack_speed = 100.0f;    // IAS (100 = 1.0x)
  float attack_point = 0.3f;      // Seconds until damage/projectile release
  float attack_range = 2.5f;
  float attack_damage = 10.0f;
  AttackDelivery delivery_type = AttackDelivery::MELEE;
  float projectile_speed = 20.0f;

  // Timing state
  double time_until_next_attack = 0.0;
  double attack_windup_timer = 0.0;
  bool in_attack_windup = false;
  Unit* current_attack_target = nullptr;

 public:
  AttackComponent();
  ~AttackComponent();

  void _physics_process(double delta) override;

  // Properties
  void set_base_attack_time(float bat);
  float get_base_attack_time() const;

  void set_attack_speed(float speed);
  float get_attack_speed() const;

  void set_attack_point(float seconds);
  float get_attack_point() const;

  void set_attack_range(float range);
  float get_attack_range() const;

  void set_attack_damage(float damage);
  float get_attack_damage() const;

  void set_delivery_type(int type);
  int get_delivery_type() const;

  void set_projectile_speed(float speed);
  float get_projectile_speed() const;

  void set_projectile_scene(const Ref<PackedScene>& scene);
  Ref<PackedScene> get_projectile_scene() const;

  // Core logic
  bool try_fire_at(Unit* target, double delta);
  float get_attack_interval() const;

 private:
  Ref<PackedScene> projectile_scene = nullptr;

  void _fire_melee(Unit* target);
  void _fire_projectile(Unit* target);
};

#endif  // GDEXTENSION_ATTACK_COMPONENT_H
