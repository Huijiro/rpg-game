#ifndef GDEXTENSION_INTERACTABLE_H
#define GDEXTENSION_INTERACTABLE_H

#include <godot_cpp/classes/area3d.hpp>

using godot::Area3D;

class Unit;

class Interactable : public Area3D {
  GDCLASS(Interactable, Area3D)

 protected:
  static void _bind_methods();

 public:
  Interactable();
  ~Interactable();

  bool can_interact(Unit* unit) const;
  void interact(Unit* unit);
};

#endif  // GDEXTENSION_INTERACTABLE_H
