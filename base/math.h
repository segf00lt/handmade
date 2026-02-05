#ifndef BASE_MATH_H
#define BASE_MATH_H

#define TYPEDEF_VEC2(T) typedef struct Vec2_##T { T x; T y; } Vec2_##T;
#define TYPEDEF_VEC3(T) typedef struct Vec3_##T { T x; T y; T z; } Vec3_##T;


TYPEDEF_VEC2(s8);
TYPEDEF_VEC2(s16);
TYPEDEF_VEC2(s32);
TYPEDEF_VEC2(s64);

TYPEDEF_VEC2(u8);
TYPEDEF_VEC2(u16);
TYPEDEF_VEC2(u32);
TYPEDEF_VEC2(u64);

TYPEDEF_VEC2(f32);
TYPEDEF_VEC2(f64);

typedef Vec2_f32 Vec2;

TYPEDEF_VEC3(s8);
TYPEDEF_VEC3(s16);
TYPEDEF_VEC3(s32);
TYPEDEF_VEC3(s64);

TYPEDEF_VEC3(u8);
TYPEDEF_VEC3(u16);
TYPEDEF_VEC3(u32);
TYPEDEF_VEC3(u64);

TYPEDEF_VEC3(f32);
TYPEDEF_VEC3(f64);

typedef Vec3_f32 Vec3;

internal f32 lerp_f32(f32 t, f32 begin, f32 end);
internal s32 round_f32_to_s32(f32 value);
internal f32 wrap_f32(f32 value, f32 min, f32 max);

#endif
