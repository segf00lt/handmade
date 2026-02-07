#ifndef GAME_H
#define GAME_H


// NOTE jfd 22/01/2026:
// This file contains game function headers and macros.


#define KM(x) ((f32)(x) * 1.0e3f)
#define M(x) x
#define DM(x) ((f32)(x) * 1.0e-1f)
#define CM(x) ((f32)(x) * 1.0e-2f)
#define MM(x) ((f32)(x) * 1.0e-3f)

#define WORLD_CHUNKS_X_COUNT           3
#define WORLD_CHUNKS_Y_COUNT           3

#define CHUNK_TILES_X_COUNT            36
#define CHUNK_TILES_Y_COUNT            20
#define TILE_SIZE_METERS               M(2.0f)
#define CHUNK_WIDTH_METERS             (TILE_SIZE_METERS*(f32)CHUNK_TILES_X_COUNT)
#define CHUNK_HEIGHT_METERS            (TILE_SIZE_METERS*(f32)CHUNK_TILES_Y_COUNT)

#define PLAYER_MOVE_SPEED        (M(20)) /* meters per second */

#define PIXELS_PER_METER (13.5f)
#define METERS_PER_PIXEL (1.0f/PIXELS_PER_METER)
#define METERS_TO_PIXELS(x) ((f32)((f32)(x)*PIXELS_PER_METER))
#define PIXELS_TO_METERS(x) ((f32)((f32)(x)*METERS_PER_PIXEL))

shared_function Game_vtable game_load_procs(void);
shared_function Game*       game_init(Platform *pp);
shared_function void        game_update_and_render(Game *gp);
shared_function void        game_get_sound_samples(Game *gp); // NOTE jfd: This has to run in under 1ms


internal b32 is_key_pressed(Game *gp, Keyboard_key key);
internal b32 was_key_pressed_once(Game *gp, Keyboard_key key);
internal b32 was_key_released(Game *gp, Keyboard_key key);

#endif
