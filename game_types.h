#ifndef GAME_TYPES_H
#define GAME_TYPES_H


// NOTE jfd 22/01/2026:
// This file contains typedefs for the game.


typedef struct Game_SoundBuffer Game_SoundBuffer;
struct Game_SoundBuffer {
  int samples_per_second;
  int sample_count;
  s16 *samples;
};

typedef struct Game_RenderBuffer Game_RenderBuffer;
struct Game_RenderBuffer {
  u8 *pixels;
  s32 width;
  s32 height;
  u32 stride; // NOTE jfd: This comes from the backbuffer thing
};

typedef struct Game_Input Game_Input;
struct Game_Input {
  KeyboardModifier modifier_mask;
  u32 key_pressed[KBD_KEY_MAX];
  b32 key_released[KBD_KEY_MAX];
  Vec2 mouse_pos;
  Vec2 mouse_delta;
};

typedef struct Game Game;
struct Game {
  f32 t;

  Arena *main_arena;
  Arena *frame_arena;
  Arena *temp_arena;

  Game_RenderBuffer render;
  Game_SoundBuffer sound;
  Game_Input input;

  s32 test_sound_pitch;

  int x_offset;
  int y_offset;
};
STATIC_ASSERT(sizeof(Game) <= MB(1), game_state_is_less_than_a_megabyte);
#define GAME_STATE_SIZE ((u64)MB(1))



typedef void  Game_UpdateAndRenderFunc(Game *gp);
typedef void  Game_GetSoundSamplesFunc(Game *gp);
typedef Game* Game_InitFunc(Platform *pp);

typedef struct Game_Vtable Game_Vtable;
struct Game_Vtable {
  Game_InitFunc            *init;
  Game_UpdateAndRenderFunc *update_and_render;
  Game_GetSoundSamplesFunc *get_sound_samples;
};

typedef Game_Vtable Game_LoadProcsFunc(void);

#endif
