#ifndef GFX_H
#define GFX_H


typedef enum GFX_EventKind {
  GFX_EVENT_NULL = 0,
  GFX_EVENT_KEY_PRESS,
  GFX_EVENT_KEY_RELEASE,
  GFX_EVENT_MOUSE_MOVE,
  GFX_EVENT_MOUSE_SCROLL,
} GFX_EventKind;

typedef enum GFX_ModifierIndex {
  GFX_MOD_INDEX_NONE = -1,

  GFX_MOD_INDEX_SHIFT,
  GFX_MOD_INDEX_CONTROL,
  GFX_MOD_INDEX_ALT,
  GFX_MOD_INDEX_META,

  GFX_MOD_INDEX_MAX,
} GFX_Modifier_Index;

typedef u32 GFX_Modifier;
#define GFX_MOD_SHIFT   ((GFX_Modifier)(1 << GFX_MOD_INDEX_SHIFT))
#define GFX_MOD_CONTROL ((GFX_Modifier)(1 << GFX_MOD_INDEX_CONTROL))
#define GFX_MOD_ALT     ((GFX_Modifier)(1 << GFX_MOD_INDEX_ALT))
#define GFX_MOD_META    ((GFX_Modifier)(1 << GFX_MOD_INDEX_META))

typedef enum GFX_Key {
  GFX_KEY_NULL = 0,

  GFX_MOUSE_BUTTON_LEFT,
  GFX_MOUSE_BUTTON_RIGHT,
  GFX_MOUSE_BUTTON_MIDDLE,

  GFX_KEY_LEFT_SHIFT,
  GFX_KEY_RIGHT_SHIFT,
  GFX_KEY_LEFT_CONTROL,
  GFX_KEY_RIGHT_CONTROL,
  GFX_KEY_LEFT_ALT,
  GFX_KEY_RIGHT_ALT,
  GFX_KEY_RIGHT_META,
  GFX_KEY_LEFT_META,

  GFX_KEY_ESCAPE,
  GFX_KEY_ENTER,
  GFX_KEY_SPACE,
  GFX_KEY_TAB,
  GFX_KEY_BACKSPACE,
  GFX_KEY_DELETE,
  GFX_KEY_CAPS_LOCK,
  GFX_KEY_PRINT_SCREEN,
  GFX_KEY_HOME,
  GFX_KEY_END,
  GFX_KEY_INSERT,
  GFX_KEY_PAGE_DOWN,
  GFX_KEY_PAGE_UP,
  GFX_KEY_LEFT_ARROW,
  GFX_KEY_RIGHT_ARROW,
  GFX_KEY_UP_ARROW,
  GFX_KEY_DOWN_ARROW,

  GFX_KEY_F1,
  GFX_KEY_F2,
  GFX_KEY_F3,
  GFX_KEY_F4,
  GFX_KEY_F5,
  GFX_KEY_F6,
  GFX_KEY_F7,
  GFX_KEY_F8,
  GFX_KEY_F9,
  GFX_KEY_F10,
  GFX_KEY_F11,
  GFX_KEY_F12,

  GFX_KEY_0,
  GFX_KEY_1,
  GFX_KEY_2,
  GFX_KEY_3,
  GFX_KEY_4,
  GFX_KEY_5,
  GFX_KEY_6,
  GFX_KEY_7,
  GFX_KEY_8,
  GFX_KEY_9,

  GFX_KEY_A,
  GFX_KEY_B,
  GFX_KEY_C,
  GFX_KEY_D,
  GFX_KEY_E,
  GFX_KEY_F,
  GFX_KEY_G,
  GFX_KEY_H,
  GFX_KEY_I,
  GFX_KEY_J,
  GFX_KEY_K,
  GFX_KEY_L,
  GFX_KEY_M,
  GFX_KEY_N,
  GFX_KEY_O,
  GFX_KEY_P,
  GFX_KEY_Q,
  GFX_KEY_R,
  GFX_KEY_S,
  GFX_KEY_T,
  GFX_KEY_U,
  GFX_KEY_V,
  GFX_KEY_W,
  GFX_KEY_X,
  GFX_KEY_Y,
  GFX_KEY_Z,

  GFX_KEY_MAX,

} GFX_Key;


typedef struct GFX_Event GFX_Event;
struct GFX_Event {
  GFX_Event *next;
  GFX_Event *prev;
  GFX_EventKind kind;
  GFX_Modifier modifier_mask;
  GFX_Key key;
  b32 is_repeat;
  u32 repeat_count;
  u32 character;
  Vec2 mouse_pos;
  Vec2 scroll_delta;
};

typedef struct GFX_Input GFX_Input;
struct GFX_Input {
  GFX_Modifier modifier_mask;
  u32 key_pressed[GFX_KEY_MAX];
  b32 key_released[GFX_KEY_MAX];
  Vec2 mouse_pos;
  Vec2 mouse_delta;
};

typedef struct GFX_EventList GFX_EventList;
struct GFX_EventList {
  s64 count;
  GFX_Event *first;
  GFX_Event *last;
};


GFX_Modifier gfx_get_modifiers(void);
b32          gfx_is_modifier_key(GFX_Key key);
void         gfx_poll_input_events(GFX_EventList *event_list, GFX_Input *input);
b32          gfx_is_key_pressed(GFX_Input *input, GFX_Key key);
b32          gfx_was_key_pressed_once(GFX_Input *input, GFX_Key key);
b32          gfx_was_key_released(GFX_Input *input, GFX_Key key);


#endif
