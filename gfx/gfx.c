#ifndef GFX_C
#define GFX_C


b32 gfx_is_modifier_key(GFX_Key key) {
  b32 result = (key >= GFX_KEY_LEFT_SHIFT && key <= GFX_KEY_LEFT_META);
  return result;
}

b32 gfx_was_key_pressed_once(GFX_Input *input, GFX_Key key) {
  return !!(input->key_pressed[key] == 1);
}

b32 gfx_is_key_pressed(GFX_Input *input, GFX_Key key) {
  return !!(input->key_pressed[key] > 0);
}

b32 gfx_was_key_released(GFX_Input *input, GFX_Key key) {
  return (input->key_released[key] == true);
}

// TODO jfd: mouse clicks and scroll wheel
void gfx_poll_input_events(GFX_EventList *event_list, GFX_Input *input) {
  memory_zero(input->key_released, sizeof(input->key_released));

  for(GFX_Event *event = event_list->first; event; event = event->next) {

    switch(event->kind) {
      case GFX_EVENT_KEY_PRESS: {

        switch(event->key) {
          case GFX_KEY_LEFT_CONTROL: case GFX_KEY_RIGHT_CONTROL: {
            input->modifier_mask |= GFX_MOD_CONTROL;
          } break;
          case GFX_KEY_LEFT_SHIFT: case GFX_KEY_RIGHT_SHIFT: {
            input->modifier_mask |= GFX_MOD_SHIFT;
          } break;
          case GFX_KEY_LEFT_ALT: case GFX_KEY_RIGHT_ALT: {
            input->modifier_mask |= GFX_MOD_ALT;
          } break;
          default: {
            input->key_pressed[event->key] += 1 + event->repeat_count;
          } break;
        }

      } break;
      case GFX_EVENT_KEY_RELEASE: {

        switch(event->key) {
          case GFX_KEY_LEFT_CONTROL: case GFX_KEY_RIGHT_CONTROL: {
            input->modifier_mask &= ~GFX_MOD_CONTROL;
          } break;
          case GFX_KEY_LEFT_SHIFT: case GFX_KEY_RIGHT_SHIFT: {
            input->modifier_mask &= ~GFX_MOD_SHIFT;
          } break;
          case GFX_KEY_LEFT_ALT: case GFX_KEY_RIGHT_ALT: {
            input->modifier_mask &= ~GFX_MOD_ALT;
          } break;
          default: {
            input->key_pressed[event->key] = 0;
            input->key_released[event->key] = true;
          } break;
        }

      } break;
      case GFX_EVENT_MOUSE_MOVE: {
        input->mouse_pos = event->mouse_pos;
        input->mouse_delta.x = event->mouse_pos.x - input->mouse_pos.x;
        input->mouse_delta.y = event->mouse_pos.y - input->mouse_pos.y;
      } break;
    }
  }

  // TODO jfd: remove this
  event_list->count = 0;
  event_list->first = event_list->last = 0;
  // arena_clear(a);

}


#endif
