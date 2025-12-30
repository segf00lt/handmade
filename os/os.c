#ifndef OS_C
#define OS_C

#if defined(OS_LINUX)

#include "os_linux.c"

#elif defined(OS_WEB)

#include "os_web.c"

#elif defined(OS_MAC)

#include "os_mac.c"

#elif defined(OS_WINDOWS)

#include "os_windows.c"

#endif

#endif
