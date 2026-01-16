#ifndef OS_H
#define OS_H

void* os_alloc(u64 size);
void  os_free(void *ptr);

Str8 os_get_current_dir(Arena *a);
b32 os_set_current_dir(Arena *a, Str8 dir_path);

b32 os_move_file(Arena *a, Str8 old_path, Str8 new_path);
b32 os_remove_file(Arena *a, Str8 path);

b32 os_file_exists(Arena *a, Str8 path);

Str8 os_read_entire_file(Arena *a, Str8 path);

// OS_file_iter* os_file_iter_begin(Arena *a, Str8 path, OS_file_iter_flags flags);
// b32 os_file_iter_next(Arena *a, OS_file_iter *iter, OS_file_info *file_info);
// void os_file_iter_end(OS_file_iter *iter);

Str8 os_file_name_from_path(Str8 path);

b32 os_make_dir(Arena *a, Str8 path);

b32 os_sleep_ms(u32 ms);

void*      os_library_load(Arena *a, Str8 path);
void       os_library_unload(void* lib);
Void_Func* os_library_load_func(Arena *a, void* lib, Str8 name);

#endif
