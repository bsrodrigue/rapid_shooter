#include "player.h"
#include "collision.h"
#include "draw.h"
#include "entities.h"
#include "input_manager.h"
#include <raylib.h>
#include <raymath.h>

InputManager player_input_manager;

Player::Player(Vector2 position) {
  this->position = position;
  this->velocity = {0, 0}; // Initialize velocity
  this->angle = 0;
  this->speed = 200.0f;        // Max speed (units per second)
  this->acceleration = 800.0f; // Acceleration (units per second squared)
  this->deceleration =
      500.0f; // Deceleration/friction (units per second squared)
  this->health = 100;
  this->max_health = 100;
}

void Player::load_texture(Texture2D texture) { this->texture = texture; }

Vector2 Player::get_next_position() {
  // Get input direction
  Vector2 input_direction = {0, 0};
  if (player_input_manager.isActionDown(GameAction::MOVE_UP))
    input_direction.y -= 1.0f; // Up
  if (player_input_manager.isActionDown(GameAction::MOVE_DOWN))
    input_direction.y += 1.0f; // Down
  if (player_input_manager.isActionDown(GameAction::MOVE_LEFT))
    input_direction.x -= 1.0f; // Left
  if (player_input_manager.isActionDown(GameAction::MOVE_RIGHT))
    input_direction.x += 1.0f; // Right

  // Normalize direction to prevent faster diagonal movement
  if (Vector2Length(input_direction) > 0.0f) {
    input_direction = Vector2Normalize(input_direction);
  }

  // Apply acceleration based on input
  float delta_time = GetFrameTime();
  if (Vector2Length(input_direction) > 0.0f) {
    velocity = Vector2Add(
        velocity, Vector2Scale(input_direction, acceleration * delta_time));
  }

  // Apply deceleration (friction) when no input or to slow down
  if (Vector2Length(velocity) > 0.0f) {
    Vector2 friction =
        Vector2Scale(Vector2Normalize(velocity), -deceleration * delta_time);
    if (Vector2Length(velocity) < Vector2Length(friction)) {
      velocity = {0, 0}; // Stop if friction would overshoot
    } else {
      velocity = Vector2Add(velocity, friction);
    }
  }

  // Cap the velocity at max_speed
  if (Vector2Length(velocity) > speed) {
    velocity = Vector2Scale(Vector2Normalize(velocity), speed);
  }

  // Calculate next position based on velocity
  return Vector2Add(position, Vector2Scale(velocity, delta_time));
}

Vector2 Player::get_confirmed_position(Vector2 next_position,
                                       std::vector<Vector2> wall_positions) {
  Vector2 confirmed_position = this->position;

  // Check Y movement (vertical)
  if (check_cells_collision(wall_positions, {position.x, next_position.y}) ==
      -1) {
    confirmed_position.y = next_position.y;
  } else {
    velocity.y = 0; // Stop vertical velocity on collision
  }

  // Check X movement (horizontal)
  if (check_cells_collision(wall_positions, {next_position.x, position.y}) ==
      -1) {
    confirmed_position.x = next_position.x;
  } else {
    velocity.x = 0; // Stop horizontal velocity on collision
  }

  return confirmed_position;
}

void Player::handle_player_movement(std::vector<Vector2> wall_positions) {
  Vector2 next_position = get_next_position();
  this->position = get_confirmed_position(next_position, wall_positions);
}

void Player::draw() { draw_ship(position, angle); }
