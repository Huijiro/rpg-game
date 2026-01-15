#include "unit_component.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "unit.hpp"

using godot::ClassDB;
using godot::D_METHOD;
using godot::Engine;
using godot::Object;
using godot::String;
using godot::UtilityFunctions;

UnitComponent::UnitComponent() = default;

UnitComponent::~UnitComponent() = default;

void UnitComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_unit"), &UnitComponent::get_unit);
}

void UnitComponent::_ready() {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  // Validate parent is a Unit
  Node* parent = get_parent();
  owner_unit = Object::cast_to<Unit>(parent);

  if (owner_unit == nullptr) {
    UtilityFunctions::push_error(
        "[" + get_class() + "] must be a child of Unit, but parent is: " +
        (parent != nullptr ? parent->get_class() : String("null")));
  }
}

Unit* UnitComponent::get_unit() const {
  return owner_unit;
}
