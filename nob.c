#define NOB_IMPLEMENTATION
#include "nob.h"

#define CC "clang"

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  #if 0
  {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CC,
      "-std=c99",
      "-Wall",
      "-Wextra",
      "-Wno-unused-parameter",
      "-g",
      "-fno-omit-frame-pointer",
      "-no-pie",
      "-O0",
      "-pthread",
      "-o",
      "job_system_example",
      "job_system_example.c");

    if (!nob_cmd_run_sync(cmd)) return 1;
  }
  #endif

  {
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, CC,
      "-std=c99",
      "-Wall",
      "-Wextra",
      "-Wno-unused-parameter",
      "-g",
      "-fno-omit-frame-pointer",
      "-no-pie",
      "-O0",
      "-pthread",
      "-o",
      "semaphore_exercise",
      "semaphore_exercise.c");

    if (!nob_cmd_run_sync(cmd)) return 1;
  }

  return 0;
}

