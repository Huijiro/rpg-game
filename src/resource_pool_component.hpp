#ifndef GDEXTENSION_RESOURCE_POOL_COMPONENT_H
#define GDEXTENSION_RESOURCE_POOL_COMPONENT_H

#include <godot_cpp/variant/string_name.hpp>

#include "unit_component.hpp"

using godot::StringName;

class ResourcePoolComponent : public UnitComponent {
  GDCLASS(ResourcePoolComponent, UnitComponent)

 protected:
  static void _bind_methods();

  StringName pool_id = "default";
  float max_value = 100.0f;
  float current_value = 100.0f;

 public:
  ResourcePoolComponent();
  ~ResourcePoolComponent();

  void set_pool_id(StringName id);
  StringName get_pool_id() const;

  void set_max_value(float value);
  float get_max_value() const;

  void set_current_value(float value);
  float get_current_value() const;

  bool can_spend(float amount) const;
  bool try_spend(float amount);
  void restore(float amount);
};

#endif  // GDEXTENSION_RESOURCE_POOL_COMPONENT_H
