#ifndef GAME_C
#define GAME_C

#define TILE_MAP_ROWS    10
#define TILE_MAP_COLUMNS 18
#define TILE_SIZE        55

/////////////////////////////////
// globals

#ifdef HANDMADE_HOTRELOAD

Platform_get_keyboard_modifiers_func *platform_get_keyboard_modifiers;
Platform_read_entire_file_func  *platform_read_entire_file;
Platform_write_entire_file_func *platform_write_entire_file;

#endif




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

  u8 a = (u8)(color.a*MAX_U8);
  u8 r = (u8)(color.r*MAX_U8);
  u8 g = (u8)(color.g*MAX_U8);
  u8 b = (u8)(color.b*MAX_U8);

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

  for(int y = begin_y; y < begin_y + round_f32_to_s32(line_thickness); y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < end_x; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }


  for(int y = begin_y + round_f32_to_s32(line_thickness); y < end_y - round_f32_to_s32(line_thickness); y++) {
    u32 *pixel_row = (u32*)(row + y * gp->render.stride);

    for(int x = begin_x; x < begin_x + round_f32_to_s32(line_thickness); x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

    for(int x = end_x - round_f32_to_s32(line_thickness); x < end_x; x++) {

      Color cur_color = color_from_pixel(pixel_row[x]);

      Color final_color = alpha_blend(cur_color, color);

      u32 final_pixel_color = pixel_from_color(final_color);

      pixel_row[x] = final_pixel_color;
    }

  }

  for(int y = end_y - round_f32_to_s32(line_thickness); y < end_y; y++) {
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

internal void
func tile_from_point(f32 x, f32 y, int *tile_x, int *tile_y) {

  f32 canon_x = x / TILE_SIZE + 0.5f;
  f32 canon_y = y / TILE_SIZE + 0.5f;
  // TODO jfd: handle negative values in round_f32_to_s32()
  *tile_x = round_f32_to_s32(canon_x) - 1;
  *tile_y = round_f32_to_s32(canon_y) - 1;

  *tile_x = (*tile_x >= TILE_MAP_COLUMNS) ? (TILE_MAP_COLUMNS - 1) : MAX(*tile_x, 0);
  *tile_y = (*tile_y >= TILE_MAP_ROWS)    ? (TILE_MAP_ROWS - 1)    : MAX(*tile_y, 0);

}

shared_function void
func game_update_and_render(Game *gp) {

  if(gp->did_reload) {
    gp->did_reload = false;

    // gp->should_init_player = true;

  }

  { /* run_once */

    if(gp->should_init_player) {
      gp->should_init_player = false;

      gp->player_x = 400;
      gp->player_y = 100;
      gp->player_width = 30;
      gp->player_height = 30;
    }

  } /* run_once */

  u8 tile_map[TILE_MAP_ROWS][TILE_MAP_COLUMNS] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  },
    { 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0,  },
    { 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  },
    { 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  },
    { 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,  },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  },
  };
  Vec2 tile_map_origin = {
    0,
    0,
  };
  f32 tile_size = TILE_SIZE;
  f32 tile_map_width = ARRLEN(tile_map[0]) * tile_size;
  f32 tile_map_height = ARRLEN(tile_map) * tile_size;


  clear_screen(gp);

  { /* get keyboard and mouse input */

    // TODO jfd: mouse movement and clicks
    f32 player_speed = 4.80f*tile_size;

    f32 scaled_player_speed = player_speed * gp->t;


    gp->player_vel_x = 0;
    gp->player_vel_y = 0;
    if(is_key_pressed(gp, KBD_KEY_W)) {
      gp->player_vel_y -= scaled_player_speed;
    }
    if(is_key_pressed(gp, KBD_KEY_A)) {
      gp->player_vel_x -= scaled_player_speed;
    }
    if(is_key_pressed(gp, KBD_KEY_S)) {
      gp->player_vel_y += scaled_player_speed;
    }
    if(is_key_pressed(gp, KBD_KEY_D)) {
      gp->player_vel_x += scaled_player_speed;
    }

  } /* get keyboard and mouse input */

  int player_tile_x = 0;
  int player_tile_y = 0;

  { /* update world */

    f32 new_player_x = gp->player_x + gp->player_vel_x;
    f32 new_player_y = gp->player_y + gp->player_vel_y;

    int cur_player_tile_x;
    int cur_player_tile_y;
    tile_from_point(gp->player_x, gp->player_y, &cur_player_tile_x, &cur_player_tile_y);

    int new_player_tile_x;
    int new_player_tile_y;
    tile_from_point(new_player_x, new_player_y, &new_player_tile_x, &new_player_tile_y);

    if(tile_map[new_player_tile_y][new_player_tile_x]) {

      if(new_player_tile_x > cur_player_tile_x) {

      } else {
      }

      if(new_player_tile_y > cur_player_tile_y) {
      } else {
      }

    } else {
      gp->player_x = new_player_x;
      gp->player_y = new_player_y;
    }

    player_tile_x = new_player_tile_x;
    player_tile_y = new_player_tile_y;

  } /* update world */

  { /* draw */

    for(int row = 0; row < TILE_MAP_ROWS; row++) {
      for(int col = 0; col < TILE_MAP_COLUMNS; col++) {
        u8 tile_id = tile_map[row][col];
        f32 color = 0.1f;
        if(tile_id) {
          color = 0.8f;
        }

        draw_rect(gp, (Color){ color, color, color, 1 }, tile_map_origin.x + col*tile_size, tile_map_origin.y + row*tile_size, tile_size, tile_size);
      }
    }

    // draw_rect_lines(gp, (Color){ 1, 1, 1, 1 }, 1.0f, 20, 20, 80, 80);

    draw_rect(gp,
      (Color){ 0, 0.4f, 0.9f, 1 },
      tile_map_origin.x + player_tile_x*tile_size,
      tile_map_origin.y + player_tile_y*tile_size,
      tile_size, tile_size
    );
    draw_rect(gp,
      (Color){ 0.95f, 0.2f, 0.4f, 0.8f },
      gp->player_x - 15,
      gp->player_y - 15,
      30,
      30
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
