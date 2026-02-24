#ifndef PLATFORM_INCLUDE_C
#define PLATFORM_INCLUDE_C


#include "platform.c"


#if OS_WINDOWS

#include "platform_core_win32.c"
#include "platform_win32.c"

#elif OS_LINUX

#include "platform_core_win32.c"
#include "platform_linux.c"

#elif OS_MAC

#include "platform_core_macos.c"
#include "platform_macos.c"

#elif OS_WEB

#error unsupported platform

#else
#error unsupported platform
#endif



#endif
