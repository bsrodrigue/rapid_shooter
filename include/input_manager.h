#include <map>
#include <raylib.h>
#include <raymath.h>
#include "common.h"

#pragma once

enum class GameAction {
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,

  // Targeting
  AIM,

  SHOOT,
  INTERACT,
  DASH,
  CANCEL,
  SAVE_LEVEL,
};

enum class InputType {
  KEYBOARD,
  MOUSE,
  TOUCH,
  GAMEPAD_BUTTON,
  GAMEPAD_AXIS,
};

struct InputBinding {
  InputType type;
  int key;
  int gamepad_axis;
  float axis_threshold;
  bool is_positive;
};

class InputManager {
public:
  InputManager() {
    // Default keyboard bindings
    // bindings[GameAction::CANCEL] = {InputType::KEYBOARD, KEY_ESCAPE, -1,
    // 0.0f,
    //                                 false};
    // bindings[GameAction::MOVE_LEFT] = {InputType::KEYBOARD, KEY_A, -1, 0.0f,
    //                                    false};
    // bindings[GameAction::MOVE_RIGHT] = {InputType::KEYBOARD, KEY_D, -1, 0.0f,
    //                                     false};
    // bindings[GameAction::MOVE_UP] = {InputType::KEYBOARD, KEY_W, -1, 0.0f,
    //                                  false};
    // bindings[GameAction::MOVE_DOWN] = {InputType::KEYBOARD, KEY_S, -1, 0.0f,
    //                                    false};
    // bindings[GameAction::SHOOT] = {InputType::KEYBOARD, MOUSE_BUTTON_LEFT,
    // -1,
    //                                0.0f, false};
    // bindings[GameAction::INTERACT] = {InputType::KEYBOARD, KEY_SPACE, -1,
    // 0.0f,
    //                                   false};
    // bindings[GameAction::DASH] = {InputType::MOUSE, MOUSE_BUTTON_RIGHT, -1,
    //                               0.0f, false};

    // Default Xbox controller bindings (example)
    bindings[GameAction::MOVE_LEFT] = {InputType::GAMEPAD_AXIS, -1,
                                       GAMEPAD_AXIS_LEFT_X, -0.5f, false};
    bindings[GameAction::MOVE_RIGHT] = {InputType::GAMEPAD_AXIS, -1,
                                        GAMEPAD_AXIS_LEFT_X, 0.5f, true};
    bindings[GameAction::MOVE_UP] = {InputType::GAMEPAD_AXIS, -1,
                                     GAMEPAD_AXIS_LEFT_Y, -0.5f, false};
    bindings[GameAction::MOVE_DOWN] = {InputType::GAMEPAD_AXIS, -1,
                                       GAMEPAD_AXIS_LEFT_Y, 0.5f, true};

    bindings[GameAction::SHOOT] = {InputType::GAMEPAD_BUTTON,
                                   GAMEPAD_BUTTON_RIGHT_TRIGGER_2, -1, 0.0f,
                                   false};
    bindings[GameAction::INTERACT] = {InputType::GAMEPAD_BUTTON,
                                      GAMEPAD_BUTTON_RIGHT_FACE_DOWN, -1, 0.0f,
                                      false};
    bindings[GameAction::DASH] = {InputType::GAMEPAD_BUTTON,
                                  GAMEPAD_BUTTON_LEFT_TRIGGER_1, -1, 0.0f,
                                  false};
  }

  // Check if an action is pressed
  bool isActionPressed(GameAction action) {
    auto it = bindings.find(action);
    if (it == bindings.end())
      return false;

    InputBinding binding = it->second;
    switch (binding.type) {
    case InputType::KEYBOARD:
      return IsKeyPressed(binding.key);
    case InputType::MOUSE:
      return IsMouseButtonPressed(binding.key);
    case InputType::GAMEPAD_BUTTON:
      return IsGamepadButtonPressed(0,
                                    binding.key); // Assuming player 1 (index 0)
    case InputType::GAMEPAD_AXIS: {
      float axisValue = GetGamepadAxisMovement(0, binding.gamepad_axis);
      return (binding.is_positive && axisValue >= binding.axis_threshold) ||
             (!binding.is_positive && axisValue <= binding.axis_threshold);
    }
    default:
      return false;
    }
  }

  // Check if an action is held down
  bool isActionDown(GameAction action) {
    auto it = bindings.find(action);
    if (it == bindings.end())
      return false;

    InputBinding binding = it->second;
    switch (binding.type) {
    case InputType::KEYBOARD:
      return IsKeyDown(binding.key);
    case InputType::MOUSE:
      return IsMouseButtonDown(binding.key);
    case InputType::GAMEPAD_BUTTON:
      return IsGamepadButtonDown(0, binding.key);
    case InputType::GAMEPAD_AXIS: {
      float axisValue = GetGamepadAxisMovement(0, binding.gamepad_axis);
      return (binding.is_positive && axisValue >= binding.axis_threshold) ||
             (!binding.is_positive && axisValue <= binding.axis_threshold);
    }
    default:
      return false;
    }
  }

  // Allow rebinding (e.g., for a settings menu)
  void rebindAction(GameAction action, InputBinding newBinding) {
    bindings[action] = newBinding;
  }

  Vector2 getAimDirection() {
    Vector2 direction = {0, 0};

    if (IsGamepadAvailable(0)) {
      direction.x = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
      direction.y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

      // Apply dead zone
      float deadZone = 0.2f;
      float length = Vector2Length(direction);
      if (length < deadZone) {
        direction = {0, 0};
      } else {
        // Normalize if outside dead zone to maintain consistent speed
        direction = Vector2Normalize(direction);
        direction =
            Vector2Scale(direction, (length - deadZone) / (1.0f - deadZone));
      }
    }
    return direction;
  }

  // Fallback to mouse if no gamepad is connected
  Vector2 getTargetPosition(Camera2D camera, Vector2 player_position) {
    if (IsGamepadAvailable(0)) {
      Vector2 aimDir = getAimDirection();
      if (Vector2Length(aimDir) > 0) {
        // Scale the direction to a reasonable distance from the player
        Vector2 offset =
            Vector2Scale(aimDir, 50.0f); // Adjust distance as needed
        return Vector2Add(player_position, offset);
      }
    }
    // Default to mouse if no gamepad input or no gamepad connected
    return get_world_mouse(camera);
  }

private:
  std::map<GameAction, InputBinding> bindings;
};
