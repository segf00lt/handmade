#ifndef GAME_H
#define GAME_H


// NOTE jfd 22/01/2026:
// This file contains game function headers and macros.


#define WORLD_ROWS               3
#define WORLD_COLUMNS            3

#define TILEMAP_ROWS             20
#define TILEMAP_COLUMNS          36
#define TILEMAP_TILE_SIZE        27.0f
#define TILEMAP_WIDTH            (TILEMAP_TILE_SIZE*(f32)TILEMAP_COLUMNS)
#define TILEMAP_HEIGHT           (TILEMAP_TILE_SIZE*(f32)TILEMAP_ROWS)

// #define PLAYER_MOVE_SPEED        (5.250f*TILEMAP_TILE_SIZE)
// #define PLAYER_MOVE_SPEED        (1.250f*TILEMAP_TILE_SIZE)
#define PLAYER_MOVE_SPEED        (20.250f*TILEMAP_TILE_SIZE)

shared_function Game_vtable game_load_procs(void);
shared_function Game*       game_init(Platform *pp);
shared_function void        game_update_and_render(Game *gp);
shared_function void        game_get_sound_samples(Game *gp); // NOTE jfd: This has to run in under 1ms


internal b32 is_key_pressed(Game *gp, Keyboard_key key);
internal b32 was_key_pressed_once(Game *gp, Keyboard_key key);
internal b32 was_key_released(Game *gp, Keyboard_key key);

internal void set_tilemap(Game *gp, u8 *tiles, Vec2 origin);

#endif
