/// @file animation_controller.cpp
/// Fire-and-forget signal-driven animation controller implementation

#include "animation_controller.hpp"

#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "unit.hpp"

using godot::AnimationPlayer;
using godot::ClassDB;
using godot::D_METHOD;
using godot::Node;
using godot::String;
using godot::StringName;
using godot::UtilityFunctions;

AnimationController::AnimationController() = default;

AnimationController::~AnimationController() = default;

void AnimationController::_bind_methods() {
  // Bind play_animation method with optional speed parameter
  // Speed defaults to 1.0 if not specified
  ClassDB::bind_method(D_METHOD("play_animation", "name", "speed"),
                       &AnimationController::play_animation, DEFVAL(1.0f));
}

void AnimationController::_ready() {
  // Find AnimationPlayer recursively in the Unit's scene tree
  // Searches from Unit downward, returns first AnimationPlayer found
  animation_player = find_animation_player(get_unit());

  if (!animation_player) {
    UtilityFunctions::push_error(
        "[AnimationController] No AnimationPlayer found in Unit: " +
        String(get_unit() ? get_unit()->get_name() : "null"));
  }
}

void AnimationController::play_animation(const StringName& name, float speed) {
  // Silently ignore if AnimationPlayer not found
  // This allows components to emit signals even without animation setup
  if (!animation_player) {
    return;
  }

  // Debug output for animation troubleshooting
  // Format: [Animation] Playing: {name} @ {speed}x
  // Example: [Animation] Playing: attack_1 @ 1.2x
  UtilityFunctions::print("[Animation] Playing: " + String(name) + " @ " +
                          String::num(speed) + "x");

  // Play animation immediately
  // Godot's AnimationPlayer handles crossfading/blending based on configuration
  animation_player->play(name);

  // Apply speed multiplier
  // Used for animation speed sync with game mechanics (attack speed, movement
  // speed) Example: AttackComponent emits attack_speed_changed(1.2) Animation
  // plays 20% faster to match attack speed stat
  animation_player->set_speed_scale(speed);
}

AnimationPlayer* AnimationController::find_animation_player(Node* node) {
  // Recursive helper to search scene tree for AnimationPlayer
  // Searches depth-first, returns first match found

  if (!node) {
    return nullptr;
  }

  // Check if current node is AnimationPlayer
  // This allows AnimationPlayer to be direct child of Unit
  if (auto* ap = Object::cast_to<AnimationPlayer>(node)) {
    return ap;
  }

  // Recursively search children
  // This allows AnimationPlayer to be nested anywhere in scene tree
  // Example valid structures:
  //   Unit
  //   ├── AnimationPlayer (direct child) ✓
  //   │
  //   └── SomeNode
  //       └── AnimationPlayer (nested) ✓
  for (int i = 0; i < node->get_child_count(); i++) {
    if (auto* result = find_animation_player(node->get_child(i))) {
      return result;
    }
  }

  // Not found in this branch
  return nullptr;
}
