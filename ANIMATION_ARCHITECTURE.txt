================================================================================
                    ANIMATION SYSTEM ARCHITECTURE
================================================================================

┌─────────────────────────────────────────────────────────────────────────────┐
│                        GAME COMPONENT LAYER                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌──────────────────────┐  ┌──────────────────────┐  ┌─────────────────┐   │
│  │ MovementComponent    │  │ HealthComponent      │  │ AttackComponent │   │
│  ├──────────────────────┤  ├──────────────────────┤  ├─────────────────┤   │
│  │ Speed: 5.0           │  │ MaxHealth: 100       │  │ BaseAttackTime: │   │
│  │ Rotation: Instant    │  │ Current: 75          │  │ 1.7             │   │
│  │ Velocity: (2,0,3)    │  │ Status: Alive        │  │ AttackSpeed: 120│   │
│  ├──────────────────────┤  ├──────────────────────┤  ├─────────────────┤   │
│  │ SIGNALS:             │  │ SIGNALS:             │  │ SIGNALS:        │   │
│  │ • movement_started   │  │ • health_changed     │  │ • attack_started│   │
│  │ • movement_stopped   │  │ • damage_taken ◄─NEW │  │ • attack_point_ │   │
│  │                      │  │ • died               │  │   reached       │   │
│  │ Emitted when:        │  │                      │  │ • attack_hit    │   │
│  │ velocity changes     │  │ Emitted when:        │  │ • attack_speed_ │   │
│  │ from 0 or to 0       │  │ damage applied       │  │   changed ◄─NEW │   │
│  │                      │  │ or unit dies         │  │                 │   │
│  │                      │  │                      │  │ Emitted when:   │   │
│  │                      │  │                      │  │ attack windup   │   │
│  │                      │  │                      │  │ begins + speed  │   │
│  │                      │  │                      │  │ multiplier calc │   │
│  └──────────────────────┘  └──────────────────────┘  └─────────────────┘   │
│         │                           │                        │              │
│         │ emit_signal()             │ emit_signal()          │              │
│         │                           │                        │              │
└─────────┼───────────────────────────┼────────────────────────┼──────────────┘
          │                           │                        │
          │                           │                        │
┌─────────▼───────────────────────────▼────────────────────────▼──────────────┐
│                      GODOT SIGNAL SYSTEM                                     │
│                   (Configured in Editor UI)                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  Signal Definitions (Auto-discovered by Godot):                             │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │ MovementComponent::movement_started()                               │   │
│  │ MovementComponent::movement_stopped()                               │   │
│  │ HealthComponent::health_changed(current: float, max: float)        │   │
│  │ HealthComponent::damage_taken(amount: float)                       │   │
│  │ HealthComponent::died(source: Object)                              │   │
│  │ AttackComponent::attack_started(target: Object)                    │   │
│  │ AttackComponent::attack_point_reached(target: Object)              │   │
│  │ AttackComponent::attack_hit(target: Object, damage: float)         │   │
│  │ AttackComponent::attack_speed_changed(multiplier: float)           │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│  Signal Connections (Created in Node → Signals Tab):                        │
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │ movement_started()                                                   │   │
│  │   → AnimationController.play_animation("walk", 1.0)                 │   │
│  │                                                                      │   │
│  │ movement_stopped()                                                   │   │
│  │   → AnimationController.play_animation("idle", 1.0)                 │   │
│  │                                                                      │   │
│  │ attack_started(Object)                                               │   │
│  │   → AnimationController.play_animation("attack", 1.0)               │   │
│  │                                                                      │   │
│  │ attack_speed_changed(float) ◄─ KEY: Signal arg becomes param       │   │
│  │   → AnimationController.play_animation("attack", <signal_arg_0>)   │   │
│  │                              ↑                                      │   │
│  │                              Signal's multiplier (1.2) is          │   │
│  │                              automatically passed as speed param    │   │
│  │                                                                      │   │
│  │ damage_taken(float)                                                 │   │
│  │   → AnimationController.play_animation("hit_react", 1.0)           │   │
│  │                                                                      │   │
│  │ died(Object)                                                        │   │
│  │   → AnimationController.play_animation("death", 0.5)               │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────┬──────────────────────────────────────────────────────────────────┘
          │
          │ Godot calls connected Callables
          │
┌─────────▼──────────────────────────────────────────────────────────────────┐
│                    ANIMATION CONTROLLER COMPONENT                          │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  AnimationController (UnitComponent)                                      │
│  ├─ animation_player: AnimationPlayer* (discovered at _ready)           │
│  │                                                                        │
│  └─ play_animation(name: String, speed: float = 1.0)                   │
│     │                                                                     │
│     ├─ Prints debug: "[Animation] Playing: walk @ 1.0x"                │
│     │                                                                     │
│     ├─ Calls: animation_player->play(name)                             │
│     │         Godot starts playing named animation                      │
│     │                                                                     │
│     └─ Calls: animation_player->set_speed_scale(speed)                 │
│              Godot adjusts playback speed:                              │
│              • 1.0 = normal speed                                       │
│              • 1.2 = 20% faster (for attack speed sync)               │
│              • 0.8 = 20% slower                                        │
│                                                                            │
└─────────┬──────────────────────────────────────────────────────────────────┘
          │
          │ Calls AnimationPlayer methods
          │
┌─────────▼──────────────────────────────────────────────────────────────────┐
│                     ANIMATION PLAYER (Godot Native)                        │
├────────────────────────────────────────────────────────────────────────────┤
│                                                                            │
│  AnimationPlayer Properties:                                             │
│  ├─ Animations Resource                                                  │
│  │  ├─ idle       (0.5s, loops, 0.2s crossfade)                        │
│  │  ├─ walk       (0.8s, loops, 0.15s crossfade)                       │
│  │  ├─ attack_1   (1.0s, no loop, 0.05s crossfade)                     │
│  │  ├─ hit_react  (0.3s, no loop)                                      │
│  │  └─ death      (1.5s, no loop, 0.3s crossfade)                      │
│  │                                                                       │
│  └─ Playback State                                                       │
│     ├─ current_animation: StringName                                     │
│     ├─ speed_scale: float (1.0 = normal)                                │
│     ├─ is_playing: bool                                                  │
│     └─ progress: float (0.0 to 1.0)                                      │
│                                                                            │
│  Key Features:                                                            │
│  ├─ Crossfading: Smooth blend between animations                        │
│  ├─ Speed Scaling: Multiplies animation playback speed                  │
│  ├─ Animation Events: Call methods at specific frames                   │
│  ├─ Looping: Per-animation loop settings                                │
│  └─ Godot Optimization: Fully optimized by Godot engine                │
│                                                                            │
└────────────────────────────────────────────────────────────────────────────┘


================================================================================
                         EXECUTION FLOW EXAMPLE
================================================================================

Time: Player clicks "Move" button
─────────────────────────────────

1. Unit._physics_process(delta)
   ├─ MovementComponent.process_movement(delta) called
   │  ├─ Calculate velocity (non-zero)
   │  ├─ Check: is_moving=true, was_moving=false
   │  └─ EMIT: emit_signal("movement_started")
   │
   └─ Unit applies velocity to physics
   
2. Godot Signal System
   ├─ Signal: movement_started()
   ├─ Find connected Callables
   └─ Call: AnimationController.play_animation("walk", 1.0)
   
3. AnimationController.play_animation("walk", 1.0)
   ├─ Print: "[Animation] Playing: walk @ 1.0x"
   ├─ Call: animation_player->play("walk")
   └─ Call: animation_player->set_speed_scale(1.0)
   
4. Godot AnimationPlayer
   ├─ Find animation named "walk"
   ├─ Start playback from frame 0
   ├─ Set playback speed to 1.0x
   └─ Render animation each frame
   
5. Result: Character walks with walking animation playing


Time: Attack happens at 120% attack speed
──────────────────────────────────────────

1. Unit._physics_process(delta)
   ├─ Distance to target < attack_range
   ├─ AttackComponent.try_fire_at(target, delta) called
   │  ├─ Check: ready_to_attack=true
   │  ├─ Set: in_attack_windup=true
   │  ├─ EMIT: emit_signal("attack_started", target)
   │  ├─ Calculate: multiplier = 1.7 / 1.416 = 1.2
   │  └─ EMIT: emit_signal("attack_speed_changed", 1.2)
   │
   └─ Unit sets desired_location to target

2. Godot Signal System (Two signals)
   ├─ Signal 1: attack_started(target)
   │  └─ Call: AnimationController.play_animation("attack", 1.0)
   │
   └─ Signal 2: attack_speed_changed(1.2)
      └─ Call: AnimationController.play_animation("attack", 1.2)
              ↑ The float multiplier is passed as 'speed' parameter

3. AnimationController.play_animation("attack", 1.2)
   ├─ Print: "[Animation] Playing: attack @ 1.2x"
   ├─ Call: animation_player->play("attack")
   └─ Call: animation_player->set_speed_scale(1.2)
              ↑ AnimationPlayer plays 20% faster

4. Godot AnimationPlayer
   ├─ Play animation named "attack"
   ├─ Set playback speed to 1.2x (20% faster than normal)
   ├─ Timeline: 0ms → 833ms (normal 1000ms)
   ├─ At frame 12 (250ms): [Animation Event] Apply damage
   └─ Result: Animation completes faster, matches attack speed
   
5. Result: Attack animation is 20% faster to sync with 120% attack speed


Time: Unit takes damage
──────────────────────

1. AttackComponent._fire_melee(target)
   ├─ Get target's HealthComponent
   └─ Call: target_health->apply_damage(10.0, this)

2. HealthComponent.apply_damage(10.0, source)
   ├─ current_health -= 10.0
   ├─ EMIT: emit_signal("damage_taken", 10.0)
   ├─ EMIT: emit_signal("health_changed", 65, 100)
   └─ Return: false (didn't die)

3. Godot Signal System
   ├─ Signal: damage_taken(10.0)
   └─ Call: AnimationController.play_animation("hit_react", 1.0)

4. AnimationController.play_animation("hit_react", 1.0)
   ├─ Print: "[Animation] Playing: hit_react @ 1.0x"
   ├─ Call: animation_player->play("hit_react")
   └─ Call: animation_player->set_speed_scale(1.0)

5. Godot AnimationPlayer
   ├─ Blend from current animation (walk) to hit_react (0.15s crossfade)
   ├─ Play hit_react animation (0.3s)
   └─ On completion, return to previous state

6. Result: Character plays hit reaction animation


Time: Unit dies
───────────────

1. HealthComponent.apply_damage(200.0, killer)
   ├─ current_health = 0.0
   ├─ EMIT: emit_signal("damage_taken", 200.0)
   ├─ EMIT: emit_signal("health_changed", 0, 100)
   ├─ EMIT: emit_signal("died", killer)
   └─ Return: true (unit died)

2. Godot Signal System
   ├─ Signal: died(killer)
   └─ Call: AnimationController.play_animation("death", 0.5)

3. AnimationController.play_animation("death", 0.5)
   ├─ Print: "[Animation] Playing: death @ 0.5x"
   ├─ Call: animation_player->play("death")
   └─ Call: animation_player->set_speed_scale(0.5)
              ↑ Play at half speed for dramatic effect

4. Godot AnimationPlayer
   ├─ Blend to death animation (0.3s crossfade)
   ├─ Play death animation slowly (1.5s * 2 = 3.0s)
   ├─ [Optional] At frame end, keep pose (no loop)
   └─ Result: Dramatic slow-motion death

5. MovementComponent._on_owner_unit_died()
   └─ queue_free() - remove component when unit dies

6. Result: Character plays death animation and stays on ground


================================================================================
                         KEY DESIGN PRINCIPLES
================================================================================

1. FIRE-AND-FORGET
   Components emit signals ➜ Signal system handles routing ➜ No response expected
   
2. DECOUPLING
   Game logic doesn't know about animations
   Animations don't affect game logic
   Signal system is the bridge
   
3. EDITOR-DRIVEN
   All animation configuration in Godot editor
   No hardcoded animation names in C++
   Easy to change without code
   
4. SPEED SYNCHRONIZATION
   Game speed values ➜ Calculate multiplier ➜ Emit signal with multiplier
   AnimationController receives multiplier ➜ Passes to AnimationPlayer
   Result: Animation speed matches game mechanic speed
   
5. GODOT-NATIVE
   Uses Godot's native systems: signals, AnimationPlayer, editor
   No custom animation state machine
   Leverages Godot's optimization


================================================================================
                         COMPONENT HIERARCHY
================================================================================

Unit (CharacterBody3D)
│
├── MeshInstance3D
│   └── Character visual model
│
├── CollisionShape3D
│   └── Physics collider
│
├── NavigationAgent3D
│   └── Pathfinding agent
│
├── AnimationPlayer ◄─ DISCOVERED by AnimationController
│   ├── idle (animations)
│   ├── walk
│   ├── attack_1
│   ├── hit_react
│   └── death
│
├── MovementComponent (extends NavigationAgent3D)
│   ├── Emits: movement_started(), movement_stopped()
│   └── Calls: play_animation("walk"), play_animation("idle")
│
├── HealthComponent (extends UnitComponent)
│   ├── Emits: damage_taken(amount), died(source)
│   └── Calls: play_animation("hit_react"), play_animation("death")
│
├── AttackComponent (extends UnitComponent)
│   ├── Emits: attack_started(target), attack_speed_changed(multiplier)
│   └── Calls: play_animation("attack", multiplier)
│
└── AnimationController (extends UnitComponent) ◄─ THE HUB
    ├─ Listens to all signals
    ├─ Discovers AnimationPlayer
    └─ Calls: play_animation(name, speed)
       └─ Updates AnimationPlayer state


================================================================================
                            TYPICAL SCENE
================================================================================

GodotGame/main.tscn
│
└── Unit (player-controlled)
    ├── AnimationPlayer (walk, attack, death, etc)
    ├── MovementComponent (movement_started, movement_stopped)
    ├── HealthComponent (damage_taken, died)
    ├── AttackComponent (attack_speed_changed)
    └── AnimationController
        ├─ Signal: movement_started() → play_animation("walk", 1.0)
        ├─ Signal: movement_stopped() → play_animation("idle", 1.0)
        ├─ Signal: attack_started() → play_animation("attack", 1.0)
        ├─ Signal: attack_speed_changed(float) → play_animation("attack", <arg>)
        └─ Signal: died() → play_animation("death", 0.5)

All signal → animation connections happen in Godot editor
Zero code changes needed to add/modify animations


================================================================================
