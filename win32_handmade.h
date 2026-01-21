#ifndef WIN32_HANDMADE_H
#define WIN32_HANDMADE_H


// TODO jfd: Some of these things really should be moved in to a separate file that is platform independent
//           but is between the game and the platform cradle.

typedef struct HMH_Win32_WindowDimensions HMH_Win32_WindowDimensions;
struct HMH_Win32_WindowDimensions {
  s32 width;
  s32 height;
};

typedef struct HMH_Win32_Backbuffer HMH_Win32_Backbuffer;
struct HMH_Win32_Backbuffer {
  BITMAPINFO bitmap_info;
  u8 *bitmap_memory;
  s32 bitmap_height;
  s32 bitmap_width;
  u32 bytes_per_pixel;
  u32 stride;
};

typedef struct HMH_Win32_SoundOutput HMH_Win32_SoundOutput;
struct HMH_Win32_SoundOutput {
  int samples_per_second;
  int buffer_size;
  u32 running_sample_index;
  int bytes_per_sample;
  int latency_sample_count;
};


LRESULT CALLBACK hmh_win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

void hmh_win32_resize_backbuffer(HMH_Win32_Backbuffer *backbuffer, int window_width, int window_height);

void hmh_win32_display_buffer_in_window(HMH_Win32_Backbuffer *backbuffer, HDC device_context, int window_width, int window_height, int x, int y, int width, int height);

HMH_Win32_WindowDimensions hmh_win32_get_window_dimensions(HWND window_handle);

void hmh_win32_load_xinput(void);

#define HMH_WIN32_XINPUT_SET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef HMH_WIN32_XINPUT_SET_STATE(HMH_Win32_XInput_SetStateFunc);

HMH_WIN32_XINPUT_SET_STATE(_hmh_win32_xinput_set_state_stub) {
  return 0;
}

#define HMH_WIN32_XINPUT_GET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef HMH_WIN32_XINPUT_GET_STATE(HMH_Win32_XInput_GetStateFunc);

HMH_WIN32_XINPUT_GET_STATE(_hmh_win32_xinput_get_state_stub) {
  return 0;
}

#define HMH_WIN32_DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter)
typedef HMH_WIN32_DIRECT_SOUND_CREATE(HMH_Win32_DirectSound_CreateFunc);
HMH_WIN32_DIRECT_SOUND_CREATE(_hmh_win32_direct_sound_create_stub) {
  return 0;
}


#endif
