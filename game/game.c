#ifndef GAME_C
#define GAME_C


/////////////////////////////////
// globals

#include "game_random.h"

#define TILE_KIND_MAX 5


/////////////////////////////////
// functions


internal u32
func get_random(Game *gp) {
  if(gp->random_number_index >= ARRLEN(game_random_choices)) {
    gp->random_number_index = 0;
  }
  u32 result = game_random_choices[gp->random_number_index++];
  return result;
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

internal v2_s32
func chunk_pos_from_point(Game *gp, v2 v) {
  v2_s32 chunk = cast_v2_f32_to_s32(scale_v2(v, 1.0f/CHUNK_SIZE));

  return chunk;
}

internal void
func recanonicalize_coord(Game *gp, u32 *tile, f32 *tile_rel) {
  // NOTE jfd: casey made the tiles be drawn from their center, but that doesn't really change anything for us
  // s32 carry = (s32)floor_f32(*tile_rel * (1.0f/TILE_SIZE_METERS));
  s32 carry = (s32)round_f32(*tile_rel * (1.0f/TILE_SIZE_METERS));
  *tile += carry;
  *tile_rel -= (f32)carry * TILE_SIZE_METERS;
  // ASSERT(*tile_rel >= 0.0f);
  // ASSERT(*tile_rel <= TILE_SIZE_METERS);
  ASSERT(*tile_rel >= -0.5f*TILE_SIZE_METERS);
  ASSERT(*tile_rel <= 0.5f*TILE_SIZE_METERS);
}

internal Tile_map_pos
func recanonicalize_pos(Game *gp, Tile_map_pos pos) {
  Tile_map_pos result = pos;

  recanonicalize_coord(gp, &result.tile.x, &result.tile_rel.x);
  recanonicalize_coord(gp, &result.tile.y, &result.tile_rel.y);

  return result;
}

internal Chunk_pos
func chunk_pos_from_tile_map_pos(Game *gp, Tile_map_pos pos) {
  Chunk_pos result;

  u32 abs_tile_x = pos.tile.x;
  u32 abs_tile_y = pos.tile.y;

  result = chunk_pos_from_abs_tile_pos(gp, abs_tile_x, abs_tile_y);

  return result;
}

internal Chunk_pos
func chunk_pos_from_abs_tile_pos(Game *gp, u32 abs_tile_x, u32 abs_tile_y) {
  Chunk_pos result;

  result.chunk.x = (s32)abs_tile_x >> CHUNK_SHIFT;
  result.chunk.y = (s32)abs_tile_y >> CHUNK_SHIFT;
  result.tile.x  = abs_tile_x & CHUNK_MASK;
  result.tile.y  = abs_tile_y & CHUNK_MASK;

  return result;
}

internal Tile_map_pos
func tile_map_pos_from_point(Game *gp, f32 x, f32 y) {
  Tile_map_pos result = {0};
  v2 v = { x, y };

  v2 tile = truncate_v2(scale_v2(v, 1.0f/TILE_SIZE_METERS));

  v2 tile_rel = sub_v2(v, scale_v2(tile, TILE_SIZE_METERS));

  result.tile_rel = tile_rel;

  result.tile = cast_v2_f32_to_u32(tile);

  return result;
}

force_inline Chunk*
func get_chunk(Game *gp, s32 chunk_x, s32 chunk_y) {
  Chunk *chunk = 0;
  // TODO jfd: load chunks if not already loaded

  if(chunk_x >= 0 && chunk_x < (s32)gp->world_chunks_x_count && chunk_y >= 0 && chunk_y < (s32)gp->world_chunks_y_count) {
    chunk = &gp->world_chunks[chunk_x + chunk_y*gp->world_chunks_x_count];
  }

  return chunk;
}

force_inline u8
func get_tile_of_chunk(Game *gp, Chunk *chunk, s32 tile_x, s32 tile_y) {
  ASSERT((tile_x >= 0) && (tile_x < CHUNK_SIZE) && (tile_y >= 0) && (tile_y < CHUNK_SIZE));
  u8 tile = 0;
  if(chunk) {
    tile = chunk->tiles[tile_x + tile_y*CHUNK_SIZE];
  }
  return tile;
}

force_inline void
func set_tile_of_chunk(Game *gp, Chunk *chunk, s32 tile_x, s32 tile_y, u8 tile_value) {
  ASSERT((tile_x >= 0) && (tile_x < CHUNK_SIZE) && (tile_y >= 0) && (tile_y < CHUNK_SIZE));
  if(chunk) {
    chunk->tiles[tile_x + tile_y*CHUNK_SIZE] = tile_value;
  }
}

force_inline u8
func tile_from_tile_map_pos(Game *gp, Tile_map_pos pos) {
  u8 tile = 0;

  Chunk_pos chunk_pos = chunk_pos_from_tile_map_pos(gp, pos);
  Chunk *chunk = get_chunk(gp, chunk_pos.chunk.x, chunk_pos.chunk.y);
  tile = get_tile_of_chunk(gp, chunk, chunk_pos.tile.x, chunk_pos.tile.y);

  return tile;
}

internal void
func set_tile_from_abs_tile_pos(Game *gp, u32 abs_tile_x, u32 abs_tile_y, u8 tile_value) {
  Chunk_pos chunk_pos = chunk_pos_from_abs_tile_pos(gp, abs_tile_x, abs_tile_y);
  Chunk *chunk = get_chunk(gp, chunk_pos.chunk.x, chunk_pos.chunk.y);
  set_tile_of_chunk(gp, chunk, chunk_pos.tile.x, chunk_pos.tile.y, tile_value);
}

internal void
func init_player(Game *gp) {
  gp->player_pos = tile_map_pos_from_point(gp, 4, 2);
  gp->player_width = CM(150);
  gp->player_height = CM(150);
}

internal void
func init_tile_map(Game *gp) {
  gp->world_chunks_x_count = 4;
  gp->world_chunks_y_count = 4;

  gp->world_chunks = push_array_no_zero(gp->main_arena, Chunk, gp->world_chunks_x_count*gp->world_chunks_y_count);

  for(u32 i = 0; i < gp->world_chunks_y_count; i++) {
    for(u32 j = 0; j < gp->world_chunks_x_count; j++) {
      Chunk chunk = {
        .tiles = push_array(gp->main_arena, u8, CHUNK_SIZE*CHUNK_SIZE),
      };

      gp->world_chunks[j + i*gp->world_chunks_x_count] = chunk;
    }
  }

  u32 screen_x = 0;
  u32 screen_y = 0;

  u32 tiles_per_width = 35;
  u32 tiles_per_height = 20;
  for(u32 screen_index = 0; screen_index < 100; screen_index++) {

    for(u32 tile_y = 0; tile_y < tiles_per_height; tile_y++) {
      for(u32 tile_x = 0; tile_x < tiles_per_width; tile_x++) {
        u32 abs_tile_x = screen_x*tiles_per_width + tile_x;
        u32 abs_tile_y = screen_y*tiles_per_height + tile_y;

        u8 tile_value = 0;

        if((tile_x == 0) || (tile_x == (tiles_per_width - 1))) {
          if(tile_y == (tiles_per_height / 2)) {
            tile_value = 0;
          } else {
            tile_value = 1;
          }
        }

        if((tile_y == 0) || (tile_y == (tiles_per_height - 1))) {
          if(tile_x == (tiles_per_width / 2)) {
            tile_value = 0;
          } else {
            tile_value = 1;
          }
        }

        set_tile_from_abs_tile_pos(gp, abs_tile_x, abs_tile_y, tile_value);

      }
    }

    u32 random_choice = get_random(gp) % 2;
    if(random_choice == 0) {
      screen_x += 1;
    } else {
      screen_y += 1;
    }

  }

}


internal void
func draw_tile_by_point(Game *gp, Color color, v2 point) {
  Tile_map_pos tile_map_pos = tile_map_pos_from_point(gp, point.x, point.y);

  int tile_col = tile_map_pos.tile.x;
  int tile_row = tile_map_pos.tile.y;

  v2 tile_screen_point = { (f32)tile_col * TILE_SIZE_METERS, (f32)(tile_row & CHUNK_MASK) * TILE_SIZE_METERS };
  tile_screen_point = sub_v2(tile_screen_point, gp->camera_pos);
  tile_screen_point    = scale_v2(tile_screen_point, PIXELS_PER_METER);
  tile_screen_point.y = gp->render.height - tile_screen_point.y - TILE_SIZE_METERS*PIXELS_PER_METER;
  v2 tile_screen_size  = scale_v2((v2){ TILE_SIZE_METERS, TILE_SIZE_METERS }, PIXELS_PER_METER);
  draw_rect(gp,
    color,
    tile_screen_point.x,
    tile_screen_point.y,
    tile_screen_size.x,
    tile_screen_size.y
  );
}

internal void
func draw_tile_map(Game *gp) {

  // Tile_map_pos camera_tile_map_pos = tile_map_pos_from_point(gp, gp->camera_pos.x, gp->camera_pos.y);

  Tile_map_pos camera_tile_map_pos = gp->player_pos;

  s32 rows_to_draw = (s32)(((f32)gp->render.height*METERS_PER_PIXEL) / TILE_SIZE_METERS);
  s32 cols_to_draw = (s32)(((f32)gp->render.width*METERS_PER_PIXEL)  / TILE_SIZE_METERS);

  s32 half_rows_to_draw = rows_to_draw >> 1;
  s32 half_cols_to_draw = cols_to_draw >> 1;


  for(int row = -half_rows_to_draw - 2; row <= half_rows_to_draw+2; row++) {
    for(int col = -half_cols_to_draw - 2; col <= half_cols_to_draw+2; col++) {

      Tile_map_pos cur_tile_map_pos = {0};
      cur_tile_map_pos.tile.x = (s32)camera_tile_map_pos.tile.x + col;
      cur_tile_map_pos.tile.y = (s32)camera_tile_map_pos.tile.y + row - 1;

      u8 tile = tile_from_tile_map_pos(gp, cur_tile_map_pos);

      Color tile_colors[TILE_KIND_MAX] = {
        { 0.1f,  0.1f, 0.1f, 1 },
        { 0.8f,  0.8f, 0.8f, 1 },
        { 0.0f,  0.8f, 0.4f, 1 },
        { 0.36f, 0.2f, 0.1f, 1 },
        { 0.96f, 1.0f, 0.0f, 1 },
      };

      Color color = tile_colors[tile];

      #if 1
      if(cur_tile_map_pos.tile.x % CHUNK_SIZE == 0 || cur_tile_map_pos.tile.y % CHUNK_SIZE == 0) {
        color = alpha_blend(color, (Color){ 1.0f, 0.5f, 0, 0.5f });
      }
      #endif

      s32 tile_row = half_rows_to_draw + row;
      s32 tile_col = half_cols_to_draw + col;
      v2 tile_screen_point = { (f32)(tile_col) * TILE_SIZE_METERS, (f32)((tile_row)) * TILE_SIZE_METERS };
      tile_screen_point = sub_v2(tile_screen_point, camera_tile_map_pos.tile_rel);
      tile_screen_point = add_value_v2(tile_screen_point, TILE_SIZE_METERS*0.5);
      tile_screen_point = scale_v2(tile_screen_point, PIXELS_PER_METER);
      tile_screen_point.y = gp->render.height - tile_screen_point.y;


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

}

shared_function void
func game_update_and_render(Game *gp) {

  if(gp->did_reload) {
    gp->did_reload = false;


    gp->once = false;

  }

  if(gp->once) {
    gp->once = false;
    gp->should_init_player = true;

  }


  { /* run_once */


    if(gp->should_init_player) {
      gp->should_init_player = false;

      init_player(gp);
    }

  } /* run_once */


  { /* get keyboard and mouse input */

    // TODO jfd: mouse movement and clicks
    f32 player_speed = PLAYER_MOVE_SPEED;

    f32 scaled_player_speed = player_speed * gp->t;

    if(is_key_pressed(gp, KBD_KEY_LEFT_SHIFT)) {
      scaled_player_speed *= 0.2f;
    }

    gp->player_vel = (v2){0};

    if(is_key_pressed(gp, KBD_KEY_W)) {
      gp->player_vel.y += scaled_player_speed;
    }
    if(is_key_pressed(gp, KBD_KEY_A)) {
      gp->player_vel.x -= scaled_player_speed;
    }
    if(is_key_pressed(gp, KBD_KEY_S)) {
      gp->player_vel.y -= scaled_player_speed;
    }
    if(is_key_pressed(gp, KBD_KEY_D)) {
      gp->player_vel.x += scaled_player_speed;
    }

  } /* get keyboard and mouse input */

  int player_tile_col = 0;
  int player_tile_row = 0;

  { /* update world */

    Tile_map_pos new_pos = gp->player_pos;
    new_pos.tile_rel = add_v2(new_pos.tile_rel, gp->player_vel);
    new_pos = recanonicalize_pos(gp, new_pos);

    { /* tile collisions */

      if(tile_from_tile_map_pos(gp, new_pos)) {
        new_pos = gp->player_pos;
      }

    } /* tile collisions */

    gp->player_pos = new_pos;

    player_tile_col = gp->player_pos.tile.x;
    player_tile_row = gp->player_pos.tile.y;

  } /* update world */

  { /* draw */

    Tile_map_pos player_tile_map_pos = gp->player_pos;

    v2 player_tile_rel = { player_tile_map_pos.tile_rel.x, player_tile_map_pos.tile_rel.y };
    v2 player_tile = { (f32)(player_tile_map_pos.tile.x & CHUNK_MASK), (f32)(player_tile_map_pos.tile.y & CHUNK_MASK) };
    v2 player_pos = add_v2(player_tile_rel, scale_v2(player_tile, TILE_SIZE_METERS));

    /* update_camera */
    gp->camera_pos = (v2){
      player_pos.x - ((f32)gp->render.width)*METERS_PER_PIXEL*0.5f,
      player_pos.y - ((f32)gp->render.height)*METERS_PER_PIXEL*0.5f,
    };

    clear_screen(gp);


    draw_tile_map(gp);

    // TODO jfd: make tile rendering consistet with draw_tile_map()
    #if 0
    {
      v2 tile_screen_point = { (f32)player_tile_col * TILE_SIZE_METERS, (f32)(player_tile_row) * TILE_SIZE_METERS };
      tile_screen_point = add_value_v2(tile_screen_point, 0.5f*TILE_SIZE_METERS);
      tile_screen_point = sub_v2(tile_screen_point, gp->player_pos.tile_rel);
      tile_screen_point    = scale_v2(tile_screen_point, PIXELS_PER_METER);
      tile_screen_point.y = gp->render.height - tile_screen_point.y;
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

    #if 0
    {
      Color color = { 0.6f, 0.0f, 1.0f, 1 };
      draw_tile_by_point(gp, color, player_pos);
    }
    #endif

    v2 player_screen_pos = scale_v2(sub_v2(add_value_v2(player_pos, 0.5f*TILE_SIZE_METERS), gp->camera_pos), PIXELS_PER_METER);
    // v2 player_screen_pos = scale_v2(sub_v2(player_pos, gp->camera_pos), PIXELS_PER_METER);
    player_screen_pos.y = gp->render.height - player_screen_pos.y;

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
  // TODO jfd 23/02/26: there is some popping in the sound, not sure if it has to do with hot-reloading
  // debug_output_sound(gp);

}


shared_function Game*
func game_init(Platform *pp) {
  ASSERT(GAME_STATE_SIZE < pp->backbuffer_size);

  u64 alloc_size = pp->backbuffer_size;
  u8 *alloc_ptr = (u8*)(pp->backbuffer);

  Game *gp = (Game*)alloc_ptr;
  alloc_ptr += GAME_STATE_SIZE;
  alloc_size -= GAME_STATE_SIZE;

  u64 game_frame_arena_backbuffer_size = MB(5);
  u64 game_temp_arena_backbuffer_size  = MB(5);
  u64 game_main_arena_backbuffer_size = alloc_size - game_frame_arena_backbuffer_size - game_temp_arena_backbuffer_size;

  u8 *game_main_arena_backbuffer = alloc_ptr;
  alloc_ptr += game_main_arena_backbuffer_size;

  u8 *game_frame_arena_backbuffer = alloc_ptr;
  alloc_ptr += game_frame_arena_backbuffer_size;

  u8 *game_temp_arena_backbuffer = alloc_ptr;
  alloc_ptr += game_temp_arena_backbuffer_size;

  ASSERT(alloc_ptr <= ((u8*)pp->backbuffer + pp->backbuffer_size));

  gp->main_arena  = arena_create_ex(game_main_arena_backbuffer_size, true, game_main_arena_backbuffer);
  gp->frame_arena = arena_create_ex(game_frame_arena_backbuffer_size, true, game_frame_arena_backbuffer);
  gp->temp_arena  = arena_create_ex(game_temp_arena_backbuffer_size,  true, game_temp_arena_backbuffer);

  gp->did_reload = true;

  init_player(gp);

  init_tile_map(gp);

  return gp;
}


#endif
