cmake_minimum_required(VERSION 3.10) # Set the minimum required version of CMake

# Project name and language
project(rapid_shooter LANGUAGES CXX)

# Define directories
set(SRC_DIR src)
set(INCLUDE_DIR include)
set(BUILD_DIR build)
set(EXTERNAL_DIR external)

# Find all source files in src/
file(GLOB_RECURSE ALL_SRC ${SRC_DIR}/*.cc)
file(GLOB_RECURSE HEADER_FILES ${INCLUDE_DIR}/*.h)

# Define common source files (everything except the entry points)
set(COMMON_SRC ${ALL_SRC})
list(REMOVE_ITEM COMMON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/game_main.cc")
list(REMOVE_ITEM COMMON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/editor_main.cc")

# Add ImGui and rlImGui source files
set(IMGUI_DIR ${EXTERNAL_DIR}/imgui)
set(RLIMGUI_DIR ${EXTERNAL_DIR}/rlImGui)

set(IMGUI_SRC ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp
              ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)

set(RLIMGUI_SRC ${RLIMGUI_DIR}/rlImGui.cpp)

# Find and link against required libraries
find_library(RAYLIB_LIBRARY raylib)
if(NOT RAYLIB_LIBRARY)
  message(FATAL_ERROR "raylib not found. Please ensure it is installed.")
endif()

# Common include directories
set(COMMON_INCLUDES ${INCLUDE_DIR} ${IMGUI_DIR} ${RLIMGUI_DIR})

# Function to setup target
function(setup_target TARGET_NAME ENTRY_FILE)
  add_executable(${TARGET_NAME} ${COMMON_SRC} ${ENTRY_FILE} ${IMGUI_SRC} ${RLIMGUI_SRC})
  
  target_include_directories(${TARGET_NAME} PRIVATE ${COMMON_INCLUDES})
  
  target_link_libraries(
    ${TARGET_NAME}
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
    ${TARGET_NAME}
    PROPERTIES CXX_STANDARD 17
               CXX_STANDARD_REQUIRED ON
               COMPILE_OPTIONS "-Wall;-Wextra;-g"
               RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
  )
endfunction()

# Specify the executable targets
setup_target(rapid_shooter ${SRC_DIR}/game_main.cc)
setup_target(rapid_editor ${SRC_DIR}/editor_main.cc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
