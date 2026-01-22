#ifndef PLATFORM_H
#define PLATFORM_H


// NOTE jfd 22/01/2026:
// This file contains interfaces that the game (or application) can call the platform from.
// It serves as a platform abstraction layer that contains only those things that the game
// absolutely must be able to ask the platform for.

typedef enum PLTFM_EventKind {
  EVENT_NULL = 0,
  EVENT_KEY_PRESS,
  EVENT_KEY_RELEASE,
  EVENT_MOUSE_MOVE,
  EVENT_MOUSE_CLICK,
  EVENT_MOUSE_SCROLL,
} PLTFM_EventKind;

typedef enum KeyboardModifierIndex {
  KBD_MOD_INDEX_NONE = -1,

  KBD_MOD_INDEX_SHIFT,
  KBD_MOD_INDEX_CONTROL,
  KBD_MOD_INDEX_ALT,
  KBD_MOD_INDEX_META,

  KBD_MOD_INDEX_MAX,
} KeyboardModifierIndex;

typedef u32 KeyboardModifier;
#define KBD_MOD_SHIFT   ((KeyboardModifier)(1 << KBD_MOD_INDEX_SHIFT))
#define KBD_MOD_CONTROL ((KeyboardModifier)(1 << KBD_MOD_INDEX_CONTROL))
#define KBD_MOD_ALT     ((KeyboardModifier)(1 << KBD_MOD_INDEX_ALT))
#define KBD_MOD_META    ((KeyboardModifier)(1 << KBD_MOD_INDEX_META))

typedef enum MouseButton {
  MOUSE_BUTTON_LEFT,
  MOUSE_BUTTON_RIGHT,
  MOUSE_BUTTON_MIDDLE,
} MouseButton;

typedef enum KeyboardKey {
  KBD_KEY_NULL = 0,

  KBD_KEY_LEFT_SHIFT,
  KBD_KEY_RIGHT_SHIFT,
  KBD_KEY_LEFT_CONTROL,
  KBD_KEY_RIGHT_CONTROL,
  KBD_KEY_LEFT_ALT,
  KBD_KEY_RIGHT_ALT,
  KBD_KEY_RIGHT_META,
  KBD_KEY_LEFT_META,

  KBD_KEY_ESCAPE,
  KBD_KEY_ENTER,
  KBD_KEY_SPACE,
  KBD_KEY_TAB,
  KBD_KEY_BACKSPACE,
  KBD_KEY_DELETE,
  KBD_KEY_CAPS_LOCK,
  KBD_KEY_PRINT_SCREEN,
  KBD_KEY_HOME,
  KBD_KEY_END,
  KBD_KEY_INSERT,
  KBD_KEY_PAGE_DOWN,
  KBD_KEY_PAGE_UP,
  KBD_KEY_LEFT_ARROW,
  KBD_KEY_RIGHT_ARROW,
  KBD_KEY_UP_ARROW,
  KBD_KEY_DOWN_ARROW,

  KBD_KEY_F1,
  KBD_KEY_F2,
  KBD_KEY_F3,
  KBD_KEY_F4,
  KBD_KEY_F5,
  KBD_KEY_F6,
  KBD_KEY_F7,
  KBD_KEY_F8,
  KBD_KEY_F9,
  KBD_KEY_F10,
  KBD_KEY_F11,
  KBD_KEY_F12,

  KBD_KEY_0,
  KBD_KEY_1,
  KBD_KEY_2,
  KBD_KEY_3,
  KBD_KEY_4,
  KBD_KEY_5,
  KBD_KEY_6,
  KBD_KEY_7,
  KBD_KEY_8,
  KBD_KEY_9,

  KBD_KEY_A,
  KBD_KEY_B,
  KBD_KEY_C,
  KBD_KEY_D,
  KBD_KEY_E,
  KBD_KEY_F,
  KBD_KEY_G,
  KBD_KEY_H,
  KBD_KEY_I,
  KBD_KEY_J,
  KBD_KEY_K,
  KBD_KEY_L,
  KBD_KEY_M,
  KBD_KEY_N,
  KBD_KEY_O,
  KBD_KEY_P,
  KBD_KEY_Q,
  KBD_KEY_R,
  KBD_KEY_S,
  KBD_KEY_T,
  KBD_KEY_U,
  KBD_KEY_V,
  KBD_KEY_W,
  KBD_KEY_X,
  KBD_KEY_Y,
  KBD_KEY_Z,

  KBD_KEY_MAX,

} KeyboardKey;


typedef struct PLTFM_Event PLTFM_Event;
struct PLTFM_Event {
  PLTFM_Event *next;
  PLTFM_Event *prev;
  PLTFM_EventKind kind;
  KeyboardModifier modifier_mask;
  KeyboardKey key;
  MouseButton mouse_button;
  b16 is_repeat;
  u16 repeat_count;
  u32 character;
  Vec2 mouse_pos;
  Vec2 scroll_delta;
};

typedef struct PLTFM_EventList PLTFM_EventList;
struct PLTFM_EventList {
  s64 count;
  PLTFM_Event *first;
  PLTFM_Event *last;
};

internal KeyboardModifier platform_get_keyboard_modifiers(void);
internal b32              platform_is_modifier_key(KeyboardKey key);

internal Str8 DEBUG_platform_read_entire_file(Str8 path);
internal b32  DEBUG_platform_write_entire_file(Str8 data, Str8 path);


#endif
