#include "base.h"

/*
 * headers
 */

LRESULT CALLBACK os_win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
void os_win32_resize_dib_section(int width, int height);
void os_win32_update_window(HDC device_context, RECT *window_rect, int x, int y, int width, int height);

/*
 * globals
 */

global b32 app_is_running;
global BITMAPINFO bitmap_info;
global void *bitmap_memory;
global int bitmap_width;
global int bitmap_height;
global const u64 bytes_per_pixel = 4;

/*
 * functions
 */

void render_weird_gradient(int x_offset, int y_offset) {
  u64 stride = bitmap_width * bytes_per_pixel;

  u8 *row = (u8*)bitmap_memory;
  for(int y = 0; y < bitmap_height; y++) {
    u32 *pixel = (u32*)row;

    for(int x = 0; x < bitmap_width; x++) {
      //
      // pixel in memory
      //  0  1  2  3
      //  B  G  R  x
      // 00 00 00 00

      u8 r = (x + x_offset);
      u8 g = 0;
      u8 b = (y + y_offset);

      *pixel++ = (r << 16) | (g << 8) | b;
    }

    row += stride;
  }

}

void os_win32_resize_dib_section(int width, int height) {
  // TODO jfd: bulletproof this
  // maybe don't free first, free after, then free first if that fails

  // TODO jfd: free our DIBSection

  if(bitmap_memory) {
    os_free(bitmap_memory);
  }

  bitmap_width = width;
  bitmap_height = height;

  bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
  bitmap_info.bmiHeader.biWidth = bitmap_width;
  bitmap_info.bmiHeader.biHeight = -bitmap_height;
  bitmap_info.bmiHeader.biPlanes = 1;
  bitmap_info.bmiHeader.biBitCount = 32;
  bitmap_info.bmiHeader.biCompression = BI_RGB; // pixel format

  u64 bitmap_memory_size = bitmap_width*bitmap_height * bytes_per_pixel;
  bitmap_memory = os_alloc(bitmap_memory_size);

}

void os_win32_update_window(HDC device_context, RECT *window_rect, int x, int y, int width, int height) {

  int window_width = window_rect->right - window_rect->left;
  int window_height = window_rect->bottom - window_rect->top;
  StretchDIBits(
    device_context,
    0, 0, bitmap_width, bitmap_height,
    0, 0, window_width, window_height, // destination coordinates
    bitmap_memory,
    &bitmap_info,
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
      RECT client_rect;

      GetClientRect(window, &client_rect);
      int width = client_rect.right - client_rect.left;
      int height = client_rect.bottom - client_rect.top;

      os_win32_resize_dib_section(width, height);

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

      os_win32_update_window(device_context, &client_rect, x, y, width, height);
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
          render_weird_gradient(x_offset, y_offset);

          ++x_offset;
        } // draw

        // NOTE jfd: blit to screen
        HDC device_context;
        defer_loop(device_context = GetDC(window_handle), ReleaseDC(window_handle, device_context)) {
          RECT client_rect;
          GetClientRect(window_handle, &client_rect);
          int window_width = client_rect.right - client_rect.left;
          int window_height = client_rect.bottom - client_rect.top;

          os_win32_update_window(device_context, &client_rect, 0, 0, window_width, window_height);
        }
      }

    } else {
    }

  } else {
    // TODO jfd: logging
  }

  return 0;
}
