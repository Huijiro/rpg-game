target_sources(
  ${PROJECT_NAME} PRIVATE
  ./register_types.hpp
  ./register_types.cpp

  ./beeper.h
  ./beeper.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE "src")
