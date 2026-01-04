cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(GodotGame LANGUAGES CXX VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "CMake version: ${CMAKE_VERSION}")

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/" )

set(LIB_DIR "lib/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

message( STATUS "Building ${PROJECT_NAME} for ${CMAKE_SYSTEM_PROCESSOR} on ${CMAKE_SYSTEM_NAME}")

set( BUILD_OUTPUT_DIR "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/" )

set_target_properties( ${PROJECT_NAME}
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN true
        RUNTIME_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}/${LIB_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${BUILD_OUTPUT_DIR}/${LIB_DIR}"
)

if( NOT DEFINED CMAKE_DEBUG_POSTFIX )
    set_target_properties( ${PROJECT_NAME}
        PROPERTIES
            DEBUG_POSTFIX "-d"
    )
endif()

add_subdirectory(src)

set( INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/" )

message( STATUS "Install directory: ${INSTALL_DIR}")

install( TARGETS ${PROJECT_NAME}
    LIBRARY
        DESTINATION ${INSTALL_DIR}/${LIB_DIR}
    RUNTIME
        DESTINATION ${INSTALL_DIR}/${LIB_DIR}
)

# godot-cpp
# From here: https://github.com/godotengine/godot-cpp
if ( NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/libs/godot-cpp/Makefile" )
    message(
        FATAL_ERROR
            "[${PROJECT_NAME}] The godot-cpp submodule was not downloaded. Please update submodules: git submodule update --init --recursive."
    )
endif()

if ( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release" FORCE )
  message( STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified." )
endif()

add_subdirectory( templates )

add_subdirectory(libs/godot-cpp)

include(ccache)
include(format)

set_target_properties(godot-cpp
    PROPERTIES
        CXX_VISIBILITY_PRESET hidden    # visibility needs to be the same as the main library
)

target_link_libraries( ${PROJECT_NAME}
    PRIVATE
        godot-cpp
)
