
#include "base.h"

#include "platform/platform_types.h"
#include "platform/platform.h"



#include "game_types.h"



#if OS_WINDOWS

#include "platform/platform_win32.h"
#include "platform/platform_win32.c"

#elif OS_LINUX
#error platform not supported yet

#elif OS_MAC
#error platform not supported yet

#elif OS_WEB
#error platform not supported yet

#else
#error platform not supported
#endif
