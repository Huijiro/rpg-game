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
)

target_include_directories(${PROJECT_NAME} PRIVATE "src")
