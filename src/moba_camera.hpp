#ifndef GDEXTENSION_MOBA_CAMERA_H
#define GDEXTENSION_MOBA_CAMERA_H

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

using godot::Camera3D;
using godot::Node3D;
using godot::Vector3;

class MOBACamera : public Node3D {
  GDCLASS(MOBACamera, Node3D)

 protected:
  static void _bind_methods();

 public:
  MOBACamera();
  ~MOBACamera();

  void _ready() override;
  void _physics_process(double delta) override;

  void set_target(Node3D* target);
  Node3D* get_target() const;

  void set_distance(float distance);
  float get_distance() const;

  void set_height(float height);
  float get_height() const;

  void set_follow_speed(float speed);
  float get_follow_speed() const;

  void set_pitch_angle(float angle);
  float get_pitch_angle() const;

 private:
  void _update_camera_transform(double delta, bool snap);

  Node3D* target = nullptr;
  Camera3D* camera = nullptr;
  float distance = 10.0f;     // Distance from target in horizontal plane
  float height = 8.0f;        // Height above target
  float follow_speed = 5.0f;  // Speed of camera following target
  float pitch_angle = 45.0f;  // Camera pitch angle in degrees
};

#endif  // GDEXTENSION_MOBA_CAMERA_H
