#ifndef PLATFORM_C
#define PLATFORM_C

// NOTE jfd 22/01/2026:
// This file contains implementations for things defined in platform.h that are not actually platform specific.
// I.E. various helpers for platform absctraction things.

internal b32
func platform_is_modifier_key(KeyboardKey key) {
  b32 result = (key >= KBD_KEY_LEFT_SHIFT && key <= KBD_KEY_LEFT_META);
  return result;
}

#endif
