#ifndef GAME_H
#define GAME_H


// NOTE jfd 22/01/2026:
// This file contains game function headers and macros.


#define KM(x) ((f32)(x) * 1.0e3f)
#define M(x) x
#define DM(x) ((f32)(x) * 1.0e-1f)
#define CM(x) ((f32)(x) * 1.0e-2f)
#define MM(x) ((f32)(x) * 1.0e-3f)

#define WORLD_CHUNKS_X_COUNT           1
#define WORLD_CHUNKS_Y_COUNT           1

#define CHUNK_SIZE                     256
#define CHUNK_MASK                     0xff
#define TILE_SIZE_METERS               M(2.0f)
#define CHUNK_SIZE_METERS             (TILE_SIZE_METERS*(f32)CHUNK_SIZE)

#define PLAYER_MOVE_SPEED        (M(40)) /* meters per second */

#define PIXELS_PER_METER (13.5f)
#define METERS_PER_PIXEL (1.0f/PIXELS_PER_METER)
#define METERS_TO_PIXELS(x) ((f32)((f32)(x)*PIXELS_PER_METER))
#define PIXELS_TO_METERS(x) ((f32)((f32)(x)*METERS_PER_PIXEL))


typedef struct Color Color;
struct Color {
  f32 r;
  f32 g;
  f32 b;
  f32 a;
};

typedef struct Game_sound_buffer Game_sound_buffer;
struct Game_sound_buffer {
  int samples_per_second;
  int sample_count;
  s16 *samples;
};

typedef struct Game_render_buffer Game_render_buffer;
struct Game_render_buffer {
  u8 *pixels;
  s32 width;
  s32 height;
  u32 stride; // NOTE jfd: This comes from the backbuffer thing
};

typedef struct Game_input Game_input;
struct Game_input {
  Keyboard_modifier modifier_mask;
  u32 key_pressed[KBD_KEY_MAX];
  b32 key_released[KBD_KEY_MAX];
  v2 mouse_pos;
  v2 mouse_delta;
};

TYPEDEF_SLICE(Game_input);

typedef struct Chunk Chunk;
struct Chunk {
  u8 *tiles;
};

typedef struct Chunk_pos Chunk_pos;
struct Chunk_pos {
  v2_s32 chunk;
  v2_s32 tile;
};

typedef struct World_pos World_pos;
struct World_pos {
  v2_u32 tile;
  v2 tile_rel;
};

typedef struct Game Game;
struct Game {
  f32 t;
  b32 did_reload;

  Arena *main_arena;
  Arena *frame_arena;
  Arena *temp_arena;

  Game_render_buffer render;
  Game_sound_buffer sound;
  Game_input input;

  s32 test_sound_pitch;

  f32 t_sine;

  World_pos player_pos;
  f32       player_width;
  f32       player_height;
  v2        player_vel;

  b8 once;
  b8 should_init_player;

  Chunk chunk;

  v2 camera_offset;

};
STATIC_ASSERT(sizeof(Game) <= MB(1), game_state_is_less_than_a_megabyte);
#define GAME_STATE_SIZE ((u64)MB(1))



internal Game*       game_init(Platform *pp);
internal void        game_update_and_render(Game *gp);
internal void        game_get_sound_samples(Game *gp); // NOTE jfd: This has to run in under 1ms


internal b32 is_key_pressed(Game *gp, Keyboard_key key);
internal b32 was_key_pressed_once(Game *gp, Keyboard_key key);
internal b32 was_key_released(Game *gp, Keyboard_key key);

#endif
