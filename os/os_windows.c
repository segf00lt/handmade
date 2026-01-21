#if OS_WINDOWS

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#include <windows.h>
#include <direct.h>
#include <shellapi.h>

void* os_alloc(u64 bytes) {
  void *result = (void*)VirtualAlloc(
    0,
    (SIZE_T)bytes,
    MEM_RESERVE | MEM_COMMIT,
    PAGE_READWRITE
  );
  return result;
}

void os_free(void *ptr) {
  VirtualFree(ptr, 0, MEM_RELEASE);
}

Str8 os_read_entire_file(Arena *a, Str8 path) {
  Str8 result = {0};

  FILE *f = 0;
  arena_scope(a) {
    f = fopen(cstr_copy_str8(a, path), "rb");
  }

  if(f == NULL) {
    goto end;
  }

  if(fseek(f, 0, SEEK_END) < 0) {
    goto end;
  }

  long long m = _ftelli64(f);

  if(m < 0) {
    goto end;
  }

  if(fseek(f, 0, SEEK_SET) < 0) {
    goto end;
  }

  u8 *p = push_array_no_zero(a, u8, (u64)m);

  fread((void*)p, 1, m, f);

  if(ferror(f)) {
    goto end;
  }

  result.s = p;
  result.len = (s64)m;

end:

  if(f) {
    fclose(f);
  }

  return result;
}

b32 os_file_exists(Arena *a, Str8 path) {
  b32 result = 0;

  arena_scope(a) {
    char *cstr_path = cstr_copy_str8(a, path);
    DWORD dwAttrib = GetFileAttributesA(cstr_path);
    result = dwAttrib != INVALID_FILE_ATTRIBUTES;
  }

  return result;
}

Str8 os_get_current_dir(Arena *a) {
  DWORD buf_size = GetCurrentDirectory(0, NULL);
  ASSERT(buf_size > 0);

  char *buf = push_array_no_zero(a, char, buf_size);
  ASSERT(GetCurrentDirectory(buf_size, buf) != 0);

  Str8 result = { .s = (u8*)buf, .len = buf_size - 1 };

  return result;
}

b32 os_set_current_dir_cstr(char *dir_path_cstr) {
  b32 result = 0;

  result = SetCurrentDirectory(dir_path_cstr);

  return result;
}

b32 os_move_file(Arena *a, Str8 old_path, Str8 new_path) {
  b32 result = 0;

  arena_scope(a) {
    const char *old_path_cstr = cstr_copy_str8(a, old_path);
    const char *new_path_cstr = cstr_copy_str8(a, new_path);

    result = MoveFileEx(old_path_cstr, new_path_cstr, MOVEFILE_REPLACE_EXISTING);
  }

  return result;
}

b32 os_remove_file(Arena *a, Str8 path) {
  b32 result = 1;

  arena_scope(a) {
    const char *path_cstr = cstr_copy_str8(a, path);
    if(!DeleteFileA(path_cstr)) {
      result = 0;
    }
  }

  return result;
}

void* os_library_load(Arena *a, Str8 path) {
  void *lib = 0;

  arena_scope(a) {
    LPCSTR path_cstr = (LPCSTR)cstr_copy_str8(a, path);
    HMODULE dll = LoadLibraryA(path_cstr);
    lib = (void*)dll;
  }

  return lib;
}

void os_library_unload(void* lib) {
  FreeLibrary((HMODULE)lib);
}

Void_Func* os_library_load_func(Arena *a, void* lib, Str8 name) {
  Void_Func *fn = 0;

  arena_scope(a) {
    HMODULE dll_handle = (HMODULE)lib;
    LPCSTR func_name = (LPCSTR)cstr_copy_str8(a, name);
    fn = (Void_Func*)GetProcAddress(dll_handle, func_name);
  }

  return fn;
}

b32 os_make_dir(Arena *a, Str8 path) {
  b32 result = 0;
  arena_scope(a) {
    result = _mkdir(cstr_copy_str8(a, path));
  }

  if(result < 0) {
    if(errno == EEXIST) {
      return 1;
    }

    return 0;
  }

  return 1;
}

#endif
