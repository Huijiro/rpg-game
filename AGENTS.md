# RPG Game - Development Guide for AI Agents

This document provides comprehensive guidance for AI agents working on this Godot 4.5 C++ GDExtension project.

## Project Overview

**RPG Game** is a MOBA-style action RPG built with:
- **Engine**: Godot 4.5.1
- **Primary Language**: C++ (GDExtension)
- **Build System**: CMake + Ninja
- **Scripting**: GDScript (minimal, used for scene extensions)

### Core Architecture

The project uses a GDExtension architecture where C++ classes are registered with Godot:
- **Unit**: Character entity with navigation and movement
- **InputManager**: Input handling and click-to-move mechanics
- **MOBACamera**: Isometric camera system following the player

## Building

### Prerequisites
- CMake 3.30+
- Ninja build system
- C++17 compatible compiler (GCC/Clang)
- Godot 4.5.1

### Build Commands

This repo uses an **in-source CMake build** (build files live in the repo root).
Avoid using `cmake -B build` unless you intentionally want a separate build dir,
as it will force a fresh build of `libs/godot-cpp` in that new directory.

**Debug Build (Recommended for Development)**:
```bash
cmake -S . -DCMAKE_BUILD_TYPE=Debug
ninja
```

**Release Build (Optimized)**:
```bash
cmake -S . -DCMAKE_BUILD_TYPE=Release
ninja
```

**Clean Rebuild**:
```bash
rm -rf CMakeFiles build.ninja
cmake -S . -DCMAKE_BUILD_TYPE=Debug
ninja
```

### Output
- Built library: `GodotGame/lib/Linux-x86_64/libGodotGame-d.so` (debug)
- Built library: `GodotGame/lib/Linux-x86_64/libGodotGame.so` (release)

## Project Structure

```
rpg-game/
├── src/                          # C++ source files
│   ├── unit.hpp/.cpp             # Player character implementation
│   ├── input_manager.hpp/.cpp    # Input and click-to-move system
│   ├── moba_camera.hpp/.cpp      # Isometric camera controller
│   ├── beeper.hpp/.cpp           # Example/test class
│   ├── register_types.hpp/.cpp   # GDExtension registration
│   └── CMakeLists.txt
├── GodotGame/
│   ├── main.tscn                 # Main scene with all gameplay systems
│   ├── scenes/
│   │   └── click_indicator.tscn  # Click marker visual feedback
│   ├── lib/                       # Built .so files
│   └── assets/
│       └── textures/             # Game textures
├── libs/
│   └── godot-cpp/                # Godot C++ bindings (submodule)
├── CMakeLists.txt                # Root build configuration
└── AGENTS.md                      # This file

```

## Key Classes and Components

### Unit (src/unit.hpp/.cpp)

Character entity with navigation capabilities.

**Key Features**:
- Extends `CharacterBody3D`
- Uses `NavigationAgent3D` for pathfinding
- Supports configurable movement speed
- Instant rotation to face movement direction

**Properties**:
- `desired_location: Vector3` - Target position from input
- `speed: float` - Movement speed (default: 5.0)
- `rotation_speed: float` - Rotation speed (default: 10.0, unused - instant rotation)

**Methods**:
- `set_desired_location(Vector3)` - Set movement target
- `get_navigation_agent()` - Get navigation agent reference

**Implementation Notes**:
- Waits 3 frames after entering tree before using NavigationAgent3D
- Uses `is_navigation_finished()` to detect destination reached
- Only rotates around Y-axis (vertical axis)
- Velocity includes gravity component for falling

### InputManager (src/input_manager.hpp/.cpp)

Handles player input for click-to-move mechanics.

**Key Features**:
- Raycast-based ground detection
- Spawns customizable click indicator scenes
- Auto-discovers camera and unit if not set

**Properties**:
- `controlled_unit: Unit` - Character to control
- `camera: Camera3D` - Camera for raycasting
- `raycast_distance: float` - Max raycast distance (default: 1000.0)
- `click_indicator_scene: PackedScene` - Scene to spawn at click location

**Methods**:
- `set_controlled_unit(Unit*)` - Set character to control
- `set_camera(Camera3D*)` - Set camera for raycasting

**Implementation Notes**:
- Must add nodes to scene tree BEFORE setting global position
- Marker fades over 2 seconds before removal
- Raycast uses camera's world space for direction calculations

### MOBACamera (src/moba_camera.hpp/.cpp)

Isometric camera controller following the player unit.

**Key Features**:
- Direct horizontal following (no circular rotation)
- Smooth vertical movement
- Configurable pitch and height

**Properties**:
- `target: Node3D` - Node to follow
- `distance: float` - Horizontal distance (default: 10.0, currently unused)
- `height: float` - Height above target (default: 12.0)
- `follow_speed: float` - Follow speed multiplier (default: 15.0)
- `pitch_angle: float` - Camera pitch in degrees (default: 60.0)

**Methods**:
- `set_target(Node3D*)` - Set target to follow
- `set_pitch_angle(float)` - Set camera angle

**Implementation Notes**:
- Auto-creates Camera3D child if not present
- Moves directly to desired position horizontally each frame
- Only smooths vertical (Y) movement
- Looks at target with slight upward offset for better visibility
- Front of unit is -Z direction, camera positions at +Z

## Common Patterns and Best Practices

### Adding Debug Logging

Use Godot's logging system, NOT std::cout:

```cpp
#include <godot_cpp/variant/utility_functions.hpp>

using godot::UtilityFunctions;
using godot::String;

// Simple message
UtilityFunctions::print("Debug message");

// With values
UtilityFunctions::print("[Unit] Position: " + String::num(pos.x));
```

### Creating Node Hierarchies

**CRITICAL**: Add nodes to scene tree BEFORE setting global position:

```cpp
// WRONG - Will cause "not in tree" errors
marker->set_global_position(position);
parent->add_child(marker);

// CORRECT
parent->add_child(marker);
marker->set_global_position(position);
```

### Binding Methods and Properties

Always bind methods and properties in `_bind_methods()`:

```cpp
ClassDB::bind_method(D_METHOD("set_property", "value"), 
                     &Class::set_property);
ClassDB::bind_method(D_METHOD("get_property"), 
                     &Class::get_property);
ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "property"), 
             "set_property", "get_property");
```

### Safe Navigation Agent Access

Always check if agent is in tree before using it:

```cpp
if (!navigation_agent->is_inside_tree()) {
  return;  // Not ready yet
}
```

## Scene Setup

The main scene (`GodotGame/main.tscn`) contains:

```
Node3D (root)
├── NavigationRegion3D
│   ├── StaticBody3D (ground plane)
│   ├── Obstacle1, Obstacle2, Obstacle3 (red boxes)
├── Unit
│   ├── NavigationAgent3D
│   ├── MeshInstance3D (capsule)
│   └── CollisionShape3D
├── MOBACamera
│   └── Camera3D
├── InputManager
├── Camera3D (old, kept for reference)
└── DirectionalLight3D
```

### Navigation Mesh Setup

After adding/moving obstacles:
1. Select **NavigationRegion3D**
2. Click **"Bake Mesh"** in Inspector
3. Ensure obstacles are inside the NavigationRegion3D during baking
4. Unit must be OUTSIDE NavigationRegion3D to avoid creating holes in mesh

## Common Development Tasks

### Adding a New Property to Unit

1. Add member variable in `unit.hpp`
2. Add setter/getter methods
3. Bind in `_bind_methods()`
4. Use in `_physics_process()`

Example:
```cpp
// unit.hpp
float max_health = 100.0f;

// unit.cpp _bind_methods()
ClassDB::bind_method(D_METHOD("set_max_health", "health"), 
                     &Unit::set_max_health);
ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_health"), 
             "set_max_health", "get_max_health");
```

### Debugging Navigation Issues

If unit doesn't move:
1. Check navigation mesh is baked
2. Verify NavigationAgent3D is in tree
3. Ensure desired_location is set
4. Check obstacles don't block path
5. Wait 3+ frames for initialization

If unit floats:
1. Check CollisionShape3D offset
2. Verify Unit's Y position starts at ground level

## Troubleshooting

### Build Errors

**"No member named 'xxx' in namespace 'godot'"**
- Missing include file
- Add `#include <godot_cpp/classes/xxx.hpp>` or `#include <godot_cpp/variant/xxx.hpp>`

**"File overridden by precompiled header"**
- Need clean rebuild: `rm -rf CMakeFiles && cmake -S . && ninja`

**Compilation takes forever**
- First build is slow (generates 2000+ files)
- Subsequent builds are faster
- Use `-j` flag with ninja: `ninja -j 8`

### Runtime Errors

**"Condition '!is_inside_tree()' is true"**
- Node not added to scene tree yet
- Call `parent->add_child(node)` before using node methods
- Check proper order of operations

**"No navigation mesh"**
- Open scene in Godot editor
- Select NavigationRegion3D
- Click "Bake Mesh" button
- Ensure obstacles are in the region during baking

**Unit not moving**
- Check animation playing/speed set
- Verify click was on ground (not off-mesh)
- Ensure Unit can reach destination (no obstacles blocking)
- Check InputManager has camera and unit references

## Code Organization Tips

- Keep C++ implementation focused on logic and performance
- Use GDScript only for scene-specific behavior
- Register all public C++ classes in `register_types.cpp`
- Use properties (not direct member access) for editor exposure
- Always check node pointers for null before use
- Handle frame timing issues (waiting for tree readiness)

## Git Workflow

```bash
# Check status
git status

# Stage changes
git add .

# Commit with descriptive message
git commit -m "Add feature: description"

# View log
git log --oneline
```

## Performance Considerations

- Navigation queries happen every frame - monitor for bottlenecks
- Camera follows every frame - use direct assignment, not lerp
- Raycasts are relatively cheap but avoid in hot loops
- Consider caching navigation agent reference instead of finding each frame

## Resources and Documentation

- [Godot 4.5 Documentation](https://docs.godotengine.org/en/stable/)
- [Godot C++ Bindings](https://github.com/godotengine/godot-cpp)
- [Navigation in Godot](https://docs.godotengine.org/en/stable/usage/3d/using_3d_characters/using_3d_characters.html)
