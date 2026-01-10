#include "base.h"

/*
 * tables
 */

/*
 * types
 */

typedef struct OS_Win32_Backbuffer OS_Win32_Backbuffer;
typedef struct OS_Win32_Window_Dimensions OS_Win32_Window_Dimensions;

typedef struct OS_Event_List OS_Event_List;
typedef struct OS_Event OS_Event;

typedef struct Game Game;

typedef enum OS_Event_Kind {
  OS_EVENT_NULL = 0,
  OS_EVENT_KEY_PRESS,
  OS_EVENT_KEY_RELEASE,
  OS_EVENT_MOUSE_MOVE,
} OS_Event_Kind;

typedef enum OS_Modifier_Index {
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

/*
 * structs
 */

struct OS_Win32_Backbuffer {
  BITMAPINFO bitmap_info;
  u8 *bitmap_memory;
  s32 bitmap_width;
  s32 bitmap_height;
  u32 bytes_per_pixel;
  u32 stride;
};

struct OS_Win32_Window_Dimensions {
  s32 width;
  s32 height;
};

struct OS_Event_List {
  s64 count;
  OS_Event *first;
  OS_Event *last;
};

struct OS_Event {
  OS_Event *next;
  OS_Event *prev;
  OS_Event_Kind kind;
  OS_Modifier modifier_mask;
  OS_Key key;
  b32 is_repeat;
  u32 repeat_count;
  u32 character;
  Vec2 mouse_pos;
  Vec2 mouse_delta;
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

OS_Win32_Window_Dimensions os_win32_get_window_dimensions(HWND window_handle);


OS_Modifier os_get_modifiers(void);

void poll_events(Game *gp, OS_Event_List *event_list);
b32  is_key_pressed(Game *gp, OS_Key key);
b32  was_key_pressed_once(Game *gp, OS_Key key);
b32  was_key_released(Game *gp, OS_Key key);

void os_win32_load_xinput(void);

OS_Key os_win32_key_from_virtual_keycode(WPARAM virtual_keycode);
b32 os_is_modifier_key(OS_Key key);

#define OS_WIN32_XINPUT_SET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef OS_WIN32_XINPUT_SET_STATE(OS_Win32_XInput_Set_State_Func);

OS_WIN32_XINPUT_SET_STATE(_os_win32_xinput_set_state_stub) {
  return 0;
}

#define OS_WIN32_XINPUT_GET_STATE(name) DWORD name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef OS_WIN32_XINPUT_GET_STATE(OS_Win32_XInput_Get_State_Func);

OS_WIN32_XINPUT_GET_STATE(_os_win32_xinput_get_state_stub) {
  return 0;
}

OS_Event* os_event_push(Arena *a, OS_Event_List *event_list, OS_Event_Kind event_kind);
OS_Event* os_event_pop(OS_Event_List *event_list);

/*
 * globals
 */

global Arena *scratch;
global Arena *app_event_arena;

global OS_Win32_XInput_Get_State_Func *os_win32_xinput_get_state = _os_win32_xinput_get_state_stub;
global OS_Win32_XInput_Set_State_Func *os_win32_xinput_set_state = _os_win32_xinput_set_state_stub;

global b32 app_is_running;
global OS_Win32_Backbuffer global_backbuffer = {
  .bytes_per_pixel = 4,
};

global Game _game_state_stub;
global Game *game_state = &_game_state_stub;
global OS_Event_List _app_event_list_stub;
global OS_Event_List *app_event_list = &_app_event_list_stub;

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


// TODO jfd: move these generic os functions to a os_gfx
OS_Event* os_event_push(Arena *a, OS_Event_List *event_list, OS_Event_Kind event_kind) {
  OS_Event *event = push_struct_no_zero(a, OS_Event);
  event->kind = event_kind;
  event->modifier_mask = os_get_modifiers();
  sll_queue_push(event_list->first, event_list->last, event);
  event_list->count++;
  return event;
}

OS_Event* os_event_pop(OS_Event_List *event_list) {
  OS_Event *event = event_list->last;
  sll_queue_pop(event_list->first, event_list->last);
  event_list->count--;
  return event;
}

b32 os_is_modifier_key(OS_Key key) {
  b32 result = (key >= OS_KEY_LEFT_SHIFT && key <= OS_KEY_LEFT_META);
  return result;
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

void poll_events(Game *gp, OS_Event_List *event_list) {
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
        UNIMPLEMENTED;
      } break;
    }
  }

  // TODO jfd: remove this
  event_list->count = 0;
  event_list->first = event_list->last = 0;
  arena_clear(app_event_arena);

}

OS_Modifier os_get_modifiers(void) {
  OS_Modifier modifier_mask = 0;

  if(GetKeyState(VK_CONTROL) & 0x8000) {
    modifier_mask |= OS_MOD_CONTROL;
  }

  if(GetKeyState(VK_SHIFT) & 0x8000) {
    modifier_mask |= OS_MOD_SHIFT;
  }

  if(GetKeyState(VK_MENU) & 0x8000) {
    modifier_mask |= OS_MOD_ALT;
  }

  if((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000)) {
    modifier_mask |= OS_MOD_META;
  }

  return modifier_mask;
}


OS_Win32_Window_Dimensions os_win32_get_window_dimensions(HWND window_handle) {
  RECT client_rect;
  GetClientRect(window_handle, &client_rect);

  OS_Win32_Window_Dimensions window_dimensions = {
    .width = client_rect.right - client_rect.left,
    .height = client_rect.bottom - client_rect.top,
  };

  return window_dimensions;
}

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

OS_Key os_win32_key_from_virtual_keycode(WPARAM virtual_keycode) {
  OS_Key key = 0;

  switch(virtual_keycode) {

    case VK_LBUTTON /* 0x01	Left mouse button */:
    key = OS_KEY_LEFT_MOUSE_BUTTON;
    break;
    case VK_RBUTTON /* 0x02	Right mouse button */:
    key = OS_KEY_RIGHT_MOUSE_BUTTON;
    break;
    case VK_MBUTTON /* 0x04	Middle mouse button */:
    key = OS_KEY_MIDDLE_MOUSE_BUTTON;
    break;

    case VK_CANCEL /* 0x03	Control-break processing */:
    break;
    case VK_XBUTTON1 /* 0x05	X1 mouse button */:
    break;
    case VK_XBUTTON2 /* 0x06	X2 mouse button */:
    break;
    // 0x07	Reserved
    case VK_BACK /* 0x08	Backspace key */:
    key = OS_KEY_BACKSPACE;
    break;
    case VK_TAB /* 0x09	Tab key */:
    key = OS_KEY_TAB;
    break;
    // 0x0A-0B	Reserved
    case VK_CLEAR /* 0x0C	Clear key */:
    break;
    case VK_RETURN /* 0x0D	Enter key */:
    key = OS_KEY_ENTER;
    break;
    // 0x0E-0F	Unassigned
    case VK_SHIFT /* 0x10	Shift key */:
    key = OS_KEY_LEFT_SHIFT;
    break;
    case VK_CONTROL /* 0x11	Ctrl key */:
    key = OS_KEY_LEFT_CONTROL;
    break;
    case VK_MENU /* 0x12	Alt key */:
    key = OS_KEY_LEFT_ALT;
    break;
    case VK_PAUSE /* 0x13	Pause key */:
    break;
    case VK_CAPITAL /* 0x14	Caps lock key */:
    key = OS_KEY_CAPS_LOCK;
    break;
    case VK_ESCAPE /* 0x1B	Esc key */:
    key = OS_KEY_ESCAPE;
    break;
    case VK_SPACE /* 0x20	Spacebar key */:
    key = OS_KEY_SPACE;
    break;
    case VK_PRIOR /* 0x21	Page up key */:
    key = OS_KEY_PAGE_UP;
    break;
    case VK_NEXT /* 0x22	Page down key */:
    key = OS_KEY_PAGE_DOWN;
    break;
    case VK_END /* 0x23	End key */:
    key = OS_KEY_END;
    break;
    case VK_HOME /* 0x24	Home key */:
    key = OS_KEY_HOME;
    break;
    case VK_LEFT /* 0x25	Left arrow key */:
    key = OS_KEY_LEFT_ARROW;
    break;
    case VK_UP /* 0x26	Up arrow key */:
    key = OS_KEY_UP_ARROW;
    break;
    case VK_RIGHT /* 0x27	Right arrow key */:
    key = OS_KEY_RIGHT_ARROW;
    break;
    case VK_DOWN /* 0x28	Down arrow key */:
    key = OS_KEY_DOWN_ARROW;
    break;
    case VK_SNAPSHOT /* 0x2C	Print screen key */:
    key = OS_KEY_PRINT_SCREEN;
    break;
    case VK_INSERT /* 0x2D	Insert key */:
    key = OS_KEY_INSERT;
    break;
    case VK_DELETE /* 0x2E	Delete key */:
    key = OS_KEY_DELETE;
    break;
    case 0x30 /* 0 key */:
    key = OS_KEY_0;
    break;
    case 0x31 /* 1 key */:
    key = OS_KEY_1;
    break;
    case 0x32 /* 2 key */:
    key = OS_KEY_2;
    break;
    case 0x33 /* 3 key */:
    key = OS_KEY_3;
    break;
    case 0x34 /* 4 key */:
    key = OS_KEY_4;
    break;
    case 0x35 /* 5 key */:
    key = OS_KEY_5;
    break;
    case 0x36 /* 6 key */:
    key = OS_KEY_6;
    break;
    case 0x37 /* 7 key */:
    key = OS_KEY_7;
    break;
    case 0x38 /* 8 key */:
    key = OS_KEY_8;
    break;
    case 0x39 /* 9 key */:
    key = OS_KEY_9;
    break;
    // 0x3A-40	Undefined
    case 0x41 /* A key */:
    key = OS_KEY_A;
    break;
    case 0x42 /* B key */:
    key = OS_KEY_B;
    break;
    case 0x43 /* C key */:
    key = OS_KEY_C;
    break;
    case 0x44 /* D key */:
    key = OS_KEY_D;
    break;
    case 0x45 /* E key */:
    key = OS_KEY_E;
    break;
    case 0x46 /* F key */:
    key = OS_KEY_F;
    break;
    case 0x47 /* G key */:
    key = OS_KEY_G;
    break;
    case 0x48 /* H key */:
    key = OS_KEY_H;
    break;
    case 0x49 /* I key */:
    key = OS_KEY_I;
    break;
    case 0x4A /* J key */:
    key = OS_KEY_J;
    break;
    case 0x4B /* K key */:
    key = OS_KEY_K;
    break;
    case 0x4C /* L key */:
    key = OS_KEY_L;
    break;
    case 0x4D /* M key */:
    key = OS_KEY_M;
    break;
    case 0x4E /* N key */:
    key = OS_KEY_N;
    break;
    case 0x4F /* O key */:
    key = OS_KEY_O;
    break;
    case 0x50 /* P key */:
    key = OS_KEY_P;
    break;
    case 0x51 /* Q key */:
    key = OS_KEY_Q;
    break;
    case 0x52 /* R key */:
    key = OS_KEY_R;
    break;
    case 0x53 /* S key */:
    key = OS_KEY_S;
    break;
    case 0x54 /* T key */:
    key = OS_KEY_T;
    break;
    case 0x55 /* U key */:
    key = OS_KEY_U;
    break;
    case 0x56 /* V key */:
    key = OS_KEY_V;
    break;
    case 0x57 /* W key */:
    key = OS_KEY_W;
    break;
    case 0x58 /* X key */:
    key = OS_KEY_X;
    break;
    case 0x59 /* Y key */:
    key = OS_KEY_Y;
    break;
    case 0x5A /* Z key */:
    key = OS_KEY_Z;
    break;
    case VK_LWIN /* 0x5B	Left Windows logo key */:
    key = OS_KEY_LEFT_META;
    break;
    case VK_RWIN /* 0x5C	Right Windows logo key */:
    key = OS_KEY_RIGHT_META;
    break;
    // 0x5E	Reserved
    case VK_NUMPAD0 /* 0x60	Numeric keypad 0 key */:
    key = OS_KEY_0;
    break;
    case VK_NUMPAD1 /* 0x61	Numeric keypad 1 key */:
    key = OS_KEY_1;
    break;
    case VK_NUMPAD2 /* 0x62	Numeric keypad 2 key */:
    key = OS_KEY_2;
    break;
    case VK_NUMPAD3 /* 0x63	Numeric keypad 3 key */:
    key = OS_KEY_3;
    break;
    case VK_NUMPAD4 /* 0x64	Numeric keypad 4 key */:
    key = OS_KEY_4;
    break;
    case VK_NUMPAD5 /* 0x65	Numeric keypad 5 key */:
    key = OS_KEY_5;
    break;
    case VK_NUMPAD6 /* 0x66	Numeric keypad 6 key */:
    key = OS_KEY_6;
    break;
    case VK_NUMPAD7 /* 0x67	Numeric keypad 7 key */:
    key = OS_KEY_7;
    break;
    case VK_NUMPAD8 /* 0x68	Numeric keypad 8 key */:
    key = OS_KEY_8;
    break;
    case VK_NUMPAD9 /* 0x69	Numeric keypad 9 key */:
    key = OS_KEY_9;
    break;

    case VK_F1 /* 0x70	F1 key */:
    key = OS_KEY_F1;
    break;
    case VK_F2 /* 0x71	F2 key */:
    key = OS_KEY_F2;
    break;
    case VK_F3 /* 0x72	F3 key */:
    key = OS_KEY_F3;
    break;
    case VK_F4 /* 0x73	F4 key */:
    key = OS_KEY_F4;
    break;
    case VK_F5 /* 0x74	F5 key */:
    key = OS_KEY_F5;
    break;
    case VK_F6 /* 0x75	F6 key */:
    key = OS_KEY_F6;
    break;
    case VK_F7 /* 0x76	F7 key */:
    key = OS_KEY_F7;
    break;
    case VK_F8 /* 0x77	F8 key */:
    key = OS_KEY_F8;
    break;
    case VK_F9 /* 0x78	F9 key */:
    key = OS_KEY_F9;
    break;
    case VK_F10 /* 0x79	F10 key */:
    key = OS_KEY_F10;
    break;
    case VK_F11 /* 0x7A	F11 key */:
    key = OS_KEY_F11;
    break;
    case VK_F12 /* 0x7B	F12 key */:
    key = OS_KEY_F12;
    break;
    // 0x88-8F	Reserved
    case VK_SCROLL /* 0x91	Scroll lock key */:
    break;
    // 0x92-96	OEM specific
    // 0x97-9F	Unassigned
    case VK_LSHIFT /* 0xA0	Left Shift key */:
    key = OS_KEY_LEFT_SHIFT;
    break;
    case VK_RSHIFT /* 0xA1	Right Shift key */:
    key = OS_KEY_RIGHT_SHIFT;
    break;
    case VK_LCONTROL /* 0xA2	Left Ctrl key */:
    key = OS_KEY_LEFT_CONTROL;
    break;
    case VK_RCONTROL /* 0xA3	Right Ctrl key */:
    key = OS_KEY_RIGHT_CONTROL;
    break;
    case VK_LMENU /* 0xA4	Left Alt key */:
    key = OS_KEY_LEFT_ALT;
    break;
    case VK_RMENU /* 0xA5	Right Alt key */:
    key = OS_KEY_RIGHT_ALT;
    break;
    // 0xB8-B9	Reserved
    case VK_OEM_1 /* 0xBA	It can vary by keyboard. For the US ANSI keyboard , the Semiсolon and Colon key */:
    break;
    case VK_OEM_PLUS /* 0xBB	For any country/region, the Equals and Plus key */:
    break;
    case VK_OEM_COMMA /* 0xBC	For any country/region, the Comma and Less Than key */:
    break;
    case VK_OEM_MINUS /* 0xBD	For any country/region, the Dash and Underscore key */:
    break;
    case VK_OEM_PERIOD /* 0xBE	For any country/region, the Period and Greater Than key */:
    break;
    case VK_OEM_2 /* 0xBF	It can vary by keyboard. For the US ANSI keyboard, the Forward Slash and Question Mark key */:
    break;
    case VK_OEM_3 /* 0xC0	It can vary by keyboard. For the US ANSI keyboard, the Grave Accent and Tilde key */:
    break;
    case VK_OEM_4 /* 0xDB	It can vary by keyboard. For the US ANSI keyboard, the Left Brace key */:
    break;
    case VK_OEM_5 /* 0xDC	It can vary by keyboard. For the US ANSI keyboard, the Backslash and Pipe key */:
    break;
    case VK_OEM_6 /* 0xDD	It can vary by keyboard. For the US ANSI keyboard, the Right Brace key */:
    break;
    case VK_OEM_7 /* 0xDE	It can vary by keyboard. For the US ANSI keyboard, the Apostrophe and Double Quotation Mark key */:
    break;
    case VK_OEM_8 /* 0xDF	It can vary by keyboard. For the Canadian CSA keyboard, the Right Ctrl key */:
    break;
    // 0xE0	Reserved
    // 0xE1	OEM specific
    case VK_OEM_102 /* 0xE2	It can vary by keyboard. For the European ISO keyboard, the Backslash and Pipe key */:
    break;
    // 0xC1-C2	Reserved
    case VK_GAMEPAD_A /* 0xC3	Gamepad A button */:
    break;
    case VK_GAMEPAD_B /* 0xC4	Gamepad B button */:
    break;
    case VK_GAMEPAD_X /* 0xC5	Gamepad X button */:
    break;
    case VK_GAMEPAD_Y /* 0xC6	Gamepad Y button */:
    break;
    case VK_GAMEPAD_RIGHT_SHOULDER /* 0xC7	Gamepad Right Shoulder button */:
    break;
    case VK_GAMEPAD_LEFT_SHOULDER /* 0xC8	Gamepad Left Shoulder button */:
    break;
    case VK_GAMEPAD_LEFT_TRIGGER /* 0xC9	Gamepad Left Trigger button */:
    break;
    case VK_GAMEPAD_RIGHT_TRIGGER /* 0xCA	Gamepad Right Trigger button */:
    break;
    case VK_GAMEPAD_DPAD_UP /* 0xCB	Gamepad D-pad Up button */:
    break;
    case VK_GAMEPAD_DPAD_DOWN /* 0xCC	Gamepad D-pad Down button */:
    break;
    case VK_GAMEPAD_DPAD_LEFT /* 0xCD	Gamepad D-pad Left button */:
    break;
    case VK_GAMEPAD_DPAD_RIGHT /* 0xCE	Gamepad D-pad Right button */:
    break;
    case VK_GAMEPAD_MENU /* 0xCF	Gamepad Menu/Start button */:
    break;
    case VK_GAMEPAD_VIEW /* 0xD0	Gamepad View/Back button */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON /* 0xD1	Gamepad Left Thumbstick button */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON /* 0xD2	Gamepad Right Thumbstick button */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP /* 0xD3	Gamepad Left Thumbstick up */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN /* 0xD4	Gamepad Left Thumbstick down */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT /* 0xD5	Gamepad Left Thumbstick right */:
    break;
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT /* 0xD6	Gamepad Left Thumbstick left */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP /* 0xD7	Gamepad Right Thumbstick up */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN /* 0xD8	Gamepad Right Thumbstick down */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT /* 0xD9	Gamepad Right Thumbstick right */:
    break;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT /* 0xDA	Gamepad Right Thumbstick left */:
    break;
    // 0xE6	OEM specific
    case VK_PACKET /* 0xE7	Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP */:
    break;
    // 0xE8	Unassigned
  }

  return key;
}

LRESULT CALLBACK os_win32_main_window_callback(
  HWND window,
  UINT message,
  WPARAM w_param,
  LPARAM l_param
) {
  LRESULT result = 0;

  switch(message) {
    case WM_SIZE: {

      OS_Win32_Window_Dimensions window_dimensions = os_win32_get_window_dimensions(window);

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

      OS_Event *event = os_event_push(app_event_arena, app_event_list, release ? OS_EVENT_KEY_RELEASE : OS_EVENT_KEY_PRESS);


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

      OS_Win32_Window_Dimensions window_dimensions = os_win32_get_window_dimensions(window);
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
  void *lib = os_library_open(scratch, str8_lit("Xinput1_4.dll"));
  if(lib) {
    os_win32_xinput_get_state = (OS_Win32_XInput_Get_State_Func*)os_library_load_func(scratch, lib, str8_lit("XInputGetState"));
    os_win32_xinput_set_state = (OS_Win32_XInput_Set_State_Func*)os_library_load_func(scratch, lib, str8_lit("XInputSetState"));
  }
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

  WNDCLASS window_class = {0};
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

      app_is_running = true;

      MSG message;
      int x_offset = 0;
      int y_offset = 0;

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

        // NOTE jfd: blit to screen
        HDC device_context;
        defer_loop(device_context = GetDC(window_handle), ReleaseDC(window_handle, device_context)) {
          OS_Win32_Window_Dimensions window_dimensions = os_win32_get_window_dimensions(window_handle);

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
