#include "base.h"

/*
 * types
 */

typedef struct OS_Win32_Backbuffer OS_Win32_Backbuffer;
typedef struct OS_Win32_Window_Dimensions OS_Win32_Window_Dimensions;

/*
 * structs
 */

struct OS_Win32_Backbuffer {
  BITMAPINFO bitmap_info;
  u8 *bitmap_memory;
  s32 bitmap_width;
  s32 bitmap_height;
  u32 bytes_per_pixel;
  u32 stride;
};

struct OS_Win32_Window_Dimensions {
  s32 width;
  s32 height;
};

/*
 * headers
 */

LRESULT CALLBACK os_win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
void os_win32_resize_dib_section(OS_Win32_Backbuffer *backbuffer, int window_width, int window_height);
void os_win32_display_buffer_in_window(OS_Win32_Backbuffer *backbuffer, HDC device_context, int window_width, int window_height, int x, int y, int width, int height);
OS_Win32_Window_Dimensions os_win32_get_window_dimensions(HWND window_handle);

/*
 * globals
 */

global b32 app_is_running;
global OS_Win32_Backbuffer global_backbuffer = {
  .bytes_per_pixel = 4,
};

/*
 * functions
 */

void render_weird_gradient(OS_Win32_Backbuffer *backbuffer, int x_offset, int y_offset) {
  u8 *row = (u8*)backbuffer->bitmap_memory;
  for(int y = 0; y < backbuffer->bitmap_height; y++) {
    u32 *pixel = (u32*)row;

    for(int x = 0; x < backbuffer->bitmap_width; x++) {
      //
      // pixel in memory
      //  0  1  2  3
      //  B  G  R  x
      // 00 00 00 00

      u8 r = (u8)(x + x_offset);
      u8 g = 0;
      u8 b = (u8)(y + y_offset);

      *pixel++ = (r << 16) | (g << 8) | b;
    }

    row += backbuffer->stride;
  }

}

OS_Win32_Window_Dimensions os_win32_get_window_dimensions(HWND window_handle) {
  RECT client_rect;
  GetClientRect(window_handle, &client_rect);

  OS_Win32_Window_Dimensions window_dimensions = {
    .width = client_rect.right - client_rect.left,
    .height = client_rect.bottom - client_rect.top,
  };

  return window_dimensions;
}

void os_win32_resize_dib_section(OS_Win32_Backbuffer *backbuffer, int window_width, int window_height) {
  int width = window_width;
  int height = window_height;
  // TODO jfd: bulletproof this
  // maybe don't free first, free after, then free first if that fails

  // TODO jfd: free our DIBSection

  if(backbuffer->bitmap_memory) {
    os_free(backbuffer->bitmap_memory);
  }

  backbuffer->bitmap_width = width;
  backbuffer->bitmap_height = height;

  backbuffer->bitmap_info.bmiHeader.biSize = sizeof(backbuffer->bitmap_info.bmiHeader);
  backbuffer->bitmap_info.bmiHeader.biWidth = backbuffer->bitmap_width;
  backbuffer->bitmap_info.bmiHeader.biHeight = -backbuffer->bitmap_height;
  backbuffer->bitmap_info.bmiHeader.biPlanes = 1;
  backbuffer->bitmap_info.bmiHeader.biBitCount = 32;
  backbuffer->bitmap_info.bmiHeader.biCompression = BI_RGB; // pixel format

  u64 bitmap_memory_size = backbuffer->bitmap_width*backbuffer->bitmap_height * backbuffer->bytes_per_pixel;
  backbuffer->bitmap_memory = os_alloc(bitmap_memory_size);

  backbuffer->stride = backbuffer->bitmap_width * backbuffer->bytes_per_pixel;
}

void os_win32_display_buffer_in_window(
  OS_Win32_Backbuffer *backbuffer,
  HDC device_context,
  int window_width,
  int window_height,
  int x,
  int y,
  int width,
  int height
) {

  StretchDIBits(
    device_context,
    0, 0, backbuffer->bitmap_width, backbuffer->bitmap_height,
    0, 0, window_width, window_height, // destination coordinates
    backbuffer->bitmap_memory,
    &backbuffer->bitmap_info,
    DIB_RGB_COLORS,
    SRCCOPY
  );

}

LRESULT CALLBACK os_win32_main_window_callback(
  HWND window,
  UINT message,
  WPARAM w_param,
  LPARAM l_param
) {
  LRESULT result = 0;

  switch(message) {
    case WM_SIZE: {

      OS_Win32_Window_Dimensions window_dimensions = os_win32_get_window_dimensions(window);

      os_win32_resize_dib_section(&global_backbuffer, window_dimensions.width, window_dimensions.height);

      OutputDebugStringA("WM_SIZE\n");
    } break;
    case WM_DESTROY: {
      app_is_running = false;
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE: {
      app_is_running = false;
      OutputDebugStringA("WM_CLOSE\n");
    } break;
    case WM_ACTIVATEAPP: {
      OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;
    case WM_PAINT: {
      PAINTSTRUCT paint;

      HDC device_context = BeginPaint(window, &paint);

      int x = paint.rcPaint.left;
      int y = paint.rcPaint.top;
      int width = paint.rcPaint.right - x;
      int height = paint.rcPaint.bottom - y;

      static int color = WHITENESS;
      if(color == WHITENESS) {
        color = BLACKNESS;
      } else {
        color = WHITENESS;
      }

      RECT client_rect;
      GetClientRect(window, &client_rect);
      OS_Win32_Window_Dimensions window_dimensions = os_win32_get_window_dimensions(window);
      os_win32_display_buffer_in_window(&global_backbuffer, device_context, window_dimensions.width, window_dimensions.height, x, y, width, height);
      EndPaint(window, &paint);
    } break;
    default: {
      result = DefWindowProc(window, message, w_param, l_param);
    } break;
  }

  return result;
}

int CALLBACK WinMain(
  HINSTANCE instance,
  HINSTANCE prev_instance,
  LPSTR cmd_line,
  int show_code
) {
  WNDCLASS window_class = {0};
  window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
  window_class.lpfnWndProc = os_win32_main_window_callback;
  window_class.hInstance = instance;
  window_class.lpszClassName = "Handmade Hero Window Class";

  if(RegisterClass(&window_class)) {
    HWND window_handle =
    CreateWindowExA(
      0,
      window_class.lpszClassName,
      "Handmade Hero",
      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      instance,
      0);

    if(window_handle) {

      app_is_running = true;

      MSG message;
      int x_offset = 0;
      int y_offset = 0;

      while(app_is_running) {
        // NOTE jfd: get input messages
        while(PeekMessageA(&message, window_handle, 0, 0, PM_REMOVE)) {
          if(message.message == WM_QUIT) {
            app_is_running = false;
          }
          TranslateMessage(&message); // NOTE jfd: has to do with keyboard messages
          DispatchMessage(&message);

        }

        { // draw
          render_weird_gradient(&global_backbuffer, x_offset, y_offset);

          ++x_offset;
        } // draw

        // NOTE jfd: blit to screen
        HDC device_context;
        defer_loop(device_context = GetDC(window_handle), ReleaseDC(window_handle, device_context)) {
          OS_Win32_Window_Dimensions window_dimensions = os_win32_get_window_dimensions(window_handle);

          os_win32_display_buffer_in_window(&global_backbuffer, device_context, window_dimensions.width, window_dimensions.height, 0, 0, window_dimensions.width, window_dimensions.height);
        }
      }

    } else {
    }

  } else {
    // TODO jfd: logging
  }

  return 0;
}
