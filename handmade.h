#ifndef HANDMADE_H
#define HANDMADE_H


typedef struct Game Game;
struct Game {
  f32 t;

  Arena *main_arena;
  Arena *frame_arena;
  Arena *temp_arena;

  u8 *render_buffer;
  s32 render_width;
  s32 render_height;
  u32 render_stride; // NOTE jfd: This comes from the backbuffer thing

  s32 test_sound_pitch;

  int x_offset;
  int y_offset;

  OS_Input input;
};
STATIC_ASSERT(sizeof(Game) <= MB(1), game_state_is_less_than_a_megabyte);
#define GAME_STATE_SIZE ((u64)MB(1))

void render_weird_gradient(Game *gp, int x_offset, int y_offset);

void game_update_and_render(Game *gp);

b32  is_key_pressed(Game *gp, OS_Key key);
b32  was_key_pressed_once(Game *gp, OS_Key key);
b32  was_key_released(Game *gp, OS_Key key);

#endif
