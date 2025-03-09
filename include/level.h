#pragma once

#include "config.h"
#include "entities.h"
#include "save.h"

class Level {
public:
  EditorGridCell grid[CELL_COUNT][CELL_COUNT];

  Level();

  void load_level_data(std::string filename);
  void save_level(std::string filename);
};
