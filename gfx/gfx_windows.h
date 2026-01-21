#ifndef GFX_WINDOWS_H
#define GFX_WINDOWS_H


#include <objbase.h>


GFX_Event* gfx_win32_event_push(Arena *a, GFX_EventList *event_list, GFX_EventKind event_kind);
GFX_Event* gfx_win32_event_pop(GFX_EventList *event_list);

GFX_Key gfx_win32_key_from_virtual_keycode(WPARAM virtual_keycode);

#endif
