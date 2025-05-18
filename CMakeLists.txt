cmake_minimum_required(VERSION 3.10) # Set the minimum required version of CMake

# Project name and language
project(rapid_shooter VERSION 0.1.0 LANGUAGES C CXX)

# Define directories
set(SRC_DIR src)
set(INCLUDE_DIR include)
set(BUILD_DIR build)
set(EXTERNAL_DIR external)

# Find all source files in src/
file(GLOB_RECURSE SRC_FILES ${SRC_DIR}/*.cc)
file(GLOB_RECURSE HEADER_FILES ${INCLUDE_DIR}/*.h)

# Add ImGui and rlImGui source files
set(IMGUI_DIR ${EXTERNAL_DIR}/imgui)
set(RLIMGUI_DIR ${EXTERNAL_DIR}/rlImGui)

set(IMGUI_SRC ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp
              ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)

set(RLIMGUI_SRC ${RLIMGUI_DIR}/rlImGui.cpp)

# Specify the executable target with all source files
add_executable(${PROJECT_NAME} ${SRC_FILES} ${IMGUI_SRC} ${RLIMGUI_SRC})

# Set the include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${IMGUI_DIR}
                                                   ${RLIMGUI_DIR})

# Find and link against required libraries
find_library(RAYLIB_LIBRARY raylib)
if(NOT RAYLIB_LIBRARY)
  message(FATAL_ERROR "raylib not found. Please ensure it is installed.")
endif()

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE ${RAYLIB_LIBRARY}
          m # Math library
          stdc++ # C++ standard library
          GL # OpenGL (required for ImGui and rlImGui)
          pthread # POSIX threads (required on Unix-like systems)
          dl # Dynamic linking (required on Unix-like systems)
          X11 # X11 (required on Linux for Raylib)
)

# Set compiler flags
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED ON
             COMPILE_OPTIONS "-Wall;-Wextra;-g" # Use -g instead of --debug for
                                                # debug symbols
)

# Specify the output directory for binaries
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                                 ${CMAKE_BINARY_DIR}/bin)

# Custom clean command
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
                                    "${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
