#ifndef GAME_H
#define GAME_H


// NOTE jfd 22/01/2026:
// This file contains game function headers.


shared_function Game_Vtable game_load_procs(void);
shared_function Game*       game_init(Platform *pp);
shared_function void        game_update_and_render(Game *gp);
shared_function void        game_get_sound_samples(Game *gp); // NOTE jfd: This has to run in under 1ms


internal b32 is_key_pressed(Game *gp, KeyboardKey key);
internal b32 was_key_pressed_once(Game *gp, KeyboardKey key);
internal b32 was_key_released(Game *gp, KeyboardKey key);

internal void render_weird_gradient(Game *gp, int x_offset, int y_offset);


#endif
