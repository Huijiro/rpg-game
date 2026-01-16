# Animation System Design & Implementation Guide

## Overview

This document describes the animation system for the RPG Game. The system is built on **fire-and-forget signals** that components emit when state changes occur. Animations are completely decoupled from game logic and are wired together using Godot's native signal system and the editor.

**Core Philosophy**:
- Components emit descriptive signals about game events
- AnimationController receives signals and plays corresponding animations
- All signal-to-animation mappings are configured in the Godot editor
- No hardcoded animation names or logic in C++

---

## Architecture

```
Components (MovementComponent, AttackComponent, HealthComponent)
    ↓ (emit signals on state changes)
    ↓
Godot Signal System
    ↓ (signals connected in editor)
    ↓
AnimationController.play_animation(name, speed)
    ↓ (pure fire-and-forget)
    ↓
AnimationPlayer
    ├─ Plays animation at specified speed
    ├─ Handles crossfading/blending
    ├─ Runs animation event callbacks
    └─ Manages all animation state
```

---

## Components & Signals

### MovementComponent

**Location**: `src/movement_component.hpp/.cpp`

**Signals Emitted**:
- `movement_started()` - Emitted when unit starts moving
- `movement_stopped()` - Emitted when unit stops moving

**When Emitted**:
- `movement_started()` fires when velocity becomes non-zero
- `movement_stopped()` fires when velocity becomes zero and navigation finished

**Typical Animation Connections**:
```
movement_started() → play_animation("walk", 1.0)
movement_stopped() → play_animation("idle", 1.0)
```

---

### HealthComponent

**Location**: `src/health_component.hpp/.cpp`

**Signals Emitted**:
- `health_changed(current: float, max: float)` - Health value changed (existing)
- `died(source: Object)` - Unit died (existing)
- `damage_taken(amount: float)` - Damage was applied (NEW)

**When Emitted**:
- `damage_taken()` fires whenever `apply_damage()` is called
- Emitted before `health_changed` for synchronization

**Typical Animation Connections**:
```
damage_taken(float) → play_animation("hit_react", 1.0)
died(Object) → play_animation("death", 0.8)
```

---

### AttackComponent

**Location**: `src/attack_component.hpp/.cpp`

**Signals Emitted**:
- `attack_started(target: Object)` - Attack windup began
- `attack_point_reached(target: Object)` - Damage applied at animation keyframe
- `attack_hit(target: Object, damage: float)` - Final impact (existing)
- `attack_speed_changed(speed_multiplier: float)` - Speed multiplier for animation (NEW)

**When Emitted**:
- `attack_started()` fires when attack windup begins
- `attack_speed_changed()` fires immediately after `attack_started()` with calculated multiplier
- Speed multiplier = `base_attack_time / actual_attack_interval`
  - At 100% attack speed: multiplier = 1.0
  - At 120% attack speed: multiplier = 1.2 (20% faster)

**Typical Animation Connections**:
```
attack_started(Object) → play_animation("attack_1", <signal_value>)
attack_speed_changed(float) → [second parameter of play_animation]

// In editor:
// Signal: attack_speed_changed(float) → AnimationController.play_animation
// Parameters: animation_name = "attack", speed = <signal_arg_0>
```

---

## AnimationController Component

**Location**: `src/animation_controller.hpp/.cpp`
**Extends**: `UnitComponent`

### Purpose
- Finds AnimationPlayer in Unit's scene tree
- Receives signal callbacks and plays animations
- Applies speed multipliers for animation synchronization

### Public API

```cpp
void play_animation(const StringName& name, float speed = 1.0f)
```

**Parameters**:
- `name`: Animation name to play (e.g., "walk", "attack", "death")
- `speed`: Playback speed multiplier (default: 1.0)
  - 1.0 = normal speed
  - 1.2 = 20% faster
  - 0.8 = 20% slower

**Behavior**:
- Plays animation immediately via `animation_player->play(name)`
- Sets playback speed via `animation_player->set_speed_scale(speed)`
- Prints debug message: `[Animation] Playing: {name} @ {speed}x`
- Silently ignores if no AnimationPlayer found

### AnimationPlayer Discovery

AnimationController uses **recursive search** to find AnimationPlayer:
1. Searches starting from the Unit node
2. Recursively searches all children
3. Returns first AnimationPlayer found
4. Logs error if not found

**Scene Structure**:
```
Unit (CharacterBody3D)
├── MeshInstance3D
├── CollisionShape3D
│
├── AnimationPlayer ← Discovered here
│   └── Animations: idle, walk, attack, death, etc
│
├── MovementComponent
├── HealthComponent
├── AttackComponent
└── AnimationController (references AnimationPlayer)
```

---

## Setting Up Animations in Editor

### Step 1: Create AnimationPlayer

1. Select Unit node in scene
2. Add child node → AnimationPlayer
3. Create animations in AnimationPlayer:
   - Right-click AnimationPlayer → "New Animation"
   - Create: `idle`, `walk`, `attack`, `death`, etc.

### Step 2: Add AnimationController Component

1. Select Unit node
2. Add child node → Node (rename to AnimationController)
3. In Inspector → Attach Script → Select AnimationController class

### Step 3: Connect Signals (Editor Configuration)

1. Select AnimationController node
2. Go to **Node tab** (next to Inspector)
3. Click **Signals** dropdown
4. Find each component signal you want to trigger animation:

#### Example: Movement Animations

```
Source: MovementComponent
Signal: movement_started()
  ↓ Connect to AnimationController.play_animation(String, float)
  ↓ Binds: animation_name = "walk", speed = 1.0
```

```
Source: MovementComponent
Signal: movement_stopped()
  ↓ Connect to AnimationController.play_animation(String, float)
  ↓ Binds: animation_name = "idle", speed = 1.0
```

#### Example: Attack Animations (with Speed Sync)

```
Source: AttackComponent
Signal: attack_started(Object)
  ↓ Connect to AnimationController.play_animation(String, float)
  ↓ Binds: animation_name = "attack_1", speed = 1.0
```

```
Source: AttackComponent
Signal: attack_speed_changed(float)
  ↓ Connect to AnimationController.play_animation(String, float)
  ↓ Binds: animation_name = "attack_1", speed = <signal_arg_0>

// This passes the signal's speed_multiplier as the speed parameter
```

#### Example: Death Animation

```
Source: HealthComponent
Signal: died(Object)
  ↓ Connect to AnimationController.play_animation(String, float)
  ↓ Binds: animation_name = "death", speed = 0.5
```

### Step 4: Configure Crossfading

In AnimationPlayer, configure how animations blend:

1. Select AnimationPlayer node
2. Inspector → Look for animation properties
3. Each animation can have its own crossfade duration
4. Godot handles blending automatically after first animation

**Recommended Crossfade Durations**:
- `idle`: 0.2s (slow, smooth transition)
- `walk`: 0.15s (medium)
- `attack`: 0.05s (fast snap)
- `death`: 0.3s (slow, controlled fade)

---

## Animation Playback Behavior

### First Animation (Startup)

When a Unit spawns:
1. AnimationPlayer is created (no animation playing)
2. First signal fires (e.g., movement_stopped from idle state)
3. AnimationController calls `play_animation("idle", 1.0)`
4. **AnimationPlayer starts with 0 crossfade** (plays immediately)

### Subsequent Animations

After first animation:
- Godot's AnimationPlayer uses configured crossfade durations
- Smooth blending between animations
- No animation interruption

### Speed Scaling

When `play_animation(name, speed)` is called:
```cpp
animation_player->set_speed_scale(speed);
```

This multiplies the animation playback speed:
- `speed = 1.0` → normal playback
- `speed = 1.2` → plays 20% faster
- `speed = 0.8` → plays 20% slower

**Used for**:
- Attack animations sync to attack speed stat
- Movement animations sync to movement speed
- Any gameplay-driven animation timing

---

## Animation Event Callbacks (Godot Native)

Godot's AnimationPlayer supports **method call tracks** that fire at specific animation frames.

### Creating Animation Events

1. Open animation in AnimationPlayer editor
2. Add a new track → **Call Method**
3. Add keyframe at desired frame
4. Select target node (e.g., AttackComponent)
5. Choose method to call (e.g., `attempt_hit()`)

### Example: Attack Animation Event

```
Attack Animation Timeline
├── Frame 0: attack_started signal fires
├── Frame 15 (attack_point): Call Method on AttackComponent
│   └── Method: _fire_melee() or _fire_projectile()
├── Frame 30: PlaySound("hit.wav")
└── Frame 40: Animation ends
```

This keeps animation timing synchronized with game logic:
- Animation frame 15 = `attack_point` (0.3s)
- Damage applies exactly when animation keyframe triggers
- No desync between visual and damage

---

## Debugging Animation State

### Print Output

AnimationController prints every animation played:
```
[Animation] Playing: walk @ 1.0x
[Animation] Playing: attack_1 @ 1.2x
[Animation] Playing: death @ 0.5x
```

### Godot Inspector

While running:
1. Select AnimationPlayer node
2. Inspector shows:
   - Current animation playing
   - Playback speed
   - Progress in animation

### Signal Debugging

In signal connection panel:
- Shows which signals are connected
- Visualizes signal flow
- Can test signals manually by clicking them

---

## Animation Naming Conventions

No naming convention is enforced in code. However, we recommend:

**State Animations**:
- `idle` - Idle/standing still
- `walk` - Moving at normal speed
- `run` - Moving at high speed (optional)

**Action Animations**:
- `attack_1`, `attack_2`, etc. - Attack variations
- `cast_spell` - Spell casting
- `interact` - Interact with object

**Reaction Animations**:
- `hit_react` - Hit/damage reaction
- `knockback` - Knockback/stun reaction
- `death` - Death sequence

**Example Unit**:
```
Unit/AnimationPlayer
├── idle (default state)
├── walk (movement)
├── attack_1 (basic attack)
├── attack_2 (alternate attack)
├── hit_react (damage reaction)
├── death (death animation)
```

---

## Adding New Signals

To add a new signal that triggers animations:

### 1. Add Signal to Component

**Header (e.g., `my_component.hpp`)**:
```cpp
// Signal will be added in _bind_methods()
```

**Implementation (e.g., `my_component.cpp`)**:
```cpp
void MyComponent::_bind_methods() {
  // ... existing bindings ...
  
  ADD_SIGNAL(godot::MethodInfo("my_event",
                               PropertyInfo(Variant::FLOAT, "param")));
}

void MyComponent::some_method() {
  // When event occurs:
  emit_signal("my_event", value);
}
```

### 2. Connect in Editor

1. Select AnimationController
2. Node tab → Signals
3. Find MyComponent.my_event
4. Right-click → Connect
5. Select AnimationController.play_animation
6. Set animation_name and speed

### 3. Done!

Animation will play whenever signal fires. No code changes needed.

---

## Common Issues & Solutions

### "No AnimationPlayer found in Unit"

**Problem**: AnimationController can't find AnimationPlayer

**Solution**:
1. Select Unit node in scene
2. Add AnimationPlayer child node if missing
3. Make sure AnimationPlayer is not inside a subscene that hides it
4. Rebuild the project

### Animation Doesn't Play

**Problem**: Signal connected but animation doesn't play

**Solution**:
1. Check AnimationPlayer has the animation defined
2. Verify signal connection in Node tab (should show connection)
3. Check AnimationController has valid reference (check debug prints)
4. Verify animation name in connection binds matches exactly

### Animation Plays At Wrong Speed

**Problem**: Attack animation doesn't sync with attack timing

**Solution**:
1. Check `attack_speed_changed` signal is connected
2. Verify speed parameter is set to `<signal_arg_0>` (the multiplier)
3. Ensure animation duration matches expected attack_point timing
4. Adjust animation playback speed in AnimationPlayer editor

### Too Many Animation Connections

**Problem**: Scene tree cluttered with signal connections

**Solution**:
- This is normal! Each signal → animation mapping is a connection
- Use descriptive animation names to keep track
- Document which signals connect to which animations in a comment

---

## Performance Notes

- Signal emission is negligible (microseconds)
- AnimationPlayer playback is optimized by Godot engine
- Speed scaling via `set_speed_scale()` has no performance cost
- No state tracking in AnimationController (fire-and-forget only)

---

## Example: Complete Unit Setup

### Scene Structure
```
Unit (CharacterBody3D)
├── MeshInstance3D (character model)
├── CollisionShape3D
├── AnimationPlayer
│   ├── idle (0.5s, loops)
│   ├── walk (0.8s, loops)
│   ├── attack_1 (1.0s, no loop)
│   ├── death (1.5s, no loop)
│   └── hit_react (0.3s, no loop)
├── NavigationAgent3D
├── MovementComponent
├── HealthComponent
├── AttackComponent
└── AnimationController
```

### Signal Connections
```
MovementComponent.movement_started() 
  → AnimationController.play_animation("walk", 1.0)

MovementComponent.movement_stopped() 
  → AnimationController.play_animation("idle", 1.0)

AttackComponent.attack_started(Object) 
  → AnimationController.play_animation("attack_1", 1.0)

AttackComponent.attack_speed_changed(float) 
  → AnimationController.play_animation("attack_1", <signal_arg>)

HealthComponent.damage_taken(float) 
  → AnimationController.play_animation("hit_react", 1.0)

HealthComponent.died(Object) 
  → AnimationController.play_animation("death", 0.8)
```

### In-Game Flow
```
1. Unit spawns
   → movement_stopped fires
   → plays "idle" animation

2. Player right-clicks
   → movement_started fires
   → plays "walk" animation
   → Unit walks to destination

3. Unit reaches destination
   → movement_stopped fires
   → plays "idle" animation

4. Player attacks enemy
   → attack_started fires
   → attack_speed_changed fires with multiplier
   → plays "attack_1" at correct speed
   → At animation keyframe, damage applies
   → Enemy hit_react triggers
   → Enemy health reduced

5. Enemy dies
   → died signal fires
   → plays "death" animation
   → Unit stays on ground
```

---

## Summary

The animation system provides:
- ✅ Complete decoupling of animations from game logic
- ✅ Fire-and-forget signal architecture
- ✅ Editor-driven configuration (no code changes to rewire)
- ✅ Speed synchronization with gameplay mechanics
- ✅ Support for any animation type (skeletal, sprite, shader, etc)
- ✅ Native Godot AnimationPlayer integration
- ✅ Debug output for troubleshooting
- ✅ Flexible, extensible signal system

To use:
1. Create AnimationPlayer with animations
2. Add AnimationController component
3. Wire signals to animations in editor
4. Play game and watch animations trigger!

No C++ code needed to add or modify animations.
