#include "polygl.h"
#include <windows.h>
#include <stdbool.h>
#include "src/log.h"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")


struct pwin_win32 {
  struct pwctx *class;

  HWND hwnd;
  int should_close_flag;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
      EndPaint(hwnd, &ps);
      return 0;
    }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

poly_err_t win32_create_window(struct pwin *voidwin, struct pwinconfig *conf) {
  struct pwin_win32 *win = (struct pwin_win32*)voidwin;

  HINSTANCE hInstance = GetModuleHandle(NULL);

  win->hwnd = CreateWindowEx(
    0,                            // style
    win->class->_classname,       // class window should be registered with
    conf->text,                   // window label
    WS_OVERLAPPEDWINDOW,          // type
    CW_USEDEFAULT, CW_USEDEFAULT, // x, y
    conf->width == 0 ? CW_USEDEFAULT : conf->width,   // width
    conf->height == 0 ? CW_USEDEFAULT : conf->height, // height
    NULL,                         // ?
    NULL,                         // ?
    hInstance,                    // instance
    NULL                          // ?
  );

  if (!win->hwnd) {
    return POLY_ERR_WINDOW_CREATION;
  }

  BOOL dark = TRUE; 
  DwmSetWindowAttribute(win->hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

  ShowWindow(win->hwnd, SW_SHOW);
  UpdateWindow(win->hwnd);

  return POLY_ERR_GOOD;
}

void win32_delete_window(struct pwin *voidwin) {
  struct pwin_win32 *win = (struct pwin_win32*)voidwin;
  DestroyWindow(win->hwnd);
}

poly_err_t win32_poll(struct pwctx *ctx) {
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return POLY_ERR_GOOD;
}

int win32_should_close(struct pwin *voidwin) {
  struct pwin_win32 *win = (struct pwin_win32*)voidwin;
  return !IsWindow(win->hwnd);
}

// defined in poly.c (internal api)
extern poly_err_t __allocate_space(struct pwctx *ctx);

poly_err_t pwctx_create_win32(struct pwctx *ctx, const char *_classname, size_t expected_window_count) {
  ctx->itemsize = sizeof(struct pwin_win32);
  ctx->total = expected_window_count;
  ctx->items = 0;
  ctx->buf = NULL;
  __allocate_space(ctx);

  ctx->_classname = _classname;
  ctx->_create = win32_create_window;
  ctx->_dlt = win32_delete_window;
  ctx->callback = NULL;
  ctx->poll = win32_poll;
  ctx->should_close = win32_should_close;

  HINSTANCE hInstance = GetModuleHandle(NULL);

  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);  // default icon
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);    // default cursor
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszClassName = _classname;

  if (!RegisterClassEx(&wc)) {
    return POLY_ERR_WINDOW_CLASS_CREATION;
  }

  return POLY_ERR_GOOD;
}
