#include "base.h"


#include "handmade.h"
#include "handmade.c"


#include <Xinput.h>
#include <Dsound.h>
#include <intrin.h>

#include "win32_handmade.h"



/*
 * globals
 */

global Arena *scratch;
global Arena *frame_arena;
global Arena *hmh_event_arena;

global HMH_Win32_XInput_GetStateFunc *hmh_win32_xinput_get_state = _hmh_win32_xinput_get_state_stub;
global HMH_Win32_XInput_SetStateFunc *hmh_win32_xinput_set_state = _hmh_win32_xinput_set_state_stub;

global b32 hmh_is_running;
global HMH_Win32_Backbuffer global_backbuffer = {
  .bytes_per_pixel = 4,
};

global HMH_Win32_SoundOutput _hmh_sound_output_stub;
global HMH_Win32_SoundOutput *hmh_sound_output = &_hmh_sound_output_stub;

global LPDIRECTSOUNDBUFFER hmh_sound_buffer; // this is what we write to

global Game *game_state;
global GFX_EventList _hmh_event_list_stub;
global GFX_EventList *hmh_event_list = &_hmh_event_list_stub;


/*
 * functions
 */


HMH_Win32_WindowDimensions hmh_win32_get_window_dimensions(HWND window_handle) {
  RECT client_rect;
  GetClientRect(window_handle, &client_rect);

  HMH_Win32_WindowDimensions window_dimensions = {
    .width = client_rect.right - client_rect.left,
    .height = client_rect.bottom - client_rect.top,
  };

  return window_dimensions;
}

void hmh_win32_resize_backbuffer(HMH_Win32_Backbuffer *backbuffer, int window_width, int window_height) {
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

void hmh_win32_display_buffer_in_window(HMH_Win32_Backbuffer *backbuffer, HDC device_context, int window_width, int window_height, int x, int y, int width, int height ) {

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

// TODO jfd: Eventually we'll move this in to the gfx layer
LRESULT CALLBACK hmh_win32_main_window_callback(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;

  switch(message) {
    case WM_SIZE: {

      HMH_Win32_WindowDimensions window_dimensions = hmh_win32_get_window_dimensions(window);

      hmh_win32_resize_backbuffer(&global_backbuffer, window_dimensions.width, window_dimensions.height);
    } break;
    case WM_DESTROY: {
      hmh_is_running = false;
    } break;
    case WM_CLOSE: {
      hmh_is_running = false;
    } break;
    case WM_ACTIVATEAPP: {
    } break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    {
      // NOTE jfd: ryan fleury does this, but I have no idea why
      // if(wParam != VK_MENU && (wParam < VK_F1 || VK_F24 < wParam || wParam == VK_F4))
      // {
      //   result = DefWindowProc(window, message, wParam, lParam);
      // }
    } // fallthrough;
    case WM_KEYDOWN:
    case WM_KEYUP: {
      u32 virtual_keycode = (u32)wParam;

      GFX_Key key = gfx_win32_key_from_virtual_keycode(virtual_keycode);

      u32 key_repeat_count = (u32)lParam & 0x7fff;
      b32 was_down = ((u32)lParam & (1 << 30)) != 0;
      b32 is_down = ((u32)lParam & (1 << 31)) == 0;

      b32 release = 0;
      b32 is_repeat = 0;

      if(!is_down) {
        release = 1;
      } else if(was_down) {
        is_repeat = 1;
      }

      GFX_Event *event = gfx_win32_event_push(hmh_event_arena, hmh_event_list, release ? GFX_EVENT_KEY_RELEASE : GFX_EVENT_KEY_PRESS);

      // TODO jfd: distinguish right and left modifiers with is_right_sided (???)

      event->key = key;
      event->is_repeat = is_repeat;
      event->repeat_count = key_repeat_count;

      if((event->key == GFX_KEY_LEFT_ALT || event->key == GFX_KEY_RIGHT_ALT) && event->modifier_mask & GFX_MOD_ALT) {
        event->modifier_mask &= ~GFX_MOD_ALT;
      }

      if((event->key == GFX_KEY_LEFT_CONTROL || event->key == GFX_KEY_RIGHT_CONTROL) && event->modifier_mask & GFX_MOD_CONTROL) {
        event->modifier_mask &= ~GFX_MOD_CONTROL;
      }

      if((event->key == GFX_KEY_LEFT_SHIFT || event->key == GFX_KEY_RIGHT_SHIFT) && event->modifier_mask & GFX_MOD_SHIFT) {
        event->modifier_mask &= ~GFX_MOD_SHIFT;
      }

      // TODO jfd: fill in the other fields

    } break;

    case WM_CHAR: {
    } break;

    case WM_MOUSEMOVE: {
      s16 x_pos = (s16)LOWORD(lParam);
      s16 y_pos = (s16)HIWORD(lParam);

      GFX_Event *event = gfx_win32_event_push(hmh_event_arena, hmh_event_list, GFX_EVENT_MOUSE_MOVE);
      event->mouse_pos.x = (f32)x_pos;
      event->mouse_pos.y = (f32)y_pos;

    } break;

    case WM_MOUSEWHEEL: {
      s16 wheel_delta = HIWORD(wParam);
      GFX_Event *event = gfx_win32_event_push(hmh_event_arena, hmh_event_list, GFX_EVENT_MOUSE_SCROLL);
      POINT p;
      p.x = (s32)(s16)LOWORD(lParam);
      p.y = (s32)(s16)HIWORD(lParam);
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

      HMH_Win32_WindowDimensions window_dimensions = hmh_win32_get_window_dimensions(window);
      hmh_win32_display_buffer_in_window(&global_backbuffer, device_context, window_dimensions.width, window_dimensions.height, x, y, width, height);
      EndPaint(window, &paint);
    } break;
    default: {
      result = DefWindowProc(window, message, wParam, lParam);
    } break;
  }

  return result;
}

void hmh_win32_load_xinput(void) {
  void *lib = os_library_load(scratch, str8_lit("Xinput1_4.dll"));
  if(lib) {
    hmh_win32_xinput_get_state = (HMH_Win32_XInput_GetStateFunc*)os_library_load_func(scratch, lib, str8_lit("XInputGetState"));
    hmh_win32_xinput_set_state = (HMH_Win32_XInput_SetStateFunc*)os_library_load_func(scratch, lib, str8_lit("XInputSetState"));
  }
}

void hmh_win32_init_dsound(HWND window_handle, s32 sound_buffer_size, s32 samples_per_second) {
  // NOTE jfd: load the library

  void *lib = os_library_load(scratch, str8_lit("dsound.dll"));

  if(lib) {
    // NOTE jfd: get a direct sound object
    HMH_Win32_DirectSound_CreateFunc *direct_sound_create =
    (HMH_Win32_DirectSound_CreateFunc*)os_library_load_func(scratch, lib, str8_lit("DirectSoundCreate8"));

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

      HRESULT hr = direct_sound->lpVtbl->CreateSoundBuffer(direct_sound, &buffer_description, &hmh_sound_buffer, 0);
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

void hmh_win32_clear_sound_buffer(HMH_Win32_SoundOutput *sound_output) {
  void *region1;
  DWORD region1_size;
  void *region2;
  DWORD region2_size;

  if(SUCCEEDED(hmh_sound_buffer->lpVtbl->Lock(
    hmh_sound_buffer,
    0,
    sound_output->buffer_size,
    &region1,
    &region1_size,
    &region2,
    &region2_size,
    0
  )))
  {
    memory_zero(region1, region1_size);
    memory_zero(region2, region2_size);

    hmh_sound_buffer->lpVtbl->Unlock(hmh_sound_buffer, region1, region1_size, region2, region2_size);
  }
}

void hmh_win32_fill_sound_buffer(HMH_Win32_SoundOutput *sound_output, DWORD byte_to_lock_at, DWORD bytes_to_write, Game_SoundBuffer *source_buffer) {

  void *region1;
  DWORD region1_size;
  void *region2;
  DWORD region2_size;

  if(SUCCEEDED(hmh_sound_buffer->lpVtbl->Lock(
    hmh_sound_buffer,
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

    s16 *source_sample = source_buffer->samples;

    for(s32 sample_index = 0; sample_index < region1_sample_count; sample_index++) {
      *sample_out1++ = *source_sample++;
      *sample_out1++ = *source_sample++;
      ++sound_output->running_sample_index;
    }

    s16 *sample_out2 = (s16*)region2;
    for(s32 sample_index = 0; sample_index < region2_sample_count; sample_index++) {
      *sample_out2++ = *source_sample++;
      *sample_out2++ = *source_sample++;
      ++sound_output->running_sample_index;
    }

    hmh_sound_buffer->lpVtbl->Unlock(hmh_sound_buffer, region1, region1_size, region2, region2_size);

  } /* if(Lock()) */

}




int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCode) {

  LARGE_INTEGER performance_counter_frequency;
  QueryPerformanceFrequency(&performance_counter_frequency);

  scratch         = arena_create(MB(5));
  frame_arena     = arena_create(KB(5));
  hmh_event_arena = arena_create(KB(50));

  // gfx_win32_load_xinput();

  WNDCLASSA window_class = {0};
  window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = hmh_win32_main_window_callback;
  window_class.hInstance = instance;
  window_class.lpszClassName = "Handmade Hero Window Class";

  { /* game_init */
    game_state = os_alloc(GAME_STATE_SIZE);

    game_state->main_arena = arena_create(MB(5));
    game_state->frame_arena = arena_create(MB(1));
    game_state->temp_arena = arena_create(KB(5));

  } /* game_init */

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

      // NOTE jfd: graphics test
      MSG message;

      // NOTE jfd: sound test
      hmh_sound_output->samples_per_second    = THOUSAND(48);
      hmh_sound_output->buffer_size           = hmh_sound_output->samples_per_second * sizeof(s16) * 2;
      hmh_sound_output->running_sample_index  = 0;
      hmh_sound_output->bytes_per_sample      = sizeof(s16)*2;
      hmh_sound_output->latency_sample_count  = hmh_sound_output->samples_per_second / 15;

      hmh_win32_init_dsound(window_handle, hmh_sound_output->buffer_size, hmh_sound_output->samples_per_second);
      hmh_win32_clear_sound_buffer(hmh_sound_output);
      hmh_sound_buffer->lpVtbl->Play(hmh_sound_buffer, 0, 0, DSBPLAY_LOOPING);

      s16 *samples = (s16*)push_array_no_zero(scratch, u8, hmh_sound_output->buffer_size);

      hmh_is_running = true;

      LARGE_INTEGER last_counter;
      QueryPerformanceCounter(&last_counter);

      f32 frame_time = 0;

      u64 last_cycle_count = __rdtsc(); // NOTE jfd: get timestamp in cycles

      while(hmh_is_running) {

        game_state->t = frame_time;

        // NOTE jfd: get input messages
        while(PeekMessageA(&message, window_handle, 0, 0, PM_REMOVE)) {
          if(message.message == WM_QUIT) {
            hmh_is_running = false;
          }
          TranslateMessage(&message); // NOTE jfd: has to do with keyboard messages
          DispatchMessage(&message);

        }

        // TODO jfd: Implement gamepad input
        #if 0
        { /* get gamepad input */

          for(DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT; controller_index++) {
            XINPUT_STATE controller_state;
            if(gfx_win32_xinput_get_state(controller_index, &controller_state) == ERROR_SUCCESS) {
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


        gfx_poll_input_events(hmh_event_list, &game_state->input);
        arena_clear(hmh_event_arena);


        DWORD byte_to_lock_at;
        DWORD bytes_to_write;
        DWORD sound_play_cursor;
        DWORD sound_target_cursor;
        DWORD sound_write_cursor;
        b32 sound_is_valid = false;

        // TODO jfd: Tighten up sound logic so that we know where we should be writing to and can anticipate the time
        //           spent in game_update_and_render()
        if(SUCCEEDED(hmh_sound_buffer->lpVtbl->GetCurrentPosition(hmh_sound_buffer, &sound_play_cursor, &sound_write_cursor))) {

          byte_to_lock_at =
          (hmh_sound_output->running_sample_index * hmh_sound_output->bytes_per_sample) % hmh_sound_output->buffer_size;

          sound_target_cursor =
          (sound_play_cursor + (hmh_sound_output->latency_sample_count * hmh_sound_output->bytes_per_sample)) % hmh_sound_output->buffer_size;

          if(byte_to_lock_at > sound_target_cursor) {
            bytes_to_write = (hmh_sound_output->buffer_size - byte_to_lock_at);
            bytes_to_write += sound_target_cursor;
          } else {
            bytes_to_write = sound_target_cursor - byte_to_lock_at;
          }

          sound_is_valid = true;

        } else {
          byte_to_lock_at = 0;
          bytes_to_write = 0;
        }

        // TODO jfd: move to 60 fps
        game_state->sound = (Game_SoundBuffer){0};
        game_state->sound.samples_per_second = hmh_sound_output->samples_per_second;
        game_state->sound.sample_count = bytes_to_write / hmh_sound_output->bytes_per_sample;
        game_state->sound.samples = samples;

        game_state->render.pixels = global_backbuffer.bitmap_memory;
        game_state->render.width  = global_backbuffer.bitmap_width;
        game_state->render.height = global_backbuffer.bitmap_height;
        game_state->render.stride = global_backbuffer.stride;

        game_update_and_render(game_state);

        if(sound_is_valid) {
          hmh_win32_fill_sound_buffer(hmh_sound_output, byte_to_lock_at, bytes_to_write, &game_state->sound);
        }

        // NOTE jfd: blit to screen
        HDC device_context;
        defer_loop(device_context = GetDC(window_handle), ReleaseDC(window_handle, device_context)) {
          HMH_Win32_WindowDimensions window_dimensions = hmh_win32_get_window_dimensions(window_handle);

          hmh_win32_display_buffer_in_window(&global_backbuffer, device_context, window_dimensions.width, window_dimensions.height, 0, 0, window_dimensions.width, window_dimensions.height);
        }

        s64 end_cycle_count = __rdtsc();

        s64 elapsed_cycles = end_cycle_count - last_cycle_count;

        last_cycle_count = end_cycle_count;

        LARGE_INTEGER end_counter;
        QueryPerformanceCounter(&end_counter);

        f64 elapsed_time = (f64)(end_counter.QuadPart - last_counter.QuadPart);
        f64 frame_time_ms = THOUSAND(elapsed_time) / (f64)performance_counter_frequency.QuadPart;

        frame_time = (f32)frame_time_ms * 1.0e-3f;

        f64 fps = (f64)performance_counter_frequency.QuadPart / elapsed_time;

        last_counter = end_counter;

        OutputDebugStringA(
          cstrf(frame_arena,
            "frame time (ms):  %g    fps:  %g    mega cycles: %d\n",
            frame_time_ms, fps, elapsed_cycles / MILLION(1))
        );

        arena_clear(frame_arena);

      }

    } else {
    }

  } else {
    // TODO jfd: logging
  }

  return 0;
}
