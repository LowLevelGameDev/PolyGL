#include "polygl.h"
#include <windows.h>
#include <stdbool.h>

struct pwin_win32 {
  struct pwctx *class;
  poly_err_t (*dlt)(struct pwin*);
  poly_err_t (*open)(struct pwin*, int height, int width);
  int        (*should_close)(struct pwin*);
  poly_err_t (*callback)(struct pwin*, poly_callback_type_t, void *callback);
  poly_err_t (*poll)(struct pwin*);

  // Add needed global variables here
  HWND hwnd;
  int should_close_flag;
};

// Forward declare the window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Implement the functions
void dlt(struct pwin *voidwin) {
  // Clean up resources
  struct pwin_win32 *win = (struct pwin_win32*)voidwin;
  if (win->hwnd != NULL) DestroyWindow(win->hwnd);
  win->hwnd = NULL;
}

poly_err_t open(struct pwin *voidwin, struct pwinconfig *conf) {
  struct pwin_win32 *win = (struct pwin_win32*)voidwin;

  if (win != NULL) {
    win->dlt(voidwin);
  }

  // Create the window
  HWND parent_hwnd = NULL;
  if (conf->parent != NULL) {
    parent_hwnd = ((struct pwin_win32*)conf->parent)->hwnd;
  }

  if (conf->width == 0) {
    conf->width = CW_USEDEFAULT;
  }
  if (conf->height == 0) {
    conf->height = CW_USEDEFAULT;
  }

  // Assuming you have allocated memory for char arrays
  win->hwnd = CreateWindowEx(
    0,                              // Optional window styles.
    win->class->_classname,         // Window class
    conf->text,                     // Window text
    WS_OVERLAPPEDWINDOW,            // Window style
    CW_USEDEFAULT, CW_USEDEFAULT,   // x, y
    conf->width, conf->height,      // width, height
    parent_hwnd,                    // Parent window    
    NULL,                           // Menu
    GetModuleHandle(NULL),          // Instance handle
    NULL                            // Additional application data
  );

  if (win->hwnd == NULL) {
    return POLY_ERR_WINDOW_CREATION_FAILURE;
  }

  ShowWindow(win->hwnd, SW_SHOW);

  win->should_close_flag = false;
  return 0;
}

poly_err_t callback(struct pwin *voidwin, poly_callback_type_t type, void *callback) {
  // Implement callback handling as needed
  return 0;
}

poly_err_t poll(struct pwin *voidwin) {
  struct pwin_win32 *win = (struct pwin_win32*)voidwin;

  MSG msg = {};
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      win->should_close_flag = true;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return 0;
}

int should_close(struct pwin *voidwin) {
  struct pwin_win32 *win = (struct pwin_win32*)voidwin;
  return win->should_close_flag;
}

// Define the window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
  case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

poly_err_t pwctx_create_win32  (struct pwctx *ctx, const char *_classname) {
  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;          // hwnd message callback
  wc.hInstance = GetModuleHandle(NULL); // what is this?
  wc.lpszClassName = _classname;        // class identifier

  RegisterClass(&wc);

  // set ctx info
  ctx->_classname = _classname;

  // set ctx buffer
  ctx->buffer = NULL;
  ctx->buffer_size = 0;
  ctx->item_size = sizeof(struct pwin_win32);

  // set ctx functions
  ctx->_create = open;
  ctx->_dlt = dlt;
  ctx->callback = callback;
  ctx->poll = poll;
  ctx->should_close = should_close;
  return POLY_ERR_GOOD;
}