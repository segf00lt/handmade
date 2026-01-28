#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H


// NOTE jfd 22/01/2026:
// This file contains type definitions for the game (or application) to interface with the platform.

typedef enum Platform_EventKind {
  EVENT_NULL = 0,
  EVENT_KEY_PRESS,
  EVENT_KEY_RELEASE,
  EVENT_MOUSE_MOVE,
  EVENT_MOUSE_CLICK,
  EVENT_MOUSE_SCROLL,
} Platform_EventKind;


typedef struct Platform_Event Platform_Event;
struct Platform_Event {
  Platform_Event *next;
  Platform_Event *prev;
  Platform_EventKind kind;
  KeyboardModifier modifier_mask;
  KeyboardKey key;
  MouseButton mouse_button;
  b16 is_repeat;
  u16 repeat_count;
  u32 character;
  Vec2 mouse_pos;
  Vec2 scroll_delta;
};

typedef struct Platform_EventList Platform_EventList;
struct Platform_EventList {
  s64 count;
  Platform_Event *first;
  Platform_Event *last;
};



typedef KeyboardModifier Platform_GetKeyboardModifiersFunc(void);
typedef Str8             Platform_DebugReadEntireFileFunc(Str8);
typedef b32              Platform_DebugWriteEntireFileFunc(Str8, Str8);

typedef struct Platform_Vtable Platform_Vtable;
struct Platform_Vtable {
  Platform_GetKeyboardModifiersFunc *get_keyboard_modifiers;
  Platform_DebugReadEntireFileFunc  *debug_read_entire_file;
  Platform_DebugWriteEntireFileFunc *debug_write_entire_file;
};

typedef struct Platform Platform;
struct Platform {
  void *game_memory_backbuffer;
  u64 game_memory_backbuffer_size;
  Platform_Vtable vtable;
};

#endif
