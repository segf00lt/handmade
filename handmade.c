#ifndef HANDMADE_C
#define HANDMADE_C

b32 was_key_pressed_once(Game *gp, OS_Key key) {
  return !!(gp->key_pressed[key] == 1);
}

b32 is_key_pressed(Game *gp, OS_Key key) {
  return !!(gp->key_pressed[key] > 0);
}

b32 was_key_released(Game *gp, OS_Key key) {
  return (gp->key_released[key] == true);
}


void render_weird_gradient(Game *gp, int x_offset, int y_offset) {
  u8 *row = gp->render_buffer;
  for(int y = 0; y < gp->render_height; y++) {
    u32 *pixel = (u32*)row;

    for(int x = 0; x < gp->render_width; x++) {
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

    row += gp->render_stride;
  }

}

void game_update_and_render(Game *gp) {

  // TODO jfd: mouse movement and clicks
  { /* get keyboard and mouse input */

    int step_pixels = 1;

    if(is_key_pressed(gp, OS_KEY_W)) {
      gp->y_offset -= step_pixels;
    }
    if(is_key_pressed(gp, OS_KEY_A)) {
      gp->x_offset -= step_pixels;
    }
    if(is_key_pressed(gp, OS_KEY_S)) {
      gp->y_offset += step_pixels;
    }
    if(is_key_pressed(gp, OS_KEY_D)) {
      gp->x_offset += step_pixels;
    }

  } /* get keyboard and mouse input */


  { /* draw */

    render_weird_gradient(gp, gp->x_offset, gp->y_offset);

  } /* draw */

}





#endif
