#ifndef GAME_C
#define GAME_C


/////////////////////////////////
// globals

#ifdef HANDMADE_HOTRELOAD

Platform_get_keyboard_modifiers_func *platform_get_keyboard_modifiers;
Platform_read_entire_file_func  *platform_read_entire_file;
Platform_write_entire_file_func *platform_write_entire_file;

#endif




/////////////////////////////////
// functions


Game_vtable
func game_load_procs(void) {
  Game_vtable vtable = {
    .init              = &game_init,
    .update_and_render = &game_update_and_render,
    .get_sound_samples = &game_get_sound_samples,
  };

  return vtable;
}

Game*
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

  gp->once = true;

  #ifdef HANDMADE_HOTRELOAD

  platform_get_keyboard_modifiers  = pp->vtable.get_keyboard_modifiers;
  platform_read_entire_file  = pp->vtable.read_entire_file;
  platform_write_entire_file = pp->vtable.write_entire_file;

  #endif

  return gp;
}


internal void
func render_weird_gradient(Game *gp, int x_offset, int y_offset) {
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
func output_sound(Game *gp) {
  Game_sound_buffer *sound_buffer = &gp->sound;

  s16 tone_volume = 600;
  int tone_hz = 256;
  int wave_period = sound_buffer->samples_per_second / tone_hz;

  s16 *sample_out = sound_buffer->samples;
  for(s32 sample_index = 0; sample_index < sound_buffer->sample_count; sample_index++) {
    f32 sine_value = sinf(gp->t_sine);

    #if 0
    OutputDebugStringA(cstrf(gp->temp_arena, "sine_value = %f\n", sine_value));
    arena_clear(gp->temp_arena);
    #endif

    s16 sample_value = (s16)(sine_value * tone_volume);

    *sample_out++ = sample_value;
    *sample_out++ = sample_value;

    gp->t_sine += (2.0f*PI*1.0f/((f32)wave_period));
    if(gp->t_sine > 2.0f*PI) {
      gp->t_sine -= 2.0f*PI;
    }

  }

}

void
func game_update_and_render(Game *gp) {


  { /* get keyboard and mouse input */

    // TODO jfd: mouse movement and clicks
    int step_pixels = 8;

    if(is_key_pressed(gp, KBD_KEY_W)) {
      gp->y_offset -= step_pixels;
    }
    if(is_key_pressed(gp, KBD_KEY_A)) {
      gp->x_offset -= step_pixels;
    }
    if(is_key_pressed(gp, KBD_KEY_S)) {
      gp->y_offset += step_pixels;
    }
    if(is_key_pressed(gp, KBD_KEY_D)) {
      gp->x_offset += step_pixels;
    }

  } /* get keyboard and mouse input */


  { /* draw */

    render_weird_gradient(gp, gp->x_offset, gp->y_offset);

  } /* draw */

  arena_clear(gp->frame_arena);

}

void
func game_get_sound_samples(Game *gp) {

  // TODO jfd: allow sample offsets here for more robust platform options
  output_sound(gp);

}


#endif
