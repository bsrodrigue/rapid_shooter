#include "engine.h"
#include <cstdio>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: game [level_file]\n");
    return 1;
  }

  run_engine("game", argv[1]);
  return 0;
}
