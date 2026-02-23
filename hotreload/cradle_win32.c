#ifndef HOTRELOAD_CRADLE_WINDOWS_C
#define HOTRELOAD_CRADLE_WINDOWS_C

#include <windows.h>

typedef void* Module_init_func(HINSTANCE, HINSTANCE, LPSTR, int);
typedef int   Module_main_func(void *);


int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode) {

  void *module_state = 0;


  for(;;) {
    HMODULE module = 0;

    Module_init_func *module_init = 0;
    Module_main_func *module_main = 0;

    {

      DWORD attrs = GetFileAttributesA(MODULE".dll");
      int result = (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
      if(!result) {
        attrs = GetFileAttributesA(MODULE".dll.live");
        result = (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
        if(!result) {
          OutputDebugStringA(MODULE".dll.live not found\n");
          return 1;
        }
      } else {
        if(!MoveFileEx(MODULE".dll", MODULE".dll.live", MOVEFILE_REPLACE_EXISTING)) {
          OutputDebugStringA("module file rename failed\n");
          return 1;
        }
      }

    }

    module = LoadLibraryA(MODULE".dll.live");
    if(!module) {
      OutputDebugStringA("failed to load module\n");
      return 1;
    }

    module_init = (Module_init_func*)GetProcAddress(module, "module_init");
    if(!module_init) {
      OutputDebugStringA("failed to load module_init()");
      return 1;
    }

    module_main = (Module_main_func*)GetProcAddress(module, "module_main");
    if(!module_main) {
      OutputDebugStringA("failed to load module_main()");
      return 1;
    }

    if(!module_state) {
      module_state = module_init(instance, prevInstance, cmdLine, showCode);
    }

    int reload_module = module_main(module_state);

    FreeLibrary(module);

    if(!reload_module) {
      break;
    }

  }

  if(!MoveFileEx(MODULE".dll.live", MODULE".dll", MOVEFILE_REPLACE_EXISTING)) {
    OutputDebugStringA("module file rename failed\n");
    return 1;
  }

  return 0;
}


#endif
