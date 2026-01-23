#ifndef GAME_C
#define GAME_C



/////////////////////////////////
// globals





/////////////////////////////////
// functions

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

      *pixel++ = (r << 16) | (g << 8) | b;
    }

    row += gp->render.stride;
  }

}


internal b32
func game_was_key_pressed_once(Game *gp, KeyboardKey key) {
  return !!(gp->input.key_pressed[key] == 1);
}

internal b32
func game_is_key_pressed(Game *gp, KeyboardKey key) {
  return !!(gp->input.key_pressed[key] > 0);
}

internal b32
func game_was_key_released(Game *gp, KeyboardKey key) {
  return (gp->input.key_released[key] == true);
}

internal void
func game_output_sound(Game *gp) {
  Game_SoundBuffer *sound_buffer = &gp->sound;

  local_persist f32 t_sine;
  s16 tone_volume = 600;
  int tone_hz = 256;
  int wave_period = sound_buffer->samples_per_second / tone_hz;

  s16 *sample_out = sound_buffer->samples;
  for(s32 sample_index = 0; sample_index < sound_buffer->sample_count; sample_index++) {
    f32 sine_value = sinf(t_sine);
    s16 sample_value = (s16)(sine_value * tone_volume);

    *sample_out++ = sample_value;
    *sample_out++ = sample_value;

    t_sine += 2.0f*PI*1.0f/((f32)wave_period);
  }

}

internal void
func game_update_and_render(Game *gp) {

  local_persist b32 once = true;
  if(once) {
    once = false;

    Str8 test_file_data = platform_debug_read_entire_file(str8_lit("game.c"));
    if(platform_debug_write_entire_file(test_file_data, str8_lit("copy_of_game.txt"))) {
      OutputDebugStringA("cowabunga file copied\n");
    }
  }


  { /* get keyboard and mouse input */

    // TODO jfd: mouse movement and clicks
    int step_pixels = 1;

    if(game_is_key_pressed(gp, KBD_KEY_W)) {
      gp->y_offset -= step_pixels;
    }
    if(game_is_key_pressed(gp, KBD_KEY_A)) {
      gp->x_offset -= step_pixels;
    }
    if(game_is_key_pressed(gp, KBD_KEY_S)) {
      gp->y_offset += step_pixels;
    }
    if(game_is_key_pressed(gp, KBD_KEY_D)) {
      gp->x_offset += step_pixels;
    }

  } /* get keyboard and mouse input */


  { /* play sound */

    // TODO jfd: allow sample offsets here for more robust platform options
    game_output_sound(gp);

  } /* play sound */

  { /* draw */

    render_weird_gradient(gp, gp->x_offset, gp->y_offset);

  } /* draw */

  arena_clear(gp->frame_arena);

}



#endif
