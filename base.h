#ifndef BASE_H
#define BASE_H

#include "base/platform_info.h"

#include "base/core.h"

#ifndef BASE_CORE_ONLY

#include "base/math.h"

#include "base/arena.h"
#include "base/str.h"
#include "base/array.h"

#include "os/os.h"

#endif

#endif

#ifndef BASE_C
#define BASE_C

#include "base/core.c"

#ifndef BASE_CORE_ONLY

#include "base/arena.c"
#include "base/str.c"
#include "base/array.c"

#include "os/os.c"

#endif

#endif
