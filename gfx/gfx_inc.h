#ifndef GFX_INC_H
#define GFX_INC_H


#include "gfx.h"


#if OS_WINDOWS

#include "gfx_windows.h"

#elif OS_LINUX

#error graphics not implemented for this platform

#elif OS_MAC

#error graphics not implemented for this platform

#elif OS_WEB

#error graphics not implemented for this platform

#endif


#endif
