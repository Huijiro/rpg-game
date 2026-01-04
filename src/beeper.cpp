#include "beeper.h"
#include <godot_cpp/variant/utility_functions.hpp>

Beeper::Beeper() {}

Beeper::~Beeper() {}

void Beeper::_bind_methods() {}

void Beeper::Beeper::_enter_tree() {
  godot::UtilityFunctions::print("Beep!");
}
