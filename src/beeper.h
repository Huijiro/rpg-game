#ifndef GDEXTENSION_BEEPER_H
#define GDEXTENSION_BEEPER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/wrapped.hpp>

using godot::Node;

class Beeper : public Node {
  GDCLASS(Beeper, Node)

 protected:
  static void _bind_methods();

 public:
  Beeper();
  ~Beeper();
  void _enter_tree() override;
};

#endif  // GDEXTENSION_BEEPER_H
