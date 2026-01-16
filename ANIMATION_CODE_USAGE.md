# Animation System - Code Usage Guide

This document explains how to work with the animation system from a C++ code perspective.

---

## Overview

The animation system consists of:

1. **AnimationController** - Component that plays animations
2. **Signal Emitters** - Components that emit state-change signals
3. **Signal Receivers** - AnimationController listens and plays animations

**Code Flow**:
```
Component detects state change
  ↓
emit_signal("signal_name", data)
  ↓
Signal propagates to connected receivers (AnimationController)
  ↓
AnimationController.play_animation(name, speed)
  ↓
AnimationPlayer executes animation
```

---

## AnimationController (API Reference)

**Location**: `src/animation_controller.hpp`

### Class Definition

```cpp
class AnimationController : public UnitComponent {
  GDCLASS(AnimationController, UnitComponent)
  
  AnimationPlayer* animation_player = nullptr;
  
  void _ready() override;
  void play_animation(const StringName& name, float speed = 1.0f);
  
private:
  AnimationPlayer* find_animation_player(Node* node);
};
```

### Usage in Code

#### Creating an AnimationController

In Godot editor, add as a child node:
```
Unit (CharacterBody3D)
  └── AnimationController (Node)
```

Or in C++ (if needed):
```cpp
AnimationController* anim_ctrl = memnew(AnimationController);
unit->add_child(anim_ctrl);
```

#### Playing Animations Directly (Not Recommended)

While possible, direct calls should be rare. Signals are preferred:

```cpp
// Get AnimationController reference
AnimationController* anim_ctrl = Object::cast_to<AnimationController>(
    unit->get_node("AnimationController"));

if (anim_ctrl) {
  // Play animation at normal speed
  anim_ctrl->play_animation("walk");
  
  // Play animation at 1.5x speed
  anim_ctrl->play_animation("attack", 1.5f);
}
```

#### Playing Animations via Signals (Recommended)

Components should emit signals instead of calling AnimationController directly:

```cpp
// In your component
void MyComponent::some_event() {
  // Do game logic
  // ...
  
  // Emit signal for AnimationController to handle animation
  emit_signal("my_event", my_value);
}
```

AnimationController listens via editor signal connections.

---

## Emitting Signals (Component API)

### Adding a New Signal

**In Header** (`my_component.hpp`):
```cpp
class MyComponent : public UnitComponent {
  GDCLASS(MyComponent, UnitComponent)
  
protected:
  static void _bind_methods();
  
  // ... other code ...
};
```

**In Implementation** (`my_component.cpp`):
```cpp
void MyComponent::_bind_methods() {
  // Bind existing methods first
  ClassDB::bind_method(D_METHOD("my_method"), &MyComponent::my_method);
  
  // Add signal (new)
  ADD_SIGNAL(godot::MethodInfo("my_event",
                               PropertyInfo(Variant::FLOAT, "value")));
}

void MyComponent::my_method() {
  // When something happens:
  emit_signal("my_event", 5.0f);
  
  // Signal definition required ↑
  // Connected in editor → AnimationController.play_animation()
}
```

### Signal Best Practices

**Do**:
- Name signals descriptively: `movement_started`, `attack_finished`, `damage_taken`
- Pass relevant data: speeds, amounts, targets
- Emit at logical state changes
- Document what signal means in code comments

**Don't**:
- Include "animation" in signal name: `animation_started` ❌ → `movement_started` ✅
- Emit every frame if not necessary
- Pass huge objects as signal parameters
- Call animation methods directly from component

---

## Component Signal Reference

### MovementComponent Signals

**Location**: `src/movement_component.hpp`

```cpp
// Signal emitted when unit starts moving
signal movement_started()

// Signal emitted when unit stops moving
signal movement_stopped()
```

**When Emitted**:
```cpp
// In process_movement()
bool is_moving = (velocity.length() > 0.01f);

if (is_moving && !was_moving) {
  emit_signal(StringName("movement_started"));
} else if (!is_moving && was_moving) {
  emit_signal(StringName("movement_stopped"));
}

was_moving = is_moving;
```

**Typical Usage**:
```cpp
// Editor connects:
// movement_started() → AnimationController.play_animation("walk", 1.0)
// movement_stopped() → AnimationController.play_animation("idle", 1.0)

// Result: Animations play when unit moves/stops
```

**Code Example - Movement Tracking**:
```cpp
// If you need to detect movement in another component:
Unit* unit = get_unit();
if (unit) {
  MovementComponent* movement = Object::cast_to<MovementComponent>(
      unit->get_node("MovementComponent"));
  
  if (movement && movement->is_at_destination()) {
    // Unit stopped moving
  }
}
```

---

### HealthComponent Signals

**Location**: `src/health_component.hpp`

```cpp
// Emitted when health value changes
signal health_changed(current: float, max: float)

// Emitted when unit dies
signal died(source: Object)

// Emitted when damage is applied
signal damage_taken(amount: float)
```

**When Emitted**:
```cpp
// In apply_damage()
bool HealthComponent::apply_damage(float amount, Object* source) {
  // Apply damage
  current_health = std::max(0.0f, current_health - amount);
  
  // Emit damage taken (NEW)
  emit_signal("damage_taken", amount);
  
  // Emit health changed
  emit_signal("health_changed", current_health, max_health);
  
  // If died, emit death
  if (current_health <= 0.0f) {
    emit_signal("died", source);
    return true;
  }
  return false;
}
```

**Typical Usage**:
```cpp
// Editor connects:
// damage_taken(float) → AnimationController.play_animation("hit_react", 1.0)
// died(Object) → AnimationController.play_animation("death", 0.5)

// Result: Hit reaction and death animations play automatically
```

**Code Example - Health Monitoring**:
```cpp
// Listen to health changes in custom code:
Unit* unit = get_unit();
if (unit) {
  HealthComponent* health = unit->get_health_component();
  
  if (health) {
    // Connect to health_changed signal
    health->connect(
        StringName("health_changed"),
        Callable(this, StringName("_on_health_changed")));
  }
}

void MyComponent::_on_health_changed(float current, float max) {
  // Update UI, trigger events, etc.
  UtilityFunctions::print("Health: " + String::num(current) + "/" + String::num(max));
}
```

---

### AttackComponent Signals

**Location**: `src/attack_component.hpp`

```cpp
// Emitted when attack windup starts
signal attack_started(target: Object)

// Emitted when attack point reached (damage applies)
signal attack_point_reached(target: Object)

// Emitted when attack hits target
signal attack_hit(target: Object, damage: float)

// Emitted with speed multiplier for animation sync
signal attack_speed_changed(speed_multiplier: float)
```

**When Emitted**:
```cpp
// In try_fire_at()
bool AttackComponent::try_fire_at(Unit* target, double delta) {
  if (!in_attack_windup && time_until_next_attack <= 0.0) {
    in_attack_windup = true;
    attack_windup_timer = 0.0;
    current_attack_target = target;
    
    // Emit attack started
    emit_signal("attack_started", target);
    
    // Emit speed multiplier for animation sync (NEW)
    float speed_multiplier = base_attack_time / get_attack_interval();
    emit_signal("attack_speed_changed", speed_multiplier);
    
    return false;
  }
  return false;
}
```

**Speed Multiplier Calculation**:
```cpp
// get_attack_interval() returns actual attack time based on attack_speed stat
float get_attack_interval() const {
  float attack_speed_factor = attack_speed / 100.0f;  // 100 = 1.0x
  return base_attack_time / attack_speed_factor;
}

// At 100% attack speed:
// speed_multiplier = 1.7 / 1.7 = 1.0 (normal speed)

// At 120% attack speed:
// speed_multiplier = 1.7 / 1.416 = 1.2 (20% faster)

// This multiplier is passed to animation for sync
```

**Typical Usage**:
```cpp
// Editor connects:
// attack_started(Object) → AnimationController.play_animation("attack", 1.0)
// attack_speed_changed(float) → AnimationController.play_animation("attack", <multiplier>)

// Result: Attack animation plays and speeds up with attack_speed stat
```

**Code Example - Attack Monitoring**:
```cpp
// Listen to attacks in custom code:
Unit* unit = get_unit();
if (unit) {
  AttackComponent* attack = Object::cast_to<AttackComponent>(
      unit->get_component_by_class("AttackComponent"));
  
  if (attack) {
    attack->connect(
        StringName("attack_hit"),
        Callable(this, StringName("_on_attack_hit")));
  }
}

void MyComponent::_on_attack_hit(Object* target, float damage) {
  UtilityFunctions::print("Hit for " + String::num(damage) + " damage!");
  // Trigger VFX, sounds, hit freeze, etc.
}
```

---

## Common Usage Patterns

### Pattern 1: Simple State Animation

**Goal**: Play animation when state changes

```cpp
// In component
void MyComponent::_physics_process(double delta) {
  bool my_condition = check_something();
  
  if (my_condition != was_condition) {
    if (my_condition) {
      emit_signal("condition_met");  // Play animation in editor
    } else {
      emit_signal("condition_cleared");  // Play different animation
    }
    was_condition = my_condition;
  }
}

void MyComponent::_bind_methods() {
  ADD_SIGNAL(godot::MethodInfo("condition_met"));
  ADD_SIGNAL(godot::MethodInfo("condition_cleared"));
}
```

**Editor Setup**:
```
condition_met() → play_animation("active", 1.0)
condition_cleared() → play_animation("inactive", 1.0)
```

### Pattern 2: Speed-Synced Animation

**Goal**: Animation speed matches game mechanic

```cpp
// In component
void MyComponent::update_speed(float new_speed) {
  current_speed = new_speed;
  
  // Emit normalized speed for animation
  float speed_multiplier = new_speed / base_speed;
  emit_signal("speed_changed", speed_multiplier);
}

void MyComponent::_bind_methods() {
  ADD_SIGNAL(godot::MethodInfo("speed_changed",
                               PropertyInfo(Variant::FLOAT, "multiplier")));
}
```

**Editor Setup**:
```
speed_changed(float) → play_animation("walk", <signal_arg_0>)
```

**Result**: Walk animation plays faster/slower as speed changes

### Pattern 3: Data-Driven Animation Selection

**Goal**: Choose animation based on event data

```cpp
// In component
void MyComponent::trigger_reaction(int reaction_type) {
  // reaction_type: 0=hit, 1=stun, 2=knockback
  
  StringName anim_name;
  switch (reaction_type) {
    case 0: anim_name = StringName("hit_react"); break;
    case 1: anim_name = StringName("stun"); break;
    case 2: anim_name = StringName("knockback"); break;
    default: anim_name = StringName("idle");
  }
  
  emit_signal("reaction_triggered", anim_name);
}

void MyComponent::_bind_methods() {
  ADD_SIGNAL(godot::MethodInfo("reaction_triggered",
                               PropertyInfo(Variant::STRING, "animation_name")));
}
```

**Editor Setup**:
```
reaction_triggered(String) → [custom GDScript receives string and plays animation]
```

**Note**: For simple string passing, you might need a GDScript bridge since signal parameters need to match function parameters.

---

## Debugging Animation Signals

### Checking Signal Emission

**Enable Debug Output**:
```cpp
// AnimationController prints every animation:
// [Animation] Playing: walk @ 1.0x
// [Animation] Playing: attack @ 1.2x

// Run in editor and watch output console
```

**Manual Signal Testing**:
```cpp
// Emit signal directly from code for testing
emit_signal("movement_started");

// Or connect and test in editor:
// Select component → Node tab → right-click signal → Emit
```

### Verifying Signal Connections

```cpp
// Check if component has signal defined
if (Component::has_signal(StringName("my_signal"))) {
  UtilityFunctions::print("Signal exists!");
} else {
  UtilityFunctions::print("Signal NOT defined!");
}

// Get list of connected callables
TypedArray<Signal> signals = component->get_signal_list();
for (int i = 0; i < signals.size(); i++) {
  Signal sig = signals[i];
  UtilityFunctions::print("Signal: " + String(sig));
}
```

### Common Errors

**Error**: "Attempt to emit signal that doesn't exist"
```
Solution: Make sure signal is defined in _bind_methods() with ADD_SIGNAL
```

**Error**: "Signal not connected" (silent failure)
```
Solution: Check Node tab in editor to verify connection exists
```

**Error**: "AnimationController not found"
```cpp
// Make sure component has reference
if (!animation_player) {
  UtilityFunctions::push_error("No AnimationPlayer found!");
}
```

---

## Implementation Examples

### Example 1: Custom Action Component

```cpp
// my_action_component.hpp
class MyActionComponent : public UnitComponent {
  GDCLASS(MyActionComponent, UnitComponent)
  
protected:
  static void _bind_methods();
  
  bool is_casting = false;
  double cast_timer = 0.0;
  double cast_duration = 2.0;
  
public:
  void start_cast();
  void cancel_cast();
  void _physics_process(double delta) override;
};

// my_action_component.cpp
void MyActionComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("start_cast"), &MyActionComponent::start_cast);
  ClassDB::bind_method(D_METHOD("cancel_cast"), &MyActionComponent::cancel_cast);
  
  ADD_SIGNAL(godot::MethodInfo("cast_started"));
  ADD_SIGNAL(godot::MethodInfo("cast_finished"));
  ADD_SIGNAL(godot::MethodInfo("cast_cancelled"));
}

void MyActionComponent::start_cast() {
  if (!is_casting) {
    is_casting = true;
    cast_timer = 0.0;
    emit_signal("cast_started");  // Play cast animation
  }
}

void MyActionComponent::_physics_process(double delta) {
  if (is_casting) {
    cast_timer += delta;
    
    if (cast_timer >= cast_duration) {
      is_casting = false;
      emit_signal("cast_finished");  // Play cast finish animation
    }
  }
}

void MyActionComponent::cancel_cast() {
  if (is_casting) {
    is_casting = false;
    emit_signal("cast_cancelled");  // Play cancel animation
  }
}
```

**Editor Signal Setup**:
```
cast_started() → play_animation("spell_cast", 1.0)
cast_finished() → play_animation("spell_impact", 0.8)
cast_cancelled() → play_animation("spell_cancel", 1.2)
```

### Example 2: Status Effect Component

```cpp
// status_effect_component.hpp
class StatusEffectComponent : public UnitComponent {
  GDCLASS(StatusEffectComponent, UnitComponent)
  
protected:
  static void _bind_methods();
  
  struct Effect {
    StringName name;
    double duration;
    double timer;
  };
  
  Vector<Effect> active_effects;
  
public:
  void apply_effect(const StringName& effect_name, double duration);
  void _physics_process(double delta) override;
};

// status_effect_component.cpp
void StatusEffectComponent::_bind_methods() {
  ClassDB::bind_method(D_METHOD("apply_effect", "name", "duration"),
                       &StatusEffectComponent::apply_effect);
  
  ADD_SIGNAL(godot::MethodInfo("effect_applied",
                               PropertyInfo(Variant::STRING, "effect_name")));
  ADD_SIGNAL(godot::MethodInfo("effect_expired",
                               PropertyInfo(Variant::STRING, "effect_name")));
}

void StatusEffectComponent::apply_effect(const StringName& effect_name,
                                         double duration) {
  Effect e;
  e.name = effect_name;
  e.duration = duration;
  e.timer = 0.0;
  active_effects.push_back(e);
  
  // Signal: Play effect animation (e.g., "frozen", "burning", "stunned")
  emit_signal("effect_applied", String(effect_name));
}

void StatusEffectComponent::_physics_process(double delta) {
  for (int i = active_effects.size() - 1; i >= 0; i--) {
    active_effects[i].timer += delta;
    
    if (active_effects[i].timer >= active_effects[i].duration) {
      StringName expired = active_effects[i].name;
      active_effects.remove_at(i);
      
      // Signal: Effect expired, play removal animation
      emit_signal("effect_expired", String(expired));
    }
  }
}
```

**Editor Signal Setup**:
```
effect_applied(String) → [complex, might need GDScript bridge]
effect_expired(String) → [complex, might need GDScript bridge]

// Alternative: Use enum instead of String for animation type
```

---

## Best Practices

### ✅ Do This

```cpp
// Emit descriptive signals
emit_signal("movement_started");
emit_signal("attack_finished", damage_amount);
emit_signal("effect_triggered", effect_name);

// Let editor wire animations
// Signal → AnimationController.play_animation()

// Pass meaningful data
emit_signal("damage_taken", 25.5f);  // Amount of damage

// Track state for signal emission
if (is_moving && !was_moving) {
  emit_signal("movement_started");
}
was_moving = is_moving;
```

### ❌ Don't Do This

```cpp
// Don't call AnimationController directly in game logic
anim_controller->play_animation("walk");  // ❌

// Don't couple components
if (other_component->some_state) { ... }  // ❌

// Don't pass huge objects
emit_signal("my_signal", entire_unit);  // ❌

// Don't emit every frame unnecessarily
for (int i = 0; i < frames; i++) {
  emit_signal("every_frame");  // ❌
}
```

---

## Summary

**For Component Developers**:
1. Define signals in `_bind_methods()` using `ADD_SIGNAL()`
2. Emit signals when state changes: `emit_signal("signal_name", data)`
3. Keep signals fire-and-forget (don't expect a response)
4. Pass relevant data with signal (speeds, amounts, names)

**For Scene Setup**:
1. Create AnimationPlayer with animations
2. Add AnimationController component
3. Connect signals to `play_animation()` in editor
4. Specify animation name and speed in binds

**For Animation Synchronization**:
1. Calculate multipliers: `game_value / base_value`
2. Emit multiplier in signal
3. Pass multiplier as speed parameter to animation
4. AnimationPlayer handles playback speed adjustment

---

## Reference

- **AnimationController**: `src/animation_controller.hpp`
- **MovementComponent**: `src/movement_component.hpp`
- **HealthComponent**: `src/health_component.hpp`
- **AttackComponent**: `src/attack_component.hpp`
- **Editor Guide**: See `ANIMATION_SYSTEM.md`
- **Quick Start**: See `ANIMATION_QUICK_START.md`
