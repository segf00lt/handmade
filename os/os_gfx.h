#ifndef OS_GFX_H
#define OS_GFX_H

#include <objbase.h>
#include <Xinput.h>
#include <Dsound.h>

typedef struct OS_Win32_WindowDimensions OS_Win32_WindowDimensions;

typedef struct OS_EventList OS_EventList;
typedef struct OS_Event OS_Event;

typedef enum OS_EventKind {
  OS_EVENT_NULL = 0,
  OS_EVENT_KEY_PRESS,
  OS_EVENT_KEY_RELEASE,
  OS_EVENT_MOUSE_MOVE,
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


OS_Event* os_event_push(Arena *a, OS_EventList *event_list, OS_EventKind event_kind);
OS_Event* os_event_pop(OS_EventList *event_list);

OS_Modifier os_get_modifiers(void);
OS_Key os_win32_key_from_virtual_keycode(WPARAM virtual_keycode);
b32 os_is_modifier_key(OS_Key key);

#endif
