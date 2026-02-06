#ifndef BASE_MATH_C
#define BASE_MATH_C

#include <math.h>

#define PI ((f32)3.1415926f)

// TODO jfd: handle negative values in round_f32_to_s32()
force_inline s32
func round_f32_to_s32(f32 value) {
  s32 result = (s32)(value + 0.5f);
  return result;
}

force_inline f32
func max_f32(f32 a, f32 b) {
  // TODO jfd: reimplement
  return fmaxf(a, b);
}

force_inline f32
func abs_f32(f32 x) {
  u32 *bits = (u32*)&x;
  *bits &= ~(1 << 31);
  return x;
}

force_inline f32
func lerp_f32(f32 t, f32 begin, f32 end) {
  return t * (end - begin);
}

force_inline f32
func wrap_f32(f32 value, f32 min, f32 max)
{
    f32 range = max - min;
    f32 result = value - range*floorf((value - min)/range);
    return result;
}

force_inline Vec2
func wrap_vec2(Vec2 v, Vec2 min, Vec2 max) {
  Vec2 result = {
    wrap_f32(v.x, min.x, max.x),
    wrap_f32(v.y, min.y, max.y),
  };
  return result;
}

force_inline Vec2
func max_vec2(Vec2 a, Vec2 b) {
  Vec2 result = {
    .x = max_f32(a.x, b.x),
    .y = max_f32(a.y, b.y),
  };

  return result;
}

force_inline Vec2
func abs_vec2(Vec2 v) {
  Vec2 result = {
    .x = abs_f32(v.x),
    .y = abs_f32(v.y),
  };
  return result;
}

force_inline Vec2
func scale_vec2(Vec2 v, f32 a) {
  Vec2 result = {
    .x = v.x * a,
    .y = v.y * a,
  };

  return result;
}

force_inline Vec2
func truncate_vec2(Vec2 v) {
  // TODO jfd: this kind of truncation is undefined behaviour
  Vec2 result = {
    .x = (f32)(s32)v.x,
    .y = (f32)(s32)v.y,
  };
  return result;
}

force_inline Vec2
func add_value_vec2(Vec2 v, f32 a) {
  Vec2 result = {
    .x = v.x + a,
    .y = v.y + a,
  };
  return result;
}

force_inline Vec2
func add_vec2(Vec2 a, Vec2 b) {
  Vec2 result = {
    .x = a.x + b.x,
    .y = a.y + b.y,
  };
  return result;
}

force_inline Vec2
func sub_vec2(Vec2 a, Vec2 b) {
  Vec2 result = {
    .x = a.x - b.x,
    .y = a.y - b.y,
  };
  return result;
}


force_inline Vec2
func mul_vec2(Vec2 a, Vec2 b) {
  Vec2 result = {
    .x = a.x * b.x,
    .y = a.y * b.y,
  };
  return result;
}

force_inline Vec2
func div_vec2(Vec2 a, Vec2 b) {
  Vec2 result = {
    .x = a.x / b.x,
    .y = a.y / b.y,
  };
  return result;
}

#endif
