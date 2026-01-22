#ifndef BASE_MATH_H
#define BASE_MATH_H

#define DECL_VEC2(T) typedef struct Vec2_##T { T x; T y; } Vec2_##T;
#define DECL_VEC3(T) typedef struct Vec3_##T { T x; T y; T z; } Vec3_##T;

DECL_VEC2(s8);
DECL_VEC2(s16);
DECL_VEC2(s32);
DECL_VEC2(s64);

DECL_VEC2(u8);
DECL_VEC2(u16);
DECL_VEC2(u32);
DECL_VEC2(u64);

DECL_VEC2(f32);
DECL_VEC2(f64);

typedef Vec2_f32 Vec2;

DECL_VEC3(s8);
DECL_VEC3(s16);
DECL_VEC3(s32);
DECL_VEC3(s64);

DECL_VEC3(u8);
DECL_VEC3(u16);
DECL_VEC3(u32);
DECL_VEC3(u64);

DECL_VEC3(f32);
DECL_VEC3(f64);

typedef Vec3_f32 Vec3;

internal f32 lerp_f32(f32 t, f32 begin, f32 end);

#endif
