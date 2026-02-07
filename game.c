#ifndef GAME_C
#define GAME_C


/////////////////////////////////
// globals

#ifdef HANDMADE_HOTRELOAD

Platform_get_keyboard_modifiers_func *platform_get_keyboard_modifiers;
Platform_read_entire_file_func  *platform_read_entire_file;
Platform_write_entire_file_func *platform_write_entire_file;

#endif

#include "world_data.c"


/////////////////////////////////
// functions


shared_function Game_vtable
func game_load_procs(void) {
  Game_vtable vtable = {
    .init              = &game_init,
    .update_and_render = &game_update_and_render,
    .get_sound_samples = &game_get_sound_samples,
  };

  return vtable;
}

shared_function Game*
func game_init(Platform *pp) {
  ASSERT(GAME_STATE_SIZE < pp->game_memory_backbuffer_size);

  u64 alloc_size = pp->game_memory_backbuffer_size;
  u8 *alloc_ptr = (u8*)(pp->game_memory_backbuffer);

  Game *gp = (Game*)alloc_ptr;
  alloc_ptr += GAME_STATE_SIZE;
  alloc_size -= GAME_STATE_SIZE;

  u64 game_frame_arena_backbuffer_size = MB(5);
  u64 game_temp_arena_backbuffer_size  = MB(5);
  u64 game_main_arena_backbuffer_size = alloc_size - game_frame_arena_backbuffer_size - game_temp_arena_backbuffer_size;

  u8 *game_main_arena_backbuffer = alloc_ptr;
  alloc_ptr += game_main_arena_backbuffer_size;
  alloc_size -= game_main_arena_backbuffer_size;

  u8 *game_frame_arena_backbuffer = alloc_ptr;
  alloc_ptr += game_frame_arena_backbuffer_size;
  alloc_size -= game_frame_arena_backbuffer_size;

  u8 *game_temp_arena_backbuffer = alloc_ptr;
  alloc_ptr += game_temp_arena_backbuffer_size;
  alloc_size -= game_temp_arena_backbuffer_size;

  gp->main_arena  = arena_create_ex(game_main_arena_backbuffer_size, true, game_main_arena_backbuffer);
  gp->frame_arena = arena_create_ex(game_frame_arena_backbuffer_size, true, game_frame_arena_backbuffer);
  gp->temp_arena  = arena_create_ex(game_temp_arena_backbuffer_size,  true, game_temp_arena_backbuffer);

  gp->did_reload = true;
  gp->once = true;

  #ifdef HANDMADE_HOTRELOAD

  platform_get_keyboard_modifiers  = pp->vtable.get_keyboard_modifiers;
  platform_read_entire_file  = pp->vtable.read_entire_file;
  platform_write_entire_file = pp->vtable.write_entire_file;

  #endif

  return gp;
}


internal void
func debug_render_weird_gradient(Game *gp, int x_offset, int y_offset) {
  u8 *row = gp->render.pixels;
  for(int y = 0; y < gp->render.height; y++) {
    u32 *pixel = (u32*)row;

    for(int x = 0; x < gp->render.width; x++) {
      //
      // pixel in memory
      //  0  1  2  3
      //  B  G  R  x
      // 00 00 00 00

      u8 r = (u8)(x + x_offset);
      u8 g = 0;
      u8 b = (u8)(y + y_offset);

      // *pixel++ = (r << 16) | (g << 8) | b;
      // *pixel++ = (r << 16) | (b << 8) | g;
      *pixel++ = (r << 8) | (g << 16) | b;
    }

    row += gp->render.stride;
  }

}

force_inline Color
func alpha_blend(Color bottom, Color top) {

  Color final;
  final.a = 1.0f / (top.a + bottom.a*(1.0f - top.a));
  final.r = (top.r*top.a + bottom.r*bottom.a*(1.0f - top.a)) * final.a;
  final.g = (top.g*top.a + bottom.g*bottom.a*(1.0f - top.a)) * final.a;
  final.b = (top.b*top.a + bottom.b*bottom.a*(1.0f - top.a)) * final.a;

  return final;
}

force_inline Color
func color_from_pixel(u32 pixel) {
  // pixel in memory
  //  0  1  2  3
  //  B  G  R  x
  // 00 00 00 00

  f32 color_scale = 1.0f/(f32)MAX_U8;

  Color result = {
    (f32)((pixel & 0xff0000) >> 16) * color_scale,
    (f32)((pixel & 0x00ff00) >> 8) * color_scale,
    (f32)((pixel & 0x0000ff)) * color_scale,
    (f32)((pixel & 0xff000000) >> 24) * color_scale,
  };

  return result;
}

force_inline u32
func pixel_from_color(Color color) {
  // pixel in memory
  //  0  1  2  3
  //  B  G  R  x
  // 00 00 00 00

  u8 a = (u8)(color.a * (f32)MAX_U8);
  u8 r = (u8)(color.r * (f32)MAX_U8);
  u8 g = (u8)(color.g * (f32)MAX_U8);
  u8 b = (u8)(color.b * (f32)MAX_U8);

  u32 result = (a << 24) | (r << 16) | (g << 8) | b;
  return result;
}

internal void
func draw_rect_min_max(Game *gp, Color color, f32 min_x, f32 min_y, f32 max_x, f32 max_y) {

  f32 x0 = fminf(min_x, max_x);
  f32 x1 = fmaxf(min_x, max_x);
  f32 y0 = fminf(min_y, max_y);
  f32 y1 = fmaxf(min_y, max_y);

  int begin_x = round_f32_to_s32(fmaxf(0, x0));
  int begin_y = round_f32_to_s32(fmaxf(0, y0));
  int end_x   = round_f32_to_s32(fminf(x1, (f32)gp->render.width));
  int end_y   = round_f32_to_s32(fminf(y1, (f32)gp->render.height));

  u8 *row = gp->render.pixels;

  for(int y = begin_y; y < end_y; y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < end_x; x++) {
      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }

}

internal void
func draw_rect_lines_min_max(Game *gp, Color color, f32 line_thickness, f32 min_x, f32 min_y, f32 max_x, f32 max_y) {

  f32 x0 = fminf(min_x, max_x);
  f32 x1 = fmaxf(min_x, max_x);
  f32 y0 = fminf(min_y, max_y);
  f32 y1 = fmaxf(min_y, max_y);

  int begin_x = round_f32_to_s32(fmaxf(0, x0));
  int begin_y = round_f32_to_s32(fmaxf(0, y0));
  int end_x   = round_f32_to_s32(fminf(x1, (f32)gp->render.width));
  int end_y   = round_f32_to_s32(fminf(y1, (f32)gp->render.height));

  line_thickness = fminf(line_thickness, fminf(x1-x0, y1-y0));

  u8 *row = gp->render.pixels;

  for(int y = begin_y; y < begin_y + round_f32_to_s32(line_thickness) && y < gp->render.height; y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < end_x && x < gp->render.width; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }


  for(int y = begin_y + round_f32_to_s32(line_thickness); y < end_y - round_f32_to_s32(line_thickness) && y < gp->render.height; y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < begin_x + round_f32_to_s32(line_thickness) && x < gp->render.width; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

    for(int x = end_x - round_f32_to_s32(line_thickness); x < end_x && x < gp->render.width; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }

  for(int y = end_y - round_f32_to_s32(line_thickness); y < end_y && y < gp->render.height; y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < end_x; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }


}

internal void
func clear_screen(Game *gp) {
  u64 *pixels = (u64*)gp->render.pixels;
  s32 total_pixels = (gp->render.width * gp->render.height) >> 1;

  for(int i = 0; i < total_pixels; i++) {
    pixels[i] = ((u64)255 << (32 + 24)) | ((u64)255 << 24);
  }

}

internal void
func draw_rect(Game *gp, Color color, f32 x, f32 y, f32 width, f32 height) {

  f32 min_x = x;
  f32 min_y = y;

  f32 max_x = min_x + width;
  f32 max_y = min_y + height;

  draw_rect_min_max(gp, color, min_x, min_y, max_x, max_y);

}

internal void
func draw_rect_lines(Game *gp, Color color, f32 line_thickness, f32 x, f32 y, f32 width, f32 height) {

  f32 min_x = x;
  f32 min_y = y;

  f32 max_x = min_x + width;
  f32 max_y = min_y + height;

  draw_rect_lines_min_max(gp, color, line_thickness, min_x, min_y, max_x, max_y);

}

internal b32
func was_key_pressed_once(Game *gp, Keyboard_key key) {
  return !!(gp->input.key_pressed[key] == 1);
}

internal b32
func is_key_pressed(Game *gp, Keyboard_key key) {
  return !!(gp->input.key_pressed[key] > 0);
}

internal b32
func was_key_released(Game *gp, Keyboard_key key) {
  return (gp->input.key_released[key] == true);
}


internal void
func debug_silence(Game *gp) {
  memory_zero(gp->sound.samples, sizeof(gp->sound.samples[0]) * gp->sound.sample_count);
}

internal void
func debug_output_sound(Game *gp) {
  Game_sound_buffer *sound_buffer = &gp->sound;

  s16 tone_volume = 600;
  int tone_hz = 256;
  int wave_period = sound_buffer->samples_per_second / tone_hz;

  s16 *sample_out = sound_buffer->samples;
  for(s32 sample_index = 0; sample_index < sound_buffer->sample_count; sample_index++) {

    #if 0
    OutputDebugStringA(cstrf(gp->temp_arena, "sine_value = %f\n", sine_value));
    arena_clear(gp->temp_arena);
    #endif

    f32 sine_value = sinf(gp->t_sine);

    s16 sample_value = (s16)(sine_value * tone_volume);

    *sample_out++ = sample_value;
    *sample_out++ = sample_value;

    gp->t_sine += (2.0f*PI*1.0f/((f32)wave_period));
    if(gp->t_sine > 2.0f*PI) {
      gp->t_sine -= 2.0f*PI;
    }

  }

}

force_inline u8
func get_tile(Game *gp, s32 col, s32 row) {
  s32 tile_id = col + CHUNK_TILES_X_COUNT * row;
  tile_id = CLAMP(tile_id, 0, CHUNK_TILES_X_COUNT*CHUNK_TILES_Y_COUNT - 1);
  u8 tile = gp->chunk.tiles[tile_id];

  return tile;
}

internal v2_s32
func chunk_pos_from_point(Game *gp, v2 v) {
  v2_s32 chunk = cast_v2_f32_to_s32(mul_v2(v, (v2){ 1.0f/CHUNK_WIDTH_METERS, 1.0f/CHUNK_HEIGHT_METERS }));

  return chunk;
}

internal World_pos
func world_pos_from_point(Game *gp, f32 x, f32 y) {
  World_pos result = {0};
  v2 v = { x, y };

  v2_s32 tile = cast_v2_f32_to_s32(scale_v2(v, 1.0f/TILE_SIZE_METERS));
  v2_s32 chunk = cast_v2_f32_to_s32(mul_v2(v, (v2){ 1.0f/CHUNK_WIDTH_METERS, 1.0f/CHUNK_HEIGHT_METERS }));

  v2 tile_rel =
  sub_v2(v,
    add_v2(
      scale_v2(cast_v2_s32_to_f32(tile), TILE_SIZE_METERS),
      mul_v2(cast_v2_s32_to_f32(chunk), (v2){ CHUNK_WIDTH_METERS, CHUNK_HEIGHT_METERS })
    )
  );

  result.tile_rel = tile_rel;
  result.tile = tile;
  result.chunk = chunk;

  return result;
}

shared_function void
func game_update_and_render(Game *gp) {

  if(gp->did_reload) {
    gp->did_reload = false;

    gp->should_init_player = true;

  }

  if(gp->once) {
    gp->once = false;
    gp->should_init_player = true;

    gp->chunk.tiles = (u8*)(world_tiles[0][0]);
  }


  { /* run_once */


    if(gp->should_init_player) {
      gp->should_init_player = false;


      gp->player_pos = world_pos_from_point(gp, 4, 2);
      gp->player_width = CM(150);
      gp->player_height = CM(150);
    }

  } /* run_once */


  { /* get keyboard and mouse input */

    // TODO jfd: mouse movement and clicks
    f32 player_speed = PLAYER_MOVE_SPEED;

    f32 scaled_player_speed = player_speed * gp->t;

    gp->player_vel = (v2){0};

    if(is_key_pressed(gp, KBD_KEY_W)) {
      gp->player_vel.y -= scaled_player_speed;
    }
    if(is_key_pressed(gp, KBD_KEY_A)) {
      gp->player_vel.x -= scaled_player_speed;
    }
    if(is_key_pressed(gp, KBD_KEY_S)) {
      gp->player_vel.y += scaled_player_speed;
    }
    if(is_key_pressed(gp, KBD_KEY_D)) {
      gp->player_vel.x += scaled_player_speed;
    }

  } /* get keyboard and mouse input */

  int player_tile_col = 0;
  int player_tile_row = 0;

  { /* update world */

    b8 changed_chunk = false;

    v2     tile_rel_limit = { TILE_SIZE_METERS, TILE_SIZE_METERS };
    v2_s32 tile_limit     = { CHUNK_TILES_X_COUNT, CHUNK_TILES_Y_COUNT };
    v2_s32 chunk_limit    = { WORLD_CHUNKS_X_COUNT, WORLD_CHUNKS_Y_COUNT };

    v2_s32 cur_chunk_pos = gp->player_pos.chunk;
    v2_s32 new_chunk_pos = gp->player_pos.chunk;
    v2_s32 cur_tile_pos  = gp->player_pos.tile;
    v2_s32 new_tile_pos  = gp->player_pos.tile;
    v2 cur_tile_rel_pos  = gp->player_pos.tile_rel;
    v2 new_tile_rel_pos  = add_v2(gp->player_pos.tile_rel, gp->player_vel);

    v2_s32 carry     = {0};
    v2     carry_f32 = {0};

    carry_f32        = sub_v2(gte_v2(new_tile_rel_pos, tile_rel_limit), lt_v2(new_tile_rel_pos, (v2){0}));
    carry            = cast_v2_f32_to_s32(carry_f32);
    new_tile_rel_pos = sub_v2(new_tile_rel_pos, mul_v2(carry_f32, tile_rel_limit));

    new_tile_pos  = add_v2_s32(cur_tile_pos, carry);
    carry = sub_v2_s32(gte_v2_s32(new_tile_pos, tile_limit), lt_v2_s32(new_tile_pos, (v2_s32){0}));
    new_tile_pos  = sub_v2_s32(new_tile_pos, mul_v2_s32(carry, tile_limit));

    new_chunk_pos = add_v2_s32(new_chunk_pos, carry);
    carry           = sub_v2_s32(gte_v2_s32(new_chunk_pos, chunk_limit), lt_v2_s32(new_chunk_pos, (v2_s32){0}));
    new_chunk_pos = sub_v2_s32(new_chunk_pos, mul_v2_s32(carry, chunk_limit));

    changed_chunk = (new_chunk_pos.x != cur_chunk_pos.x || new_chunk_pos.y != cur_chunk_pos.y);

    // NOTE jfd 06/02/2026: Best to ignore collisions when transitioning between chunks.
    // Alternatively, if you did want to handle them, you would basically have the 3x3 grid of chunks
    // be what we read from when processing this. But I think for the case where the world is made up of rooms,
    // or large open spaces, this is fine, just avoid placing geometry on only one side of a chunk boundary.
    if(!changed_chunk) {

      { /* tile collisions */

        if(get_tile(gp, new_tile_pos.x, new_tile_pos.y)) {

          new_tile_pos = cur_tile_pos;
          new_tile_rel_pos = cur_tile_rel_pos;

        }

      } /* tile collisions */

    } else {

      { /* set new chunk */

        u8 *tiles = (u8*)(world_tiles[new_chunk_pos.y][new_chunk_pos.x]);

        Chunk new_chunk = {
          .tiles = tiles,
        };

        gp->chunk = new_chunk;
        gp->player_pos.chunk = new_chunk_pos;

      } /* set new chunk */

    }

    gp->player_pos.tile = new_tile_pos;
    gp->player_pos.tile_rel = new_tile_rel_pos;

    player_tile_col = gp->player_pos.tile.x;
    player_tile_row = gp->player_pos.tile.y;

  } /* update world */

  { /* draw */

    clear_screen(gp);

    for(int row = 0; row < CHUNK_TILES_Y_COUNT; row++) {
      for(int col = 0; col < CHUNK_TILES_X_COUNT; col++) {
        u8 tile = get_tile(gp, col, row);

        Color color = { 0.1f, 0.1f, 0.1f, 1 };
        if(tile == 1) {
          color = (Color){ 0.8f, 0.8f, 0.8f, 1 };
        } else if(tile == 2) {
          color = (Color){ 0.0f, 0.8f, 0.4f, 1 };
        }

        v2 tile_screen_point = { (f32)col, (f32)row };
        tile_screen_point    = scale_v2(tile_screen_point, TILE_SIZE_METERS * PIXELS_PER_METER);
        v2 tile_screen_size  = scale_v2((v2){ TILE_SIZE_METERS, TILE_SIZE_METERS }, PIXELS_PER_METER);

        draw_rect(gp,
          color,
          tile_screen_point.x,
          tile_screen_point.y,
          tile_screen_size.x,
          tile_screen_size.y
        );

      }
    }

    // draw_rect_lines(gp, (Color){ 1, 1, 1, 1 }, 1.0f, 20, 20, 80, 80);

    #if 1
    {
      v2 tile_screen_point = { (f32)player_tile_col, (f32)player_tile_row };
      tile_screen_point    = scale_v2(tile_screen_point, TILE_SIZE_METERS * PIXELS_PER_METER);
      v2 tile_screen_size  = scale_v2((v2){ TILE_SIZE_METERS, TILE_SIZE_METERS }, PIXELS_PER_METER);
      draw_rect_lines(gp,
        (Color){ 0, 0.4f, 0.9f, 1 },
        2.0f,
        tile_screen_point.x,
        tile_screen_point.y,
        tile_screen_size.x,
        tile_screen_size.y
      );
    }
    #endif

    World_pos player_pos = gp->player_pos;
    v2 player_screen_pos              = scale_v2(add_v2(player_pos.tile_rel, scale_v2(cast_v2_s32_to_f32(player_pos.tile), TILE_SIZE_METERS)), PIXELS_PER_METER);
    v2 player_rect_screen_size        = scale_v2((v2){ gp->player_width, gp->player_height }, PIXELS_PER_METER);
    v2 half_player_screen_size        = scale_v2(player_rect_screen_size, 0.5f);
    v2 player_rect_screen_pos         = sub_v2(player_screen_pos, half_player_screen_size);
    v2 player_center_rect_screen_pos  = sub_v2(player_screen_pos, scale_v2(half_player_screen_size, 0.2f));
    v2 player_center_rect_screen_size = scale_v2(player_rect_screen_size, 0.2f);

    draw_rect(gp,
      (Color){ 0.95f, 0.2f, 0.4f, 0.8f },
      player_rect_screen_pos.x,
      player_rect_screen_pos.y,
      player_rect_screen_size.x,
      player_rect_screen_size.y
    );

    draw_rect(gp,
      (Color){ 0.95f, 0.8f, 0.0f, 1.0f },
      player_center_rect_screen_pos.x,
      player_center_rect_screen_pos.y,
      player_center_rect_screen_size.x,
      player_center_rect_screen_size.y
    );

  } /* draw */

  arena_clear(gp->frame_arena);

}

shared_function void
func game_get_sound_samples(Game *gp) {

  // TODO jfd: allow sample offsets here for more robust platform options

  debug_silence(gp);
  // debug_output_sound(gp);

}


#endif
