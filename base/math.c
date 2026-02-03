#ifndef BASE_MATH_C
#define BASE_MATH_C

#include <math.h>

#define PI ((f32)3.1415926f)

internal s32
func round_f32_to_s32(f32 value) {
  s32 result = (s32)(value + 0.5f);
  return result;
}


internal f32
func lerp_f32(f32 t, f32 begin, f32 end) {
  return t * (end - begin);
}

#endif
