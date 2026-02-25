#ifndef GAME_H
#define GAME_H


// NOTE jfd 22/01/2026:
// This file contains game function headers and macros.


#define KM(x) ((f32)(x) * 1.0e3f)
#define M(x) x
#define DM(x) ((f32)(x) * 1.0e-1f)
#define CM(x) ((f32)(x) * 1.0e-2f)
#define MM(x) ((f32)(x) * 1.0e-3f)

#define CHUNK_SHIFT                    7
#define CHUNK_SIZE                     (1 << CHUNK_SHIFT)
#define CHUNK_MASK                     (CHUNK_SIZE - 1)
#define TILE_SIZE_METERS               M(2.0f)
#define CHUNK_SIZE_METERS             (TILE_SIZE_METERS*(f32)CHUNK_SIZE)

#define PLAYER_MOVE_SPEED        (M(40)) /* meters per second */

#define PIXELS_PER_METER (3.0f)
#define METERS_PER_PIXEL (1.0f/PIXELS_PER_METER)
#define METERS_TO_PIXELS(x) ((f32)((f32)(x)*PIXELS_PER_METER))
#define PIXELS_TO_METERS(x) ((f32)((f32)(x)*METERS_PER_PIXEL))




internal void debug_render_weird_gradient(Game *gp, int x_offset, int y_offset);

force_inline Color alpha_blend(Color bottom, Color top);

force_inline Color color_from_pixel(u32 pixel);

force_inline u32 pixel_from_color(Color color);

internal void draw_rect_min_max(Game *gp, Color color, f32 min_x, f32 min_y, f32 max_x, f32 max_y);

internal void draw_rect_lines_min_max(Game *gp, Color color, f32 line_thickness, f32 min_x, f32 min_y, f32 max_x, f32 max_y);

internal void clear_screen(Game *gp);

internal void draw_rect(Game *gp, Color color, f32 x, f32 y, f32 width, f32 height);

internal void draw_rect_lines(Game *gp, Color color, f32 line_thickness, f32 x, f32 y, f32 width, f32 height);

internal b32 was_key_pressed_once(Game *gp, Keyboard_key key);

internal b32 is_key_pressed(Game *gp, Keyboard_key key);

internal b32 was_key_released(Game *gp, Keyboard_key key);

internal void debug_silence(Game *gp);

internal void debug_output_sound(Game *gp);

internal v2_s32 chunk_pos_from_point(Game *gp, v2 v);

internal void recanonicalize_coord(Game *gp, u32 *tile, f32 *tile_rel);

internal Tile_map_pos recanonicalize_pos(Game *gp, Tile_map_pos pos);

internal Chunk_pos chunk_pos_from_tile_map_pos(Game *gp, Tile_map_pos pos);

internal Chunk_pos chunk_pos_from_abs_tile_pos(Game *gp, u32 abs_tile_x, u32 abs_tile_y);

internal Tile_map_pos tile_map_pos_from_point(Game *gp, f32 x, f32 y);

force_inline Chunk* get_chunk(Game *gp, s32 chunk_x, s32 chunk_y);

force_inline u8 get_tile_of_chunk(Game *gp, Chunk *chunk, s32 tile_x, s32 tile_y);

force_inline u8 tile_from_tile_map_pos(Game *gp, Tile_map_pos pos);

internal void set_tile(Game *gp, u32 tile_x, u32 tile_y, u8 tile_value);

internal void init_player(Game *gp);

internal void draw_tile_by_point(Game *gp, Color color, v2 point);

internal void draw_tile_map(Game *gp);

shared_function void game_update_and_render(Game *gp);

shared_function void game_get_sound_samples(Game *gp);

shared_function Game* game_init(Platform *pp);

#endif
