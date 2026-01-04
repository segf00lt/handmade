#include "base.h"

/*
 * headers
 */

LRESULT CALLBACK os_win32_main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
void os_win32_resize_dib_section(int width, int height);
void os_win32_update_window(HDC device_context, int x, int y, int width, int height);

/*
 * globals
 */

global b32 app_is_running;
global BITMAPINFO bitmap_info;
global void *bitmap_memory;
global HBITMAP bitmap_handle;
global HDC bitmap_device_context;

/*
 * functions
 */

void os_win32_resize_dib_section(int width, int height) {
  // TODO jfd: bulletproof this
  // maybe don't free first, free after, then free first if that fails

  // TODO jfd: free our DIBSection

  if(bitmap_handle) {
    DeleteObject((HGDIOBJ)bitmap_handle);
  } else {
    bitmap_device_context = CreateCompatibleDC(0);
  }

  bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
  bitmap_info.bmiHeader.biWidth = width;
  bitmap_info.bmiHeader.biHeight = height;
  bitmap_info.bmiHeader.biPlanes = 1;
  bitmap_info.bmiHeader.biBitCount = 32;
  bitmap_info.bmiHeader.biCompression = BI_RGB; // pixel format

  bitmap_handle = CreateDIBSection(
    bitmap_device_context,
    &bitmap_info,
    DIB_RGB_COLORS,
    &bitmap_memory,
    0,
    0
  );

}

void os_win32_update_window(HDC device_context, int x, int y, int width, int height) {

  StretchDIBits(
    device_context,
    x, y, width, height,
    x, y, width, height,
    &bitmap_memory,
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
      PatBlt(device_context, x, y, width, height, color);
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

      MSG msg;
      while(app_is_running) {
        BOOL message_result = GetMessage(&msg, NULL, 0, 0);

        if(message_result > 0) {
          TranslateMessage(&msg); // NOTE jfd: has to do with keyboard messages
          DispatchMessage(&msg);
        } else {
          break;
        }

      }

    } else {
    }

  } else {
    // TODO jfd: logging
  }

  return 0;
}
