#ifndef BASE_H
#define BASE_H

#include "base/platform_info.h"

#include "base/core.h"

#ifndef BASE_CORE_ONLY

#include "base/arena.h"
#include "base/str.h"
#include "base/array.h"

#include "base/math.h"

#include "os/os.h"

#ifndef OS_GFX_DISABLED
#include "os/os_gfx.h"
#endif

#endif

#endif

#ifndef BASE_C
#define BASE_C

#include "base/core.c"

#ifndef BASE_CORE_ONLY

#include "base/arena.c"
#include "base/str.c"
#include "base/array.c"

#include "base/math.c"

#include "os/os.c"

#ifndef OS_GFX_DISABLED
#include "os/os_gfx.c"
#endif

#endif

#endif
