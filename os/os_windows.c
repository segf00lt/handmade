#if defined(OS_WINDOWS)

#error "windows support not implemented"

#define OS_PATH_LEN MAX_PATH

Str8 os_read_entire_file(Arena *a, Str8 path) {
  Str8 result = {0};

  FILE *f;
  arena_scope(a) {
    f = fopen(push_cstr_copy_str8(a, path), "rb");
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

b32 os_file_exists(char *path) {
  b32 result = 0;

  if (_access(path, 0) != -1) result = 1;

  return result;
}

Str8 os_get_current_dir(void) {
  DWORD buf_size = GetCurrentDirectory(0, NULL);
  ASSERT(buf_size > 0);

  char *buf = scratch_push_array_no_zero(char, buf_size);
  ASSERT(GetCurrentDirectory(buf_size, buf) != 0);

  Str8 result = { .s = buf, .s = buf_size - 1 };

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
    const char *old_path_cstr = push_cstr_copy_str8(a, old_path); 
    const char *new_path_cstr = push_cstr_copy_str8(a, new_path); 

    result = MoveFileEx(old_path_cstr, new_path_cstr, MOVEFILE_REPLACE_EXISTING);
  }

  return result;
}

b32 os_remove_file(Arena *a, Str8 path) {
  u8 path_buf[OS_PATH_LEN+1];
  b32 result = 1;

  arena_scope(a) {
    const char *path_cstr = push_cstr_copy_str8(a, path);
    if(!DeleteFileA(path_cstr)) {
      result = 0;
    }
  }

  return result;
}

func b32 os_make_dir(char *path) {
  int result = mkdir(path);

  if(result < 0) {
    if(errno == EEXIST) {
      return 1;
    }

    return 0;
  }

  return 1;
}

#endif
