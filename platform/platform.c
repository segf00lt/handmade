#ifndef PLATFORM_C
#define PLATFORM_C

b32 platform_is_modifier_key(KeyboardKey key) {
  b32 result = (key >= KBD_KEY_LEFT_SHIFT && key <= KBD_KEY_LEFT_META);
  return result;
}

#endif
