#include "engine.h"
#include <cstdio>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: editor [level_file]\n");
    return 1;
  }

  run_engine("editor", argv[1]);
  return 0;
}
