#ifndef WIN32_HANDMADE_H
#define WIN32_HANDMADE_H


// TODO jfd: Some of these things really should be moved in to a separate file that is platform independent
//           but is between the game and the platform cradle.

typedef enum OS_EventKind {
  OS_EVENT_NULL = 0,
  OS_EVENT_KEY_PRESS,
  OS_EVENT_KEY_RELEASE,
  OS_EVENT_MOUSE_MOVE,
  OS_EVENT_MOUSE_SCROLL,
} OS_EventKind;

typedef enum OS_ModifierIndex {
  OS_MOD_INDEX_NONE = -1,

  OS_MOD_INDEX_SHIFT,
  OS_MOD_INDEX_CONTROL,
  OS_MOD_INDEX_ALT,
  OS_MOD_INDEX_META,

  OS_MOD_INDEX_MAX,
} OS_Modifier_Index;

typedef u32 OS_Modifier;
const OS_Modifier OS_MOD_SHIFT   = (OS_Modifier)(1 << OS_MOD_INDEX_SHIFT);
const OS_Modifier OS_MOD_CONTROL = (OS_Modifier)(1 << OS_MOD_INDEX_CONTROL);
const OS_Modifier OS_MOD_ALT     = (OS_Modifier)(1 << OS_MOD_INDEX_ALT);
const OS_Modifier OS_MOD_META    = (OS_Modifier)(1 << OS_MOD_INDEX_META);

typedef enum OS_Key {
  OS_KEY_NULL = 0,

  OS_KEY_LEFT_MOUSE_BUTTON,
  OS_KEY_RIGHT_MOUSE_BUTTON,
  OS_KEY_MIDDLE_MOUSE_BUTTON,

  OS_KEY_LEFT_SHIFT,
  OS_KEY_RIGHT_SHIFT,
  OS_KEY_LEFT_CONTROL,
  OS_KEY_RIGHT_CONTROL,
  OS_KEY_LEFT_ALT,
  OS_KEY_RIGHT_ALT,
  OS_KEY_RIGHT_META,
  OS_KEY_LEFT_META,

  OS_KEY_ESCAPE,
  OS_KEY_ENTER,
  OS_KEY_SPACE,
  OS_KEY_TAB,
  OS_KEY_BACKSPACE,
  OS_KEY_DELETE,
  OS_KEY_CAPS_LOCK,
  OS_KEY_PRINT_SCREEN,
  OS_KEY_HOME,
  OS_KEY_END,
  OS_KEY_INSERT,
  OS_KEY_PAGE_DOWN,
  OS_KEY_PAGE_UP,
  OS_KEY_LEFT_ARROW,
  OS_KEY_RIGHT_ARROW,
  OS_KEY_UP_ARROW,
  OS_KEY_DOWN_ARROW,

  OS_KEY_F1,
  OS_KEY_F2,
  OS_KEY_F3,
  OS_KEY_F4,
  OS_KEY_F5,
  OS_KEY_F6,
  OS_KEY_F7,
  OS_KEY_F8,
  OS_KEY_F9,
  OS_KEY_F10,
  OS_KEY_F11,
  OS_KEY_F12,

  OS_KEY_0,
  OS_KEY_1,
  OS_KEY_2,
  OS_KEY_3,
  OS_KEY_4,
  OS_KEY_5,
  OS_KEY_6,
  OS_KEY_7,
  OS_KEY_8,
  OS_KEY_9,

  OS_KEY_A,
  OS_KEY_B,
  OS_KEY_C,
  OS_KEY_D,
  OS_KEY_E,
  OS_KEY_F,
  OS_KEY_G,
  OS_KEY_H,
  OS_KEY_I,
  OS_KEY_J,
  OS_KEY_K,
  OS_KEY_L,
  OS_KEY_M,
  OS_KEY_N,
  OS_KEY_O,
  OS_KEY_P,
  OS_KEY_Q,
  OS_KEY_R,
  OS_KEY_S,
  OS_KEY_T,
  OS_KEY_U,
  OS_KEY_V,
  OS_KEY_W,
  OS_KEY_X,
  OS_KEY_Y,
  OS_KEY_Z,

  OS_KEY_MAX,

} OS_Key;


typedef struct OS_Event OS_Event;
struct OS_Event {
  OS_Event *next;
  OS_Event *prev;
  OS_EventKind kind;
  OS_Modifier modifier_mask;
  OS_Key key;
  b32 is_repeat;
  u32 repeat_count;
  u32 character;
  Vec2 mouse_pos;
  Vec2 scroll_delta;
};

typedef struct OS_Input OS_Input;
struct OS_Input {
  OS_Modifier modifier_mask;
  u32 key_pressed[OS_KEY_MAX];
  b32 key_released[OS_KEY_MAX];
  Vec2 mouse_pos;
  Vec2 mouse_delta;
};

typedef struct OS_EventList OS_EventList;
struct OS_EventList {
  s64 count;
  OS_Event *first;
  OS_Event *last;
};

typedef struct OS_Win32_WindowDimensions OS_Win32_WindowDimensions;
struct OS_Win32_WindowDimensions {
  s32 width;
  s32 height;
};

typedef struct OS_Win32_Backbuffer OS_Win32_Backbuffer;
struct OS_Win32_Backbuffer {
  BITMAPINFO bitmap_info;
  u8 *bitmap_memory;
  s32 bitmap_height;
  s32 bitmap_width;
  u32 bytes_per_pixel;
  u32 stride;
};

typedef struct OS_Win32_SoundOutput OS_Win32_SoundOutput;
struct OS_Win32_SoundOutput {
  int samples_per_second;
  int buffer_size;
  u32 running_sample_index;
  int bytes_per_sample;
  int latency_sample_count;
};


OS_Modifier os_get_modifiers(void);
b32 os_is_modifier_key(OS_Key key);

OS_Event* os_win32_event_push(Arena *a, OS_EventList *event_list, OS_EventKind event_kind);
OS_Event* os_win32_event_pop(OS_EventList *event_list);


LRESULT CALLBACK os_win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

void os_win32_resize_backbuffer(OS_Win32_Backbuffer *backbuffer, int window_width, int window_height);

void os_win32_display_buffer_in_window(OS_Win32_Backbuffer *backbuffer, HDC device_context, int window_width, int window_height, int x, int y, int width, int height);

OS_Win32_WindowDimensions os_win32_get_window_dimensions(HWND window_handle);

OS_Key os_win32_key_from_virtual_keycode(WPARAM virtual_keycode);

void os_win32_poll_input_events(OS_Input *input, OS_EventList *event_list);

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


#endif
