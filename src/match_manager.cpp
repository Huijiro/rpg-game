#include "match_manager.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/property_info.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "input_manager.hpp"
#include "moba_camera.hpp"
#include "unit.hpp"

using godot::ClassDB;
using godot::D_METHOD;
using godot::Engine;
using godot::PropertyInfo;
using godot::UtilityFunctions;
using godot::Variant;

MatchManager::MatchManager() = default;

MatchManager::~MatchManager() = default;

void MatchManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("set_main_unit", "unit"),
                       &MatchManager::set_main_unit);
  ClassDB::bind_method(D_METHOD("get_main_unit"), &MatchManager::get_main_unit);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "main_unit",
                            godot::PROPERTY_HINT_NODE_TYPE, "Unit"),
               "set_main_unit", "get_main_unit");

  ClassDB::bind_method(D_METHOD("set_player_controller", "controller"),
                       &MatchManager::set_player_controller);
  ClassDB::bind_method(D_METHOD("get_player_controller"),
                       &MatchManager::get_player_controller);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "player_controller",
                            godot::PROPERTY_HINT_NODE_TYPE, "InputManager"),
               "set_player_controller", "get_player_controller");

  ClassDB::bind_method(D_METHOD("set_moba_camera", "camera"),
                       &MatchManager::set_moba_camera);
  ClassDB::bind_method(D_METHOD("get_moba_camera"),
                       &MatchManager::get_moba_camera);
  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "moba_camera",
                            godot::PROPERTY_HINT_NODE_TYPE, "MOBACamera"),
               "set_moba_camera", "get_moba_camera");
}

void MatchManager::_ready() {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  if (main_unit == nullptr) {
    UtilityFunctions::push_warning("[MatchManager] main_unit is not set.");
    return;
  }

  if (player_controller == nullptr) {
    UtilityFunctions::push_warning(
        "[MatchManager] player_controller is not set.");
  } else {
    player_controller->set_controlled_unit(main_unit);
  }

  if (moba_camera == nullptr) {
    UtilityFunctions::push_warning("[MatchManager] moba_camera is not set.");
  } else {
    moba_camera->set_target(main_unit);
  }
}

void MatchManager::set_main_unit(Unit* unit) {
  main_unit = unit;
}

Unit* MatchManager::get_main_unit() const {
  return main_unit;
}

void MatchManager::set_player_controller(InputManager* controller) {
  player_controller = controller;
}

InputManager* MatchManager::get_player_controller() const {
  return player_controller;
}

void MatchManager::set_moba_camera(MOBACamera* camera) {
  moba_camera = camera;
}

MOBACamera* MatchManager::get_moba_camera() const {
  return moba_camera;
}
