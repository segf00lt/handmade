#ifndef GAME_TYPES_H
#define GAME_TYPES_H


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

typedef struct Tile_map_pos Tile_map_pos;
struct Tile_map_pos {
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

  Tile_map_pos player_pos;
  f32       player_width;
  f32       player_height;
  v2        player_vel;

  b8 once;
  b8 should_init_player;

  u32 world_chunks_x_count;
  u32 world_chunks_y_count;
  Chunk *world_chunks;
  Chunk chunk;

  v2 camera_offset;
  v2 camera_pos;

};
STATIC_ASSERT(sizeof(Game) <= MB(1), game_state_is_less_than_a_megabyte);
#define GAME_STATE_SIZE ((u64)MB(1))




#endif
