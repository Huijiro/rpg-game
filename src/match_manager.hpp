#ifndef GDEXTENSION_MATCH_MANAGER_H
#define GDEXTENSION_MATCH_MANAGER_H

#include <godot_cpp/classes/node.hpp>

using godot::Node;

class InputManager;
class MOBACamera;
class Unit;

class MatchManager : public Node {
  GDCLASS(MatchManager, Node)

 protected:
  static void _bind_methods();

 public:
  MatchManager();
  ~MatchManager();

  void _ready() override;

  void set_main_unit(Unit* unit);
  Unit* get_main_unit() const;

  void set_player_controller(InputManager* controller);
  InputManager* get_player_controller() const;

  void set_moba_camera(MOBACamera* camera);
  MOBACamera* get_moba_camera() const;

 private:
  Unit* main_unit = nullptr;
  InputManager* player_controller = nullptr;
  MOBACamera* moba_camera = nullptr;
};

#endif  // GDEXTENSION_MATCH_MANAGER_H
