#ifdef __cplusplus
 #pragma once
#endif

#ifndef __DEFINED_HEADER_GUARD_POLY_GL_MAIN__
#define __DEFINED_HEADER_GUARD_POLY_GL_MAIN__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> // size_t

// -------------------------------- Global API -------------------------------- //

typedef long int poly_err_t;
const char *strpolyerr(poly_err_t);
// DEFINES ERRORS
  #define POLY_ERR_WINDOW_CREATION_FAILURE -2
  #define POLY_ERR_MEMORY_ALLOCATION_FAILURE -1
  #define POLY_ERR_GOOD 0
  #define POLY_ERR_WARNING 1
// END

// -------------------------------- Window API -------------------------------- //

typedef long int poly_window_api_t;
// DEFINES WINDOW APIS
  #define POLY_WINDOW_API_WIN32 0
  #define POLY_WINDOW_API_WIN64 1
  #define POLY_WINDOW_API_X11 2
  #define POLY_WINDOW_API_WAYLAND 3
  #define POLY_WINDOW_API_COCOA 4
// END

typedef long int poly_key_t;
// DEFINES KEYBOARD SCANCODES
  // a-z
  // A-Z
  // 0-9
  // symbols
  // specials (windows, caps, etc...)
// END

typedef long int poly_mouse_t;
// DEFINES MOUSE CODES
  #define POLY_MOUSE_LEFT_CLICK 0
  #define POLY_MOUSE_RIGHT_CLICK 1
  #define POLY_MOUSE_MIDDLE_CLICK 2
  #define POLY_MOUSE_SIDE_BUTTON_4 3
  #define POLY_MOUSE_SIDE_BUTTON_5 4
  #define POLY_MOUSE_SCROLL_UP 5
  #define POLY_MOUSE_SCROLL_DOWN 6
// END

typedef long int poly_callback_type_t;
// DEFINES CALLBACK TYPES
  #define POLY_CALLBACK_KB 0
  #define POLY_CALLBACK_MOUSE 1
  #define POLY_CALLBACK_WINDOW_RESIZE 2
  #define POLY_CALLBACK_WINDOW_MOVE 3
// END

// poly window
struct pwin {
  struct pwctx *class;
  poly_err_t (*dlt)(struct pwin*);
  poly_err_t (*open)(struct pwin*, int height, int width);
  int        (*should_close)(struct pwin*);
  poly_err_t (*callback)(struct pwin*, poly_callback_type_t, void *callback);
  poly_err_t (*poll)(struct pwin*);
};
typedef struct pwin pwin_t;

struct pwinconfig {
  int width; int height;
  const char *text;
  struct pwin *parent;
};

// An easy multiple pwin manager to control multiple windows with a clean api
struct pwctx {
  size_t buffer_size; // complete buffer size
  size_t item_size;   // pwin item size
  size_t items; // current items in buffer
  char *buffer;

  // context info
  const char *_classname;

  // window creation
  poly_err_t (*_create)(struct pwin*, struct pwinconfig*);
  void       (*_dlt)(struct pwin*);

  // window manipulation
  poly_err_t (*callback)(struct pwin*, poly_callback_type_t, void *callback);
  poly_err_t (*poll)(struct pwin*);
  int        (*should_close)(struct pwin*);
};
typedef struct pwctx pwctx_t;

// context manipulation
void       pwctx_gdlt(struct pwctx*); // delete all windows
poly_err_t pwctx_gpoll(struct pwctx*); // poll all windows
poly_err_t pwctx_gcallback(struct pwctx*, poly_callback_type_t, void *callback); // set a callback for all windows
poly_err_t pwctx_set_buffer(struct pwctx*, size_t byte_size);

poly_err_t pwctx_create_window(struct pwctx*, size_t*, struct pwinconfig*);
poly_err_t pwctx_create_delete(struct pwctx*, size_t index);

struct pwin *pwctx_get_window(struct pwctx*, size_t index);

// context creation
poly_err_t pwctx_create_win32  (struct pwctx *ctx, const char *_classname);
poly_err_t pwctx_create_wayland(struct pwctx *ctx, const char *_classname);
poly_err_t pwctx_create_x11    (struct pwctx *ctx, const char *_classname);
poly_err_t pwctx_create_cocoa  (struct pwctx *ctx, const char *_classname);

// context deletion
void       pwctx_delete(struct pwctx*);

// -------------------------------- Graphics API -------------------------------- //

typedef long int pwin_graphics_backend_t;
// DEFINES BACKEND
  #define POLY_GRAPHICS_OPENGL 0
  #define POLY_GRAPHICS_VULKAN 1
  // DirectX* and windows GDI may be implemented when i lose my mind
// END

struct pgraphics {
  struct pg_device *devices; // list of available devices
  struct pg_logical_device *device; // current rendering device

  struct pg_pipe *pipes;
  struct pg_command *commands;

  void (*sync)(struct pgraphics*); // wait for gpu to complete current actions
  void (*aquire_frame)(struct pgraphics*); // aquire animation frame
};

size_t pg_sizeof_opengl(); poly_err_t pg_create_opengl(struct pgraphics *ctx);
size_t pg_sizeof_vulkan(); poly_err_t pg_create_vulkan(struct pgraphics *ctx);

// -------------------------------- GUI API -------------------------------- //


#ifdef __cplusplus
}
#endif
#endif // __DEFINED_HEADER_GUARD_POLY_GL_MAIN__


