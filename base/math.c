#ifndef BASE_MATH_C
#define BASE_MATH_C

#include <math.h>

#define PI ((f32)3.1415926f)

internal f32
func lerp_f32(f32 t, f32 begin, f32 end) {
  return t * (end - begin);
}

#endif
