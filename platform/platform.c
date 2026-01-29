#ifndef PLATFORM_C
#define PLATFORM_C

// NOTE jfd 22/01/2026:
// This file contains implementations for things defined in platform.h that are not actually platform specific.
// I.E. various helpers for platform absctraction things.


internal Str8
func platform_file_name_from_path(Str8 path) {
  Str8 result;

  s64 i;

  for(i = path.len - 1; i >= 0; i--) {
    if(path.s[i] == '/') break;
  }

  result = str8_slice(path, i + 1, path.len);

  return result;
}


#endif
