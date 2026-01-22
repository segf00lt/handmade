#ifndef PLATFORM_WIN32_H
#define PLATFORM_WIN32_H


// NOTE jfd 22/01/2026:
// This file contains the types and function headers for platform specific code.
// DO NOT CALL THESE FROM GAME OR APP CODE

#include <windows.h>
#include <Xinput.h>
#include <Dsound.h>
#include <intrin.h>
#include <objbase.h>


typedef struct PLTFM_WIN32_WindowDimensions PLTFM_WIN32_WindowDimensions;
struct PLTFM_WIN32_WindowDimensions {
  s32 width;
  s32 height;
};

typedef struct PLTFM_WIN32_Backbuffer PLTFM_WIN32_Backbuffer;
struct PLTFM_WIN32_Backbuffer {
  BITMAPINFO bitmap_info;
  u8 *bitmap_memory;
  s32 bitmap_height;
  s32 bitmap_width;
  u32 bytes_per_pixel;
  u32 stride;
};

typedef struct PLTFM_WIN32_SoundOutput PLTFM_WIN32_SoundOutput;
struct PLTFM_WIN32_SoundOutput {
  int samples_per_second;
  int buffer_size;
  u32 running_sample_index;
  int bytes_per_sample;
  int latency_sample_count;
};


internal LRESULT CALLBACK platform_win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

internal void platform_win32_resize_backbuffer(PLTFM_WIN32_Backbuffer *backbuffer, int window_width, int window_height);

internal void platform_win32_display_buffer_in_window(PLTFM_WIN32_Backbuffer *backbuffer, HDC device_context, int window_width, int window_height, int x, int y, int width, int height);

internal PLTFM_WIN32_WindowDimensions platform_win32_get_window_dimensions(HWND window_handle);

internal void platform_win32_load_xinput(void);

#define PLTFM_WIN32_XINPUT_SET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef PLTFM_WIN32_XINPUT_SET_STATE(PLTFM_WIN32_XInputSetStateFunc);

PLTFM_WIN32_XINPUT_SET_STATE(_platform_win32_xinput_set_state_stub) {
  return 0;
}

#define PLTFM_WIN32_XINPUT_GET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef PLTFM_WIN32_XINPUT_GET_STATE(PLTFM_WIN32_XInputGetStateFunc);

PLTFM_WIN32_XINPUT_GET_STATE(_platform_win32_xinput_get_state_stub) {
  return 0;
}

#define PLTFM_WIN32_DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
typedef PLTFM_WIN32_DIRECT_SOUND_CREATE(PLTFM_WIN32_DirectSoundCreateFunc);
PLTFM_WIN32_DIRECT_SOUND_CREATE(_platform_win32_direct_sound_create_stub) {
  return 0;
}

internal PLTFM_Event* platform_win32_event_push(Arena *a, PLTFM_EventList *event_list, PLTFM_EventKind event_kind);
internal PLTFM_Event* platform_win32_event_pop(PLTFM_EventList *event_list);

internal KeyboardKey platform_win32_keyboard_key_from_virtual_keycode(WPARAM virtual_keycode);

internal void platform_get_game_input_from_events(PLTFM_EventList *event_list, Game *gp);

internal Str8 DEBUG_platform_read_entire_file(Str8 path);
internal b32  DEBUG_platform_write_entire_file(Str8 data, Str8 path);


#endif
