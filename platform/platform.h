#ifndef PLATFORM_H
#define PLATFORM_H


// NOTE jfd 22/01/2026:
// This file contains function interfaces that the game (or application) can call the platform through.
// It serves as a platform abstraction layer that contains only those things that the game
// absolutely must be able to ask the platform for.


internal KeyboardModifier platform_get_keyboard_modifiers(void);

internal Str8 platform_debug_read_entire_file(Str8 path);
internal b32  platform_debug_write_entire_file(Str8 data, Str8 path);


#endif
