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


typedef struct PlatformWin32_WindowDimensions PlatformWin32_WindowDimensions;
struct PlatformWin32_WindowDimensions {
  s32 width;
  s32 height;
};

typedef struct PlatformWin32_Backbuffer PlatformWin32_Backbuffer;
struct PlatformWin32_Backbuffer {
  BITMAPINFO bitmap_info;
  u8 *bitmap_memory;
  s32 bitmap_height;
  s32 bitmap_width;
  u32 bytes_per_pixel;
  u32 stride;
};

typedef struct PlatformWin32_SoundOutput PlatformWin32_SoundOutput;
struct PlatformWin32_SoundOutput {
  int samples_per_second;
  DWORD buffer_size;
  u32 running_sample_index; // TODO jfd: should this be in bytes instead?
  int bytes_per_sample;
  int latency_sample_count;
  DWORD safety_bytes;
};

typedef struct PlatformWin32Debug_TimeMarker PlatformWin32Debug_TimeMarker;
struct PlatformWin32Debug_TimeMarker {
  DWORD output_play_cursor;
  DWORD output_write_cursor;
  DWORD output_location;
  DWORD output_byte_count;
  DWORD expected_flip_cursor;
  DWORD flip_play_cursor;
  DWORD flip_write_cursor;
};

TYPEDEF_SLICE(PlatformWin32Debug_TimeMarker, PlatformWin32Debug_TimeMarkerSlice);

internal LRESULT CALLBACK platform_win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

internal void platform_win32_resize_backbuffer(PlatformWin32_Backbuffer *backbuffer, int window_width, int window_height);

internal void platform_win32_display_buffer_in_window(PlatformWin32_Backbuffer *backbuffer, HDC device_context, int window_width, int window_height, int x, int y, int width, int height);

internal PlatformWin32_WindowDimensions platform_win32_get_window_dimensions(HWND window_handle);

internal void platform_win32_load_xinput(void);

#define PLATFORM_WIN32_XINPUT_SET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef PLATFORM_WIN32_XINPUT_SET_STATE(PlatformWin32_XInputSetStateFunc);

PLATFORM_WIN32_XINPUT_SET_STATE(_platform_win32_xinput_set_state_stub) {
  return 0;
}

#define PLATFORM_WIN32_XINPUT_GET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef PLATFORM_WIN32_XINPUT_GET_STATE(PlatformWin32_XInputGetStateFunc);

PLATFORM_WIN32_XINPUT_GET_STATE(_platform_win32_xinput_get_state_stub) {
  return 0;
}

#define PLATFORM_WIN32_DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
typedef PLATFORM_WIN32_DIRECT_SOUND_CREATE(PlatformWin32_DirectSoundCreateFunc);
PLATFORM_WIN32_DIRECT_SOUND_CREATE(_platform_win32_direct_sound_create_stub) {
  return 0;
}

internal Platform_Event* platform_win32_event_push(Arena *a, Platform_EventList *event_list, Platform_EventKind event_kind);
internal Platform_Event* platform_win32_event_pop(Platform_EventList *event_list);

internal KeyboardKey platform_win32_keyboard_key_from_virtual_keycode(WPARAM virtual_keycode);

internal void platform_get_game_input_from_events(Platform_EventList *event_list, Game *gp);

internal Str8 debug_platform_read_entire_file(Str8 path);
internal b32  debug_platform_write_entire_file(Str8 data, Str8 path);

internal void debug_platform_win32_sync_display(PlatformWin32_Backbuffer *backbuffer, PlatformWin32_SoundOutput *sound_output, PlatformWin32Debug_TimeMarkerSlice markers, f32 target_seconds_per_frame);

force_inline void platform_win32_sleep_ms(DWORD ms);

force_inline LARGE_INTEGER platform_win32_get_wall_clock(void);

force_inline f32 platform_win32_get_seconds_elapsed(LARGE_INTEGER begin, LARGE_INTEGER end);


#endif
