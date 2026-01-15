#include "interactable.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "unit.hpp"

using godot::ClassDB;
using godot::D_METHOD;
using godot::UtilityFunctions;

Interactable::Interactable() = default;

Interactable::~Interactable() = default;

void Interactable::_bind_methods() {
  ClassDB::bind_method(D_METHOD("can_interact", "unit"),
                       &Interactable::can_interact);
  ClassDB::bind_method(D_METHOD("interact", "unit"), &Interactable::interact);
}

bool Interactable::can_interact(Unit* unit) const {
  (void)unit;
  return true;
}

void Interactable::interact(Unit* unit) {
  (void)unit;
  UtilityFunctions::print("[Interactable] interact called on: " + get_name());
}
