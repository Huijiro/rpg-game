target_sources(
  ${PROJECT_NAME} PRIVATE
  ./register_types.hpp
  ./register_types.cpp

  ./beeper.h
  ./beeper.cpp

  ./unit.hpp
  ./unit.cpp

  ./interactable.hpp
  ./interactable.cpp

  ./input_manager.hpp
  ./input_manager.cpp

  ./moba_camera.hpp
  ./moba_camera.cpp

  ./match_manager.hpp
  ./match_manager.cpp

  ./test_movement.hpp
  ./test_movement.cpp

  ./unit_order.hpp

  ./unit_component.hpp
  ./unit_component.cpp

  ./movement_component.hpp
  ./movement_component.cpp

  ./health_component.hpp
  ./health_component.cpp

  ./resource_pool_component.hpp
  ./resource_pool_component.cpp

   ./attack_component.hpp
   ./attack_component.cpp

   ./animation_controller.hpp
   ./animation_controller.cpp

   ./projectile.hpp
   ./projectile.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE "src")
