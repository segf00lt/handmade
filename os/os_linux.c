#if defined(OS_LINUX)

#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

/*
 * function headers
 */

b32 os_sleep_ms(u32 ms);
Str8 os_file_name_from_path(Str8 path);
void* os_alloc(u64 size);
void os_free(void *ptr);
Str8 os_get_current_dir(Arena *a);
b32 os_move_file(Arena *a, Str8 old_path, Str8 new_path);
b32 os_remove_file(Arena *a, Str8 path);
b32 os_file_exists(Arena *a, Str8 path);
Str8 os_read_entire_file(Arena *a, Str8 path);
b32 os_make_dir(Arena *a, Str8 path);
void* os_library_open(Arena *a, Str8 path);
void os_library_close(void *lib);
Void_Func* os_library_load_func(Arena *a, void *lib, Str8 name);

/*
 * function bodies
 */

b32 os_sleep_ms(u32 ms) {
  b32 result = 1;
  struct timespec ts = {0};
  ts.tv_nsec = MILLION(1)*ms;
  int i = nanosleep(&ts, 0);
  if(i != 0) {
    result = 0;
  }
  return result;
}

Str8 os_file_name_from_path(Str8 path) {
  Str8 result;

  s64 i;

  for(i = path.len - 1; i >= 0; i--) {
    if(path.s[i] == '/') break;
  }

  result = str8_slice(path, i + 1, path.len);

  return result;
}

void* os_alloc(u64 size) {
  return malloc(size);
}

void os_free(void *ptr) {
  free(ptr);
}

Str8 os_get_current_dir(Arena *a) {
  size_t buf_size = 4096;

  Str8 result = {0};

  u64 pos = arena_pos(a);
  char *buf = push_array_no_zero(a, char, buf_size);

  char *s = getcwd(buf, buf_size);

  while(s == NULL && buf_size < 4*4096) {
    arena_pop_to(a, pos);
    buf_size <<= 1;
    buf = push_array_no_zero(a, char, buf_size);
    s = getcwd(buf, buf_size);
  }

  result.s = (u8*)s;
  result.len = memory_strlen(s);

  arena_pop(a, buf_size - (result.len - 1));

  return result;
}

b32 os_set_current_dir(Arena *a, Str8 path) {
  b32 result = 0;

  arena_scope(a) {
    char *path_cstr = cstr_copy_str8(a, path);
    result = !chdir(path_cstr);
  }

  return result;
}

b32 os_move_file(Arena *a, Str8 old_path, Str8 new_path) {
  b32 result = 0;

  arena_scope(a) {
    const char *old_path_cstr = cstr_copy_str8(a, old_path);
    const char *new_path_cstr = cstr_copy_str8(a, new_path);

    result = !rename(old_path_cstr, new_path_cstr);
  }

  return result;
}

b32 os_remove_file(Arena *a, Str8 path) {
  b32 result = 1;

  arena_scope(a) {
    const char *path_cstr = cstr_copy_str8(a, path);
    if(remove(path_cstr) < 0) {
      result = 0;
    }
  }

  return result;
}

b32 os_file_exists(Arena *a, Str8 path) {
  b32 result = 0;

  arena_scope(a) {
    struct stat statbuf;
    if(stat(cstr_copy_str8(a, path), &statbuf) == 0) result = 1;
  }

  return result;
}

Str8 os_read_entire_file(Arena *a, Str8 path) {
  Str8 result = {0};

  FILE *f;
  arena_scope(a) {
    f = fopen(cstr_copy_str8(a, path), "rb");
  }

  if(f == NULL) {
    goto end;
  }

  if(fseek(f, 0, SEEK_END) < 0) {
    goto end;
  }

  long m = ftell(f);

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

b32 os_make_dir(Arena *a, Str8 path) {
  int result;
  arena_scope(a) {
    result = mkdir(cstr_copy_str8(a, path), 0755);
  }

  if(result < 0) {
    if(errno == EEXIST) {
      return 1;
    }

    return 0;
  }

  return 1;
}

void* os_library_open(Arena *a, Str8 path) {
  void *lib = 0;

  arena_scope(a) {
    char *path_cstr = cstr_copy_str8(a, path);
    void *so = dlopen(path_cstr, RTLD_LAZY|RTLD_LOCAL);
    lib = so;
  }

  return lib;
}

void os_library_close(void *lib) {
  dlclose(lib);
}

Void_Func* os_library_load_func(Arena *a, void *lib, Str8 name) {
  Void_Func *fn = 0;
  arena_scope(a) {
    char *name_cstr = cstr_copy_str8(a, name);
    fn = (Void_Func*)dlsym(lib, name_cstr);
  }

  return fn;
}

#endif
