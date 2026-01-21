#ifndef GFX_WINDOWS_H
#define GFX_WINDOWS_H


#include <objbase.h>


OS_Event* os_win32_event_push(Arena *a, OS_EventList *event_list, OS_EventKind event_kind);
OS_Event* os_win32_event_pop(OS_EventList *event_list);

KeyboardKey os_win32_keyboard_key_from_virtual_keycode(WPARAM virtual_keycode);

#endif
