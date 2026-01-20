#ifndef OS_C
#define OS_C

#if OS_LINUX

#include "os_linux.c"

#elif OS_WEB

#include "os_web.c"

#elif OS_MAC

#include "os_mac.c"

#elif OS_WINDOWS

#include "os_windows.c"

#endif

#endif
