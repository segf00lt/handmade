#ifndef HOTRELOAD_MODULE_WIN32_C
#define HOTRELOAD_MODULE_WIN32_C


shared_function void*
func module_init(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode) {

  Platform_win32 *pp_win32 = platform_win32_init(instance, prevInstance, cmdLine, showCode);
  ASSERT(pp_win32);

  return (void*)pp_win32;
}


shared_function int
func module_main(void *ptr) {

  Platform_win32 *pp_win32 = (Platform_win32*)ptr;
  pp = pp_win32;

  platform_win32_main();

  int do_reload = pp_win32->do_reload;
  pp_win32->do_reload = 0;

  return do_reload;

}


#endif
