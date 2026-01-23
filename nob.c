#define NOB_IMPLEMENTATION
#include "nob.h"

#ifdef _WIN32
#define CC "cl"
#else
#define CC "clang"
#endif

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd,
    CC,
    "/nologo",
    "/W4",
    "/wd4100",
    "/Zi",
    "/Od",
    "/DHANDMADE_INTERNAL",
    "/Fe:handmade.exe",
    "handmade.c",
    "user32.lib",
    "gdi32.lib",
    "Dsound.lib",
    "dxguid.lib",
    "winmm.lib",
    "ole32.lib",
    "/link",
    "/INCREMENTAL:NO",
    ""
  );

  if(!nob_cmd_run_sync(cmd)) return 1;

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
  #endif

  return 0;
}

