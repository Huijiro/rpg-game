# Animation System - Quick Start Guide

## 5-Minute Setup

### 1. Open your Unit scene in Godot

Navigate to `GodotGame/unit.tscn` (or create a new unit scene)

### 2. Add AnimationPlayer

```
Right-click on Unit node
  → Add Child Node
  → Search "AnimationPlayer"
  → Create
```

### 3. Create some animations

```
Select AnimationPlayer node
  → Inspector
  → Hamburger menu (⋮) → New Animation
  → Name it "idle"
  → Create some keyframes or import an animation
  
Repeat for: walk, attack, death, hit_react
```

### 4. Add AnimationController component

```
Select Unit node
  → Add Child Node
  → Search "AnimationController"
  → Create
```

### 5. Wire signals in editor

```
Select AnimationController node
  → Node tab (next to Inspector)
  → Find "Signals"
  → Expand each component
  
For example:
  MovementComponent → movement_started()
    → Right-click
    → "Connect..."
    → Choose AnimationController
    → Choose play_animation(String, float)
    → Binds: animation_name = "walk", speed = 1.0
    → Click "Connect"
```

### 6. Repeat for other signals

```
MovementComponent.movement_stopped() → play_animation("idle", 1.0)
AttackComponent.attack_started(Object) → play_animation("attack", 1.0)
HealthComponent.died(Object) → play_animation("death", 0.5)
```

### 7. Test

```
Press Play
  → Walk your character around
  → Watch idle/walk animations play
  → Attack something
  → Watch attack animation play
```

Done! 🎉

---

## Key Signals by Component

### MovementComponent
- `movement_started()` → Play walk/run
- `movement_stopped()` → Play idle

### AttackComponent
- `attack_started(target)` → Play attack animation
- `attack_speed_changed(multiplier)` → Sync attack speed to animation

### HealthComponent
- `damage_taken(amount)` → Play hit reaction
- `died(source)` → Play death animation

---

## Speed Multiplier Pattern

For attack animations that need to sync with attack speed:

```
Bind TWO connections:

1. AttackComponent.attack_started(Object)
   → animation_name = "attack"
   → speed = 1.0

2. AttackComponent.attack_speed_changed(float)
   → animation_name = "attack"  
   → speed = <signal_arg_0>  // This passes the multiplier!

Result: Animation plays at speed matching your attack_speed stat
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Animation doesn't play | Check animation name in binds matches exactly |
| Wrong animation plays | Verify signal connection in Node tab |
| No signals showing | Rebuild project (Project → C++ → Build) |
| Animation too slow/fast | Adjust speed parameter in signal binds |
| AnimationController not found | Make sure you added child node, not just selected class |

---

## Animation Naming (Recommended)

```
Idle/Movement:
  - idle
  - walk
  - run

Combat:
  - attack_1
  - attack_2
  - hit_react

Death:
  - death
```

---

## Next Steps

1. **Add more animations** - Duplicate existing animations and modify
2. **Add animation events** - Use AnimationPlayer's Call Method track to sync attacks
3. **Add more signals** - Any component can emit signals, and AnimationController will play them
4. **Configure blending** - Set crossfade durations per animation in AnimationPlayer

See `ANIMATION_SYSTEM.md` for complete documentation.
