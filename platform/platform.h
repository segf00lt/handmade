#ifndef PLATFORM_H
#define PLATFORM_H


// NOTE jfd 22/01/2026:
// This file contains function interfaces that the game (or application) can call the platform through.
// It serves as a platform abstraction layer that contains only those things that the game
// absolutely must be able to ask the platform for.


internal        Keyboard_modifier platform_get_keyboard_modifiers(void);

internal        Str8              platform_read_entire_file(char *path);
internal        b32               platform_write_entire_file(Str8 data, char *path);
internal        b32               platform_file_exists(char *path);
force_inline    b32               platform_make_dir(char *dir_path);
internal        Str8              platform_get_current_dir(Arena *a);
internal        b32               platform_set_current_dir(char *dir_path);
internal        b32               platform_move_file(char *old_path, char *new_path);
internal        b32               platform_remove_file(char *path);
internal        Str8              platform_file_name_from_path(Str8 path);

force_inline    void              platform_sleep_ms(s32 ms);



#endif
