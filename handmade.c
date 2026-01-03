#include <windows.h>

/*
typedef struct tagWNDCLASSA {
  UINT      style;
  WNDPROC   lpfnWndProc;
  int       cbClsExtra;
  int       cbWndExtra;
  HINSTANCE hInstance;
  HICON     hIcon;
  HCURSOR   hCursor;
  HBRUSH    hbrBackground;
  LPCSTR    lpszMenuName;
  LPCSTR    lpszClassName;
} WNDCLASSA, *PWNDCLASSA, *NPWNDCLASSA, *LPWNDCLASSA;
*/

LRESULT CALLBACK main_window_callback(
  HWND window,
  UINT message,
  WPARAM w_param,
  LPARAM l_param
) {
  LRESULT result = 0;

  switch(message) {
    case WM_SIZE: {
      OutputDebugStringA("WM_SIZE\n");
    } break;
    case WM_DESTROY: {
      OutputDebugStringA("WM_DESTROY\n");
    } break;
    case WM_CLOSE: {
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
      static color = WHITENESS;
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
  window_class.lpfnWndProc = main_window_callback;
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

      MSG msg;
      for(;;) {
        BOOL message_result = GetMessage(&msg, NULL, 0, 0);

        if(message_result > 0) {
          TranslateMessage(&msg); // NOTE ~jfd: has to do with keyboard messages
          DispatchMessage(&msg);
        } else {
          break;
        }

      }

    } else {
    }

  } else {
    // TODO ~jfd: logging
  }

  return 0;
}
