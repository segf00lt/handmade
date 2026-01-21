#ifndef GFX_INC_C
#define GFX_INC_C


#include "gfx.c"


#if OS_WINDOWS

#include "gfx_windows.c"

#elif OS_LINUX

#error graphics not implemented for this platform

#elif OS_MAC

#error graphics not implemented for this platform

#elif OS_WEB

#error graphics not implemented for this platform

#endif


#endif
