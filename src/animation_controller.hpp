/// @file animation_controller.hpp
/// @brief Animation playback controller for Unit animations
///
/// AnimationController is a component that bridges component signals to
/// AnimationPlayer. It receives fire-and-forget signal callbacks and plays
/// corresponding animations.
///
/// Usage:
///   1. Add AnimationController as child of Unit
///   2. Unit must have an AnimationPlayer child (searched recursively)
///   3. Connect component signals to play_animation() in editor:
///      - MovementComponent.movement_started() → play_animation("walk", 1.0)
///      - AttackComponent.attack_started() → play_animation("attack", 1.0)
///      - HealthComponent.died() → play_animation("death", 0.5)
///   4. No C++ code needed - animations play automatically via signals
///
/// Example Scene Structure:
///   Unit (CharacterBody3D)
///   ├── AnimationPlayer (animations: idle, walk, attack, death)
///   ├── MovementComponent
///   ├── HealthComponent
///   ├── AttackComponent
///   └── AnimationController ← discovers AnimationPlayer automatically
///
/// Speed Synchronization:
///   AttackComponent emits attack_speed_changed(multiplier) signal
///   AnimationController receives it as speed parameter to play_animation()
///   Result: Attack animation plays at speed matching attack_speed stat
///
/// See: ANIMATION_SYSTEM.md for complete documentation
/// See: ANIMATION_CODE_USAGE.md for code examples

#ifndef GDEXTENSION_ANIMATION_CONTROLLER_H
#define GDEXTENSION_ANIMATION_CONTROLLER_H

#include <godot_cpp/classes/animation_player.hpp>

#include "unit_component.hpp"

using godot::AnimationPlayer;

/// @class AnimationController
/// @brief Manages animation playback via signals
///
/// Fire-and-forget animation controller. Components emit signals on state
/// changes, AnimationController plays corresponding animations. No coupling
/// between animations and game logic.
class AnimationController : public UnitComponent {
  GDCLASS(AnimationController, UnitComponent)

 protected:
  static void _bind_methods();

  /// Cached reference to AnimationPlayer found in scene tree
  AnimationPlayer* animation_player = nullptr;

  /// Recursively search node tree for first AnimationPlayer
  /// @param node Starting node to search from
  /// @return AnimationPlayer if found, nullptr otherwise
  AnimationPlayer* find_animation_player(godot::Node* node);

 public:
  AnimationController();
  ~AnimationController();

  /// Initialize component and find AnimationPlayer
  void _ready() override;

  /// Play animation with optional speed multiplier
  /// @param name Animation name (e.g., "walk", "attack", "death")
  /// @param speed Playback speed multiplier (default: 1.0)
  ///              - 1.0 = normal speed
  ///              - 1.2 = 20% faster (used for attack speed sync)
  ///              - 0.8 = 20% slower
  ///
  /// Called via signal binds in editor. Prints debug output.
  /// Silently ignores if AnimationPlayer not found.
  void play_animation(const godot::StringName& name, float speed = 1.0f);
};

#endif  // GDEXTENSION_ANIMATION_CONTROLLER_H
