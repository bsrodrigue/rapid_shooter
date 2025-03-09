#include "level.h"
#include "entities.h"
#include "save.h"

// TODO: Create Separate loaders for the game and the level editor
Level::Level() {}

void Level::load_level_data(std::string filename) {
  load_grid_data_from_level_file(filename.c_str(), this->grid);
}

void Level::save_level(std::string filename) {
  // TODO: Implement save_level function
}
