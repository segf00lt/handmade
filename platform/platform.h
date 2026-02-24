#ifndef PLATFORM_H
#define PLATFORM_H


// NOTE jfd 22/01/2026:
// This file contains function interfaces that the game (or application) can call the platform through.
// It serves as a platform abstraction layer that contains only those things that the game
// absolutely must be able to ask the platform for.


typedef enum Platform_event_kind {
  EVENT_NULL = 0,
  EVENT_KEY_PRESS,
  EVENT_KEY_RELEASE,
  EVENT_MOUSE_MOVE,
  EVENT_MOUSE_CLICK,
  EVENT_MOUSE_SCROLL,
} Platform_event_kind;


typedef struct Platform_event Platform_event;
struct Platform_event {
  Platform_event *next;
  Platform_event *prev;
  Platform_event_kind kind;
  Keyboard_modifier modifier_mask;
  Keyboard_key key;
  Mouse_button mouse_button;
  b16 is_repeat;
  u16 repeat_count;
  u32 character;
  v2 mouse_pos;
  v2 scroll_delta;
};

typedef struct Platform_event_list Platform_event_list;
struct Platform_event_list {
  s64 count;
  Platform_event *first;
  Platform_event *last;
};


typedef struct Platform Platform;
struct Platform {
  void *backbuffer;
  u64   backbuffer_size;
};

#if defined(PLATFORM_CORE_EXPORT)

#if OS_WINDOWS
#define PLATFORM_CORE_API __declspec(dllexport)
#endif

#elif defined(PLATFORM_CORE_IMPORT)

#if OS_WINDOWS
#define PLATFORM_CORE_API __declspec(dllimport)
#endif

#else
#define PLATFORM_CORE_API internal
#endif


PLATFORM_CORE_API    Keyboard_modifier platform_get_keyboard_modifiers(void);

PLATFORM_CORE_API    void*             platform_alloc(u64 bytes);
PLATFORM_CORE_API    void              platform_free(void *ptr);
PLATFORM_CORE_API    Str8              platform_read_entire_file(Arena *a, char *path);
PLATFORM_CORE_API    b32               platform_write_entire_file(Str8 data, char *path);
PLATFORM_CORE_API    b32               platform_file_exists(char *path);
PLATFORM_CORE_API    b32               platform_make_dir(char *dir_path);
PLATFORM_CORE_API    Str8              platform_get_current_dir(Arena *a);
PLATFORM_CORE_API    b32               platform_set_current_dir(char *dir_path);
PLATFORM_CORE_API    b32               platform_move_file(char *old_path, char *new_path);
PLATFORM_CORE_API    b32               platform_remove_file(char *path);
PLATFORM_CORE_API    Str8              platform_file_name_from_path(Str8 path);

PLATFORM_CORE_API    void              platform_sleep_ms(u32 ms);

PLATFORM_CORE_API    void*             platform_library_load(char *path);
PLATFORM_CORE_API    void              platform_library_unload(void *lib);
PLATFORM_CORE_API    Void_func*        platform_library_load_function(void *lib, char *name);



#endif
