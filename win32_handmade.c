#include "base.h"

/*
 * tables
 */

/*
 * types
 */

typedef struct Game Game;

typedef struct OS_Win32_Backbuffer OS_Win32_Backbuffer;

/*
 * structs
 */

// TODO jfd: move this to os_gfx
struct OS_Win32_Backbuffer {
  BITMAPINFO bitmap_info;
  u8 *bitmap_memory;
  s32 bitmap_width;
  s32 bitmap_height;
  u32 bytes_per_pixel;
  u32 stride;
};

typedef struct OS_Win32_SoundOutput OS_Win32_SoundOutput;
struct OS_Win32_SoundOutput {
  int samples_per_second;
  int buffer_size;
  s16 tone_volume;
  int tone_hz;
  u32 running_sample_index;
  int wave_period;
  int half_wave_period;
  int bytes_per_sample;
};

struct Game {
  OS_Modifier modifier_mask;
  u32 key_pressed[OS_KEY_MAX];
  b32 key_released[OS_KEY_MAX];
  Vec2 mouse_pos;
  Vec2 mouse_delta;
};

/*
 * headers
 */


LRESULT CALLBACK os_win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

void os_win32_resize_backbuffer(OS_Win32_Backbuffer *backbuffer, int window_width, int window_height);

void os_win32_display_buffer_in_window(OS_Win32_Backbuffer *backbuffer, HDC device_context, int window_width, int window_height, int x, int y, int width, int height);

OS_Win32_WindowDimensions os_win32_get_window_dimensions(HWND window_handle);

void poll_events(Game *gp, OS_EventList *event_list);
b32  is_key_pressed(Game *gp, OS_Key key);
b32  was_key_pressed_once(Game *gp, OS_Key key);
b32  was_key_released(Game *gp, OS_Key key);

void os_win32_load_xinput(void);

#define OS_WIN32_XINPUT_SET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef OS_WIN32_XINPUT_SET_STATE(OS_Win32_XInput_SetStateFunc);

OS_WIN32_XINPUT_SET_STATE(_os_win32_xinput_set_state_stub) {
  return 0;
}

#define OS_WIN32_XINPUT_GET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef OS_WIN32_XINPUT_GET_STATE(OS_Win32_XInput_GetStateFunc);

OS_WIN32_XINPUT_GET_STATE(_os_win32_xinput_get_state_stub) {
  return 0;
}

#define OS_WIN32_DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
typedef OS_WIN32_DIRECT_SOUND_CREATE(OS_Win32_DirectSound_CreateFunc);
OS_WIN32_DIRECT_SOUND_CREATE(_os_win32_direct_sound_create_stub) {
  return 0;
}

/*
 * globals
 */

global Arena *scratch;
global Arena *app_event_arena;

global OS_Win32_XInput_GetStateFunc *os_win32_xinput_get_state = _os_win32_xinput_get_state_stub;
global OS_Win32_XInput_SetStateFunc *os_win32_xinput_set_state = _os_win32_xinput_set_state_stub;

global b32 app_is_running;
global OS_Win32_Backbuffer global_backbuffer = {
  .bytes_per_pixel = 4,
};

global OS_Win32_SoundOutput _app_sound_output_stub;
global OS_Win32_SoundOutput *app_sound_output = &_app_sound_output_stub;

global LPDIRECTSOUNDBUFFER app_sound_buffer; // this is what we write to

global Game _game_state_stub;
global Game *game_state = &_game_state_stub;
global OS_EventList _app_event_list_stub;
global OS_EventList *app_event_list = &_app_event_list_stub;

/*
 * functions
 */

void render_weird_gradient(OS_Win32_Backbuffer *backbuffer, int x_offset, int y_offset) {
  u8 *row = (u8*)backbuffer->bitmap_memory;
  for(int y = 0; y < backbuffer->bitmap_height; y++) {
    u32 *pixel = (u32*)row;

    for(int x = 0; x < backbuffer->bitmap_width; x++) {
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

    row += backbuffer->stride;
  }

}



b32 was_key_pressed_once(Game *gp, OS_Key key) {
  return !!(gp->key_pressed[key] == 1);
}

b32 is_key_pressed(Game *gp, OS_Key key) {
  return !!(gp->key_pressed[key] > 0);
}

b32 was_key_released(Game *gp, OS_Key key) {
  return (gp->key_released[key] == true);
}

void poll_events(Game *gp, OS_EventList *event_list) {
  memory_zero(gp->key_released, sizeof(gp->key_released));

  for(OS_Event *event = event_list->first; event; event = event->next) {

    switch(event->kind) {
      case OS_EVENT_KEY_PRESS: {

        switch(event->key) {
          case OS_KEY_LEFT_CONTROL: case OS_KEY_RIGHT_CONTROL: {
            gp->modifier_mask |= OS_MOD_CONTROL;
          } break;
          case OS_KEY_LEFT_SHIFT: case OS_KEY_RIGHT_SHIFT: {
            gp->modifier_mask |= OS_MOD_SHIFT;
          } break;
          case OS_KEY_LEFT_ALT: case OS_KEY_RIGHT_ALT: {
            gp->modifier_mask |= OS_MOD_ALT;
          } break;
          default: {
            gp->key_pressed[event->key] += 1 + event->repeat_count;
          } break;
        }

      } break;
      case OS_EVENT_KEY_RELEASE: {

        switch(event->key) {
          case OS_KEY_LEFT_CONTROL: case OS_KEY_RIGHT_CONTROL: {
            gp->modifier_mask &= ~OS_MOD_CONTROL;
          } break;
          case OS_KEY_LEFT_SHIFT: case OS_KEY_RIGHT_SHIFT: {
            gp->modifier_mask &= ~OS_MOD_SHIFT;
          } break;
          case OS_KEY_LEFT_ALT: case OS_KEY_RIGHT_ALT: {
            gp->modifier_mask &= ~OS_MOD_ALT;
          } break;
          default: {
            gp->key_pressed[event->key] = 0;
            gp->key_released[event->key] = true;
          } break;
        }

      } break;
      case OS_EVENT_MOUSE_MOVE: {
        gp->mouse_pos = event->mouse_pos;
        gp->mouse_delta.x = event->mouse_pos.x - gp->mouse_pos.x;
        gp->mouse_delta.y = event->mouse_pos.y - gp->mouse_pos.y;
      } break;
    }
  }

  // TODO jfd: remove this
  event_list->count = 0;
  event_list->first = event_list->last = 0;
  arena_clear(app_event_arena);

}


// TODO jfd: move this to os_gfx
OS_Win32_WindowDimensions os_win32_get_window_dimensions(HWND window_handle) {
  RECT client_rect;
  GetClientRect(window_handle, &client_rect);

  OS_Win32_WindowDimensions window_dimensions = {
    .width = client_rect.right - client_rect.left,
    .height = client_rect.bottom - client_rect.top,
  };

  return window_dimensions;
}

// TODO jfd: move this to os_gfx
void os_win32_resize_backbuffer(OS_Win32_Backbuffer *backbuffer, int window_width, int window_height) {
  int width = window_width;
  int height = window_height;
  // TODO jfd: bulletproof this
  // maybe don't free first, free after, then free first if that fails

  // TODO jfd: free our DIBSection

  if(backbuffer->bitmap_memory) {
    os_free(backbuffer->bitmap_memory);
  }

  backbuffer->bitmap_width = width;
  backbuffer->bitmap_height = height;

  backbuffer->bitmap_info.bmiHeader.biSize = sizeof(backbuffer->bitmap_info.bmiHeader);
  backbuffer->bitmap_info.bmiHeader.biWidth = backbuffer->bitmap_width;
  backbuffer->bitmap_info.bmiHeader.biHeight = -backbuffer->bitmap_height;
  backbuffer->bitmap_info.bmiHeader.biPlanes = 1;
  backbuffer->bitmap_info.bmiHeader.biBitCount = 32;
  backbuffer->bitmap_info.bmiHeader.biCompression = BI_RGB; // pixel format

  u64 bitmap_memory_size = backbuffer->bitmap_width*backbuffer->bitmap_height * backbuffer->bytes_per_pixel;
  backbuffer->bitmap_memory = os_alloc(bitmap_memory_size);

  backbuffer->stride = backbuffer->bitmap_width * backbuffer->bytes_per_pixel;
}

// TODO jfd: move this to os_gfx
void os_win32_display_buffer_in_window(
  OS_Win32_Backbuffer *backbuffer,
  HDC device_context,
  int window_width,
  int window_height,
  int x,
  int y,
  int width,
  int height
) {

  StretchDIBits(
    device_context,
    0, 0, backbuffer->bitmap_width, backbuffer->bitmap_height,
    0, 0, window_width, window_height, // destination coordinates
    backbuffer->bitmap_memory,
    &backbuffer->bitmap_info,
    DIB_RGB_COLORS,
    SRCCOPY
  );

}

// TODO jfd: move this to os_gfx
LRESULT CALLBACK os_win32_main_window_callback(
  HWND window,
  UINT message,
  WPARAM w_param,
  LPARAM l_param
) {
  LRESULT result = 0;

  switch(message) {
    case WM_SIZE: {

      OS_Win32_WindowDimensions window_dimensions = os_win32_get_window_dimensions(window);

      os_win32_resize_backbuffer(&global_backbuffer, window_dimensions.width, window_dimensions.height);

      OutputDebugStringA("WM_SIZE\n");
    } break;
    case WM_DESTROY: {
      app_is_running = false;
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE: {
      app_is_running = false;
      OutputDebugStringA("WM_CLOSE\n");
    } break;
    case WM_ACTIVATEAPP: {
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
      // NOTE jfd: ryan fleury does this, but I have no idea why
      // if(w_param != VK_MENU && (w_param < VK_F1 || VK_F24 < w_param || w_param == VK_F4))
      // {
      //   result = DefWindowProc(window, message, w_param, l_param);
      // }
    } // fallthrough;
    case WM_KEYDOWN:
    case WM_KEYUP: {
      u32 virtual_keycode = (u32)w_param;

      OS_Key key = os_win32_key_from_virtual_keycode(virtual_keycode);

      #if 1 // debug
      if(message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
        if(key == OS_KEY_LEFT_ALT) {
          OutputDebugStringA("Pressed Alt\n");
        }
        if(key == OS_KEY_LEFT_SHIFT) {
          OutputDebugStringA("Pressed Shift\n");
        }
        if(key == OS_KEY_LEFT_CONTROL) {
          OutputDebugStringA("Pressed Control\n");
        }
        if(key == OS_KEY_W) {
          OutputDebugStringA("Pressed W\n");
        }
        if(key == OS_KEY_D) {
          OutputDebugStringA("Pressed D\n");
        }
        if(key == OS_KEY_A) {
          OutputDebugStringA("Pressed A\n");
        }
        if(key == OS_KEY_S) {
          OutputDebugStringA("Pressed S\n");
        }
      }
      if(message == WM_KEYUP || message == WM_SYSKEYUP) {
        if(key == OS_KEY_LEFT_ALT) {
          OutputDebugStringA("Released Alt\n");
        }
        if(key == OS_KEY_LEFT_SHIFT) {
          OutputDebugStringA("Released Shift\n");
        }
        if(key == OS_KEY_LEFT_CONTROL) {
          OutputDebugStringA("Released Control\n");
        }
        if(key == OS_KEY_W) {
          OutputDebugStringA("Released W\n");
        }
        if(key == OS_KEY_D) {
          OutputDebugStringA("Released D\n");
        }
        if(key == OS_KEY_A) {
          OutputDebugStringA("Released A\n");
        }
        if(key == OS_KEY_S) {
          OutputDebugStringA("Released S\n");
        }
      }
      #endif

      u32 key_repeat_count = (u32)l_param & 0x7fff;
      b32 was_down = ((u32)l_param & (1 << 30)) != 0;
      b32 is_down = ((u32)l_param & (1 << 31)) == 0;

      b32 release = 0;
      b32 is_repeat = 0;

      if(!is_down) {
        release = 1;
      } else if(was_down) {
        is_repeat = 1;
      }

      OS_Event *event = os_win32_event_push(app_event_arena, app_event_list, release ? OS_EVENT_KEY_RELEASE : OS_EVENT_KEY_PRESS);


      // TODO jfd: distinguish right and left modifiers with is_right_sided (???)

      event->key = key;
      event->is_repeat = is_repeat;
      event->repeat_count = key_repeat_count;

      if((event->key == OS_KEY_LEFT_ALT || event->key == OS_KEY_RIGHT_ALT) && event->modifier_mask & OS_MOD_ALT) {
        event->modifier_mask &= ~OS_MOD_ALT;
      }

      if((event->key == OS_KEY_LEFT_CONTROL || event->key == OS_KEY_RIGHT_CONTROL) && event->modifier_mask & OS_MOD_CONTROL) {
        event->modifier_mask &= ~OS_MOD_CONTROL;
      }

      if((event->key == OS_KEY_LEFT_SHIFT || event->key == OS_KEY_RIGHT_SHIFT) && event->modifier_mask & OS_MOD_SHIFT) {
        event->modifier_mask &= ~OS_MOD_SHIFT;
      }

      // TODO jfd: fill in the other fields

    } break;

    case WM_CHAR: {
    } break;

    case WM_MOUSEMOVE: {
      s16 x_pos = (s16)LOWORD(l_param);
      s16 y_pos = (s16)HIWORD(l_param);

      OS_Event *event = os_win32_event_push(app_event_arena, app_event_list, OS_EVENT_MOUSE_MOVE);
      event->mouse_pos.x = (f32)x_pos;
      event->mouse_pos.y = (f32)y_pos;

      #if 0
      arena_scope(scratch) {
        OutputDebugStringA(cstrf(scratch, "x: %i    y: %i\n", x_pos, y_pos));
      }
      #endif

    } break;

    case WM_MOUSEWHEEL: {
      s16 wheel_delta = HIWORD(w_param);
      OS_Event *event = os_win32_event_push(app_event_arena, app_event_list, OS_EVENT_MOUSE_SCROLL);
      POINT p;
      p.x = (s32)(s16)LOWORD(l_param);
      p.y = (s32)(s16)HIWORD(l_param);
      ScreenToClient(window, &p);
      event->mouse_pos.x = (f32)p.x;
      event->mouse_pos.y = (f32)p.y;
      event->scroll_delta.y = -(f32)wheel_delta;
    } break;

    case WM_PAINT: {
      PAINTSTRUCT paint;

      HDC device_context = BeginPaint(window, &paint);

      int x = paint.rcPaint.left;
      int y = paint.rcPaint.top;
      int width = paint.rcPaint.right - x;
      int height = paint.rcPaint.bottom - y;

      static int color = WHITENESS;
      if(color == WHITENESS) {
        color = BLACKNESS;
      } else {
        color = WHITENESS;
      }

      OS_Win32_WindowDimensions window_dimensions = os_win32_get_window_dimensions(window);
      os_win32_display_buffer_in_window(&global_backbuffer, device_context, window_dimensions.width, window_dimensions.height, x, y, width, height);
      EndPaint(window, &paint);
    } break;
    default: {
      result = DefWindowProc(window, message, w_param, l_param);
    } break;
  }

  return result;
}

void os_win32_load_xinput(void) {
  void *lib = os_library_load(scratch, str8_lit("Xinput1_4.dll"));
  if(lib) {
    os_win32_xinput_get_state = (OS_Win32_XInput_GetStateFunc*)os_library_load_func(scratch, lib, str8_lit("XInputGetState"));
    os_win32_xinput_set_state = (OS_Win32_XInput_SetStateFunc*)os_library_load_func(scratch, lib, str8_lit("XInputSetState"));
  }
}

void os_win32_init_dsound(HWND window_handle, s32 sound_buffer_size, s32 samples_per_second) {
  // NOTE jfd: load the library

  void *lib = os_library_load(scratch, str8_lit("dsound.dll"));

  if(lib) {
    // NOTE jfd: get a direct sound object
    OS_Win32_DirectSound_CreateFunc *direct_sound_create =
    (OS_Win32_DirectSound_CreateFunc*)os_library_load_func(scratch, lib, str8_lit("DirectSoundCreate8"));

    LPDIRECTSOUND direct_sound;

    if(direct_sound_create && SUCCEEDED(direct_sound_create(0, &direct_sound, 0))) {

      u16 n_channels = 2;
      u16 bits_per_sample = 16;
      u16 block_align = (n_channels * bits_per_sample) >> 3;

      WAVEFORMATEX wave_format = {0};
      wave_format.wFormatTag = WAVE_FORMAT_PCM; // play 1 or 2 channel PCM data
      wave_format.nChannels = n_channels;
      wave_format.nSamplesPerSec = samples_per_second;
      wave_format.nAvgBytesPerSec = samples_per_second * block_align;
      wave_format.nBlockAlign = block_align;
      wave_format.wBitsPerSample = bits_per_sample;
      wave_format.cbSize = 0;

      if(SUCCEEDED(direct_sound->lpVtbl->SetCooperativeLevel(direct_sound, window_handle, DSSCL_PRIORITY))) {

        DSBUFFERDESC buffer_description = {0};
        buffer_description.dwSize = sizeof(DSBUFFERDESC);
        buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;

        // NOTE jfd: create a primary buffer
        LPDIRECTSOUNDBUFFER primary_buffer;

        if(SUCCEEDED(direct_sound->lpVtbl->CreateSoundBuffer(direct_sound, &buffer_description, &primary_buffer, 0))) {

          HRESULT hr = primary_buffer->lpVtbl->SetFormat(primary_buffer, &wave_format);
          if(SUCCEEDED(hr)) {
            OutputDebugStringA("created primary buffer\n");
          } else {
            UNIMPLEMENTED;
          }

        } else {
          UNIMPLEMENTED;
        }

      } else {
        UNIMPLEMENTED;
      }

      // NOTE jfd: create secondary buffer for writing

      DSBUFFERDESC buffer_description = {0};
      buffer_description.dwSize = sizeof(DSBUFFERDESC);
      buffer_description.dwFlags = 0;
      buffer_description.dwBufferBytes = sound_buffer_size;
      buffer_description.lpwfxFormat = &wave_format;

      HRESULT hr = direct_sound->lpVtbl->CreateSoundBuffer(direct_sound, &buffer_description, &app_sound_buffer, 0);
      if(SUCCEEDED(hr)) {
        OutputDebugStringA("created secondary buffer\n");
      } else {
        UNREACHABLE;
      }

      // NOTE jfd: play sound

    } else {
      UNIMPLEMENTED;
    }

  }
}

void os_win32_fill_sound_buffer(OS_Win32_SoundOutput *sound_output, DWORD byte_to_lock_at, DWORD bytes_to_write) {

  void *region1;
  DWORD region1_size;
  void *region2;
  DWORD region2_size;

  if(SUCCEEDED(app_sound_buffer->lpVtbl->Lock(
    app_sound_buffer,
    byte_to_lock_at,
    bytes_to_write,
    &region1,
    &region1_size,
    &region2,
    &region2_size,
    0
  )))
  {

    s16 *sample_out1 = (s16*)region1;
    int region1_sample_count = region1_size / sound_output->bytes_per_sample;
    int region2_sample_count = region2_size / sound_output->bytes_per_sample;

    for(s32 sample_index = 0; sample_index < region1_sample_count; sample_index++) {
      f32 t = ((f32)sound_output->running_sample_index / (f32)sound_output->wave_period) * 2*PI;
      f32 sine_value = sinf(t);
      s16 sample_value = (s16)(sine_value * sound_output->tone_volume);

      *sample_out1++ = sample_value;
      *sample_out1++ = sample_value;
      ++sound_output->running_sample_index;
    }

    s16 *sample_out2 = (s16*)region2;
    for(s32 sample_index = 0; sample_index < region2_sample_count; sample_index++) {
      f32 t = ((f32)sound_output->running_sample_index / (f32)sound_output->wave_period) * 2*PI;
      f32 sine_value = sinf(t);
      s16 sample_value = (s16)(sine_value * sound_output->tone_volume);

      *sample_out2++ = sample_value;
      *sample_out2++ = sample_value;
      ++sound_output->running_sample_index;
    }

    app_sound_buffer->lpVtbl->Unlock(app_sound_buffer, region1, region1_size, region2, region2_size);

  } /* if(Lock()) */

}

int CALLBACK WinMain(
  HINSTANCE instance,
  HINSTANCE prev_instance,
  LPSTR cmd_line,
  int show_code
) {
  scratch = arena_create(MB(5));
  app_event_arena = arena_create(KB(50));

  OutputDebugStringA("cowabunga\n");

  // os_win32_load_xinput();

  WNDCLASSA window_class = {0};
  window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = os_win32_main_window_callback;
  window_class.hInstance = instance;
  window_class.lpszClassName = "Handmade Hero Window Class";

  if(RegisterClass(&window_class)) {
    HWND window_handle =
    CreateWindowExA(
      0,
      window_class.lpszClassName,
      "Handmade Hero",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      instance,
      0);

    if(window_handle) {

      ASSERT(SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED)));

      app_is_running = true;

      // NOTE jfd: graphics test
      MSG message;
      int x_offset = 0;
      int y_offset = 0;

      // NOTE jfd: sound test
      app_sound_output->samples_per_second    = THOUSAND(48);
      app_sound_output->buffer_size           = app_sound_output->samples_per_second * sizeof(s16) * 2;
      app_sound_output->tone_volume           = 500;
      app_sound_output->tone_hz               = 256; // about middle C
      app_sound_output->running_sample_index  = 0;
      app_sound_output->wave_period           = app_sound_output->samples_per_second / app_sound_output->tone_hz;
      app_sound_output->half_wave_period      = (app_sound_output->wave_period / 2);
      app_sound_output->bytes_per_sample      = sizeof(s16)*2;

      os_win32_init_dsound(window_handle, app_sound_output->buffer_size, app_sound_output->samples_per_second);

      app_sound_buffer->lpVtbl->Play(
        app_sound_buffer,
        0,
        0,
        DSBPLAY_LOOPING
      );

      while(app_is_running) {
        // NOTE jfd: get input messages
        while(PeekMessageA(&message, window_handle, 0, 0, PM_REMOVE)) {
          if(message.message == WM_QUIT) {
            app_is_running = false;
          }
          TranslateMessage(&message); // NOTE jfd: has to do with keyboard messages
          DispatchMessage(&message);

        }

        #if 0
        { /* get gamepad input */

          for(DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; controller_index++) {
            XINPUT_STATE controller_state;
            if(os_win32_xinput_get_state(controller_index, &controller_state) == ERROR_SUCCESS) {
              // NOTE jfd: this controller is plugged in
              XINPUT_GAMEPAD *gamepad = &controller_state.Gamepad;

              b8 dpad_up    = !!(gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
              b8 dpad_down  = !!(gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
              b8 dpad_left  = !!(gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
              b8 dpad_right = !!(gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

              b8 start_button = !!(gamepad->wButtons & XINPUT_GAMEPAD_START);
              b8 back_button  = !!(gamepad->wButtons & XINPUT_GAMEPAD_BACK);

              b8 left_thumb     = !!(gamepad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
              b8 right_thumb    = !!(gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
              b8 left_shoulder  = !!(gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
              b8 right_shoulder = !!(gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

              b8 a_button = !!(gamepad->wButtons & XINPUT_GAMEPAD_A);
              b8 b_button = !!(gamepad->wButtons & XINPUT_GAMEPAD_B);
              b8 x_button = !!(gamepad->wButtons & XINPUT_GAMEPAD_X);
              b8 y_button = !!(gamepad->wButtons & XINPUT_GAMEPAD_Y);

              s16 left_stick_x = gamepad->sThumbLX;
              s16 left_stick_y = gamepad->sThumbLY;
              s16 right_stick_x = gamepad->sThumbRX;
              s16 right_stick_y = gamepad->sThumbRY;

            } else {
              // NOTE jfd: this controller is not available
            }
          }

        } /* get gamepad input */
        #endif

        // TODO jfd: mouse movement and clicks
        { /* get keyboard and mouse input */

          poll_events(game_state, app_event_list);

          int step_pixels = 1;

          if(is_key_pressed(game_state, OS_KEY_W)) {
            y_offset -= step_pixels;
          }
          if(is_key_pressed(game_state, OS_KEY_A)) {
            x_offset -= step_pixels;
          }
          if(is_key_pressed(game_state, OS_KEY_S)) {
            y_offset += step_pixels;
          }
          if(is_key_pressed(game_state, OS_KEY_D)) {
            x_offset += step_pixels;
          }

        } /* get keyboard and mouse input */

        { /* draw */

          render_weird_gradient(&global_backbuffer, x_offset, y_offset);

        } /* draw */

        { /* test direct sound */

          DWORD sound_play_cursor;
          DWORD sound_write_cursor;

          if(SUCCEEDED(app_sound_buffer->lpVtbl->GetCurrentPosition(
            app_sound_buffer,
            &sound_play_cursor,
            &sound_write_cursor
          )))
          {

            DWORD byte_to_lock_at = (app_sound_output->running_sample_index * app_sound_output->bytes_per_sample) % app_sound_output->buffer_size;
            DWORD bytes_to_write = 0;

            // NOTE jfd: this check is innacurate
            if(byte_to_lock_at == sound_play_cursor) {
              bytes_to_write = app_sound_output->buffer_size;
            } else if(byte_to_lock_at > sound_play_cursor) {
              bytes_to_write = (app_sound_output->buffer_size - byte_to_lock_at);
              bytes_to_write += sound_play_cursor;
            } else {
              bytes_to_write = sound_play_cursor - byte_to_lock_at;
            }

            os_win32_fill_sound_buffer(app_sound_output, byte_to_lock_at, bytes_to_write);

          } /* if(GetCurrentPosision()) */

        } /* test direct sound */

        // NOTE jfd: blit to screen
        HDC device_context;
        defer_loop(device_context = GetDC(window_handle), ReleaseDC(window_handle, device_context)) {
          OS_Win32_WindowDimensions window_dimensions = os_win32_get_window_dimensions(window_handle);

          os_win32_display_buffer_in_window(&global_backbuffer, device_context, window_dimensions.width, window_dimensions.height, 0, 0, window_dimensions.width, window_dimensions.height);
        }

      }

    } else {
    }

  } else {
    // TODO jfd: logging
  }

  return 0;
}
