#define NOB_IMPLEMENTATION
#include "nob.h"


int win32_build_hot_reload_no_cradle(void) {
  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd,
    "cl",
    "/nologo",
    "/W4",
    "/wd4100",
    "/Zi",
    "/Od",
    "/DHANDMADE_INTERNAL",
    "/DHANDMADE_HOTRELOAD",
    "/LD",
    "handmade_hotreload_build.c",
    "user32.lib",
    "gdi32.lib",
    "Dsound.lib",
    "dxguid.lib",
    "winmm.lib",
    "ole32.lib",
    "/link",
    "/INCREMENTAL:NO",
    "/PDB:handmade_module.pdb",
    "/OUT:game.dll.tmp",
    ""
  );
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_rename("game.dll.tmp", "game.dll");

  return 1;
}

int win32_build_hot_reload(void) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd,
    "cl",
    "/nologo",
    "/W4",
    "/wd4100",
    "/Zi",
    "/Od",
    "/DHANDMADE_HOTRELOAD",
    "/DMODULE=\\\"game\\\"",
    "/Fe:handmade.exe",
    "hotreload/cradle_win32.c",
    "user32.lib",
    "/link",
    "/INCREMENTAL:NO",
    ""
  );
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return win32_build_hot_reload_no_cradle();
}

int win32_build(void) {
  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd,
    "cl",
    "/nologo",
    "/W4",
    "/wd4100",
    "/Zi",
    "/Od",
    "/DHANDMADE_INTERNAL",
    "/Fe:handmade.exe",
    "handmade_build.c",
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
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;
  return 1;
}

int macos_build_virutal_memory_test(void) {
  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd,
    "clang",
    "-std=c99",
    "-Wall",
    "-Wextra",
    "-Wno-unused-parameter",
    "-g",
    "-fno-omit-frame-pointer",
    // "-no-pie",
    "-O0",
    // "-pthread",
    "-o",
    "macos_virtual_memory_test",
    "macos_virtual_memory_test.c");

  if (!nob_cmd_run_sync(cmd)) return 0;

  return 1;
}

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);


  #if 1
  if(!win32_build_hot_reload()) return 1;
  #else
  if(!win32_build_hot_reload_no_cradle()) return 1;
  #endif

  return 0;

  if(!win32_build()) return 1;

  if(!macos_build_virutal_memory_test()) return 1;





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

