#ifndef GDEXTENSION_UNIT_COMPONENT_H
#define GDEXTENSION_UNIT_COMPONENT_H

#include <godot_cpp/classes/node.hpp>

using godot::Node;

class Unit;

class UnitComponent : public Node {
  GDCLASS(UnitComponent, Node)

 protected:
  static void _bind_methods();

  Unit* owner_unit = nullptr;

 public:
  UnitComponent();
  ~UnitComponent();

  void _ready() override;

  Unit* get_unit() const;
};

#endif  // GDEXTENSION_UNIT_COMPONENT_H
