#ifndef GAME_TYPES_H
#define GAME_TYPES_H


// NOTE jfd 22/01/2026:
// This file contains typedefs for the game.

typedef struct Color Color;
struct Color {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
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
  Vec2 mouse_pos;
  Vec2 mouse_delta;
};

TYPEDEF_SLICE(Game_input, Game_input_slice);

typedef struct Game Game;
struct Game {
  f32 t;

  Arena *main_arena;
  Arena *frame_arena;
  Arena *temp_arena;

  Game_render_buffer render;
  Game_sound_buffer sound;
  Game_input input;

  s32 test_sound_pitch;

  int x_offset;
  int y_offset;

  b32 once;
  f32 t_sine;

  f32 rect_x;
  f32 rect_y;
  f32 rect_width;
  f32 rect_height;


};
STATIC_ASSERT(sizeof(Game) <= MB(1), game_state_is_less_than_a_megabyte);
#define GAME_STATE_SIZE ((u64)MB(1))


typedef void  Game_update_and_render_func(Game *gp);
typedef void  Game_get_sound_samples_func(Game *gp);
typedef Game* Game_init_func(Platform *pp);

typedef struct Game_vtable Game_vtable;
struct Game_vtable {
  Game_init_func            *init;
  Game_update_and_render_func *update_and_render;
  Game_get_sound_samples_func *get_sound_samples;
};

typedef Game_vtable Game_load_procs_func(void);

#endif
