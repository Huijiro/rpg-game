#include "register_types.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "animation_controller.hpp"
#include "attack_component.hpp"
#include "beeper.h"
#include "health_component.hpp"
#include "input_manager.hpp"
#include "interactable.hpp"
#include "match_manager.hpp"
#include "moba_camera.hpp"
#include "movement_component.hpp"
#include "projectile.hpp"
#include "resource_pool_component.hpp"
#include "test_movement.hpp"
#include "unit.hpp"
#include "unit_component.hpp"

using namespace godot;

void initialize_example_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }

  GDREGISTER_RUNTIME_CLASS(Beeper)
  GDREGISTER_CLASS(Unit)
  GDREGISTER_CLASS(Interactable)
  GDREGISTER_CLASS(InputManager)
  GDREGISTER_CLASS(MOBACamera)
  GDREGISTER_CLASS(MatchManager)
  GDREGISTER_CLASS(TestMovement)
  GDREGISTER_CLASS(UnitComponent)
  GDREGISTER_CLASS(MovementComponent)
  GDREGISTER_CLASS(HealthComponent)
  GDREGISTER_CLASS(ResourcePoolComponent)
  GDREGISTER_CLASS(AttackComponent)
  GDREGISTER_CLASS(AnimationController)
  GDREGISTER_CLASS(Projectile)
}

void uninitialize_example_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
}

extern "C" {
// Initialization.
GDExtensionBool GDE_EXPORT
GDExtensionInit(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                const GDExtensionClassLibraryPtr p_library,
                GDExtensionInitialization* r_initialization) {
  godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library,
                                                 r_initialization);

  init_obj.register_initializer(initialize_example_module);
  init_obj.register_terminator(uninitialize_example_module);
  init_obj.set_minimum_library_initialization_level(
      MODULE_INITIALIZATION_LEVEL_SCENE);

  return init_obj.init();
}
}
