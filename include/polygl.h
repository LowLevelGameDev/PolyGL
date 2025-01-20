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
  #define POLY_ERR_GOOD 0
  #define POLY_ERR_MEMORY_ALLOCATION 1
  #define POLY_ERR_WINDOW_CLASS_CREATION 2
  #define POLY_ERR_WINDOW_CREATION 3  
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
struct pwin { // real defenitions are in src/window/*.c
  struct pwctx *class;
};
typedef struct pwin pwin_t;

struct pwinconfig {
  int width; int height;
  const char *text;
  struct pwin *parent;
};
typedef struct pwinconfig pwinconfig_t;

// An easy multiple pwin manager to control multiple windows with a clean api
struct pwctx {
  // context info (could be needed for apis but also could be useful for debugging by organising windows into classes)
  const char *_classname;

  // window buffer
  char *buf;
  size_t total;
  size_t items;
  size_t itemsize;

  // window creation (intended for internal use)
  poly_err_t (*_create)(struct pwin*, struct pwinconfig*);
  void       (*_dlt)(struct pwin*);

  // window manipulation (user use)
  poly_err_t (*callback)(struct pwin*, poly_callback_type_t, void *callback);
  poly_err_t (*poll)(struct pwctx*);
  int        (*should_close)(struct pwin*);
};
typedef struct pwctx pwctx_t;

// context manipulation
void       pwctx_gdlt(struct pwctx*); // delete all windows
poly_err_t pwctx_gcallback(struct pwctx*, poly_callback_type_t, void *callback); // set a callback for all windows
int        pwctx_should_close(struct pwctx*); // checks if at least on window is still running and returns 1, if no windows are running 0

poly_err_t pwctx_create_window(struct pwctx*, size_t*, struct pwinconfig*);
poly_err_t pwctx_create_delete(struct pwctx*, size_t index);

struct pwin *pwctx_get_window(struct pwctx*, size_t index);

// context creation
poly_err_t pwctx_create_win32  (struct pwctx *ctx, const char *_classname, size_t expected_window_count);
poly_err_t pwctx_create_wayland(struct pwctx *ctx, const char *_classname, size_t expected_window_count);
poly_err_t pwctx_create_x11    (struct pwctx *ctx, const char *_classname, size_t expected_window_count);
poly_err_t pwctx_create_cocoa  (struct pwctx *ctx, const char *_classname, size_t expected_window_count);

// context deletion
void       pwctx_delete(struct pwctx*);

// -------------------------------- Graphics API -------------------------------- //

typedef long int pwin_graphics_backend_t;
// DEFINES BACKEND
  #define POLY_GRAPHICS_OPENGL 0
  #define POLY_GRAPHICS_VULKAN 1
  // DirectX* and windows GDI may be implemented when i lose my mind
// END

struct pg_physical_device {
  // --- identity device info --- //
  const char *deviceName;       // human readable device name
  size_t vendorId;              // 0 if not available
  size_t deviceId;              // 0 if not available
  size_t driverVersion;         // device driver version
  int deviceType;               // 0 not available, 1: integrated, 2: discrete, 3: virtual, 4: CPU

  // --- broad device capabilites --- //
  size_t totalMemory;          // Total memory available on the device
  size_t maxMemoryAllocation;  // Maximum memory allocation size
  size_t apiVersion;           // Version of the graphics API supported by the device
  size_t maxImageDimension2D;  // Maximum 2D image dimension supported
  size_t maxImageDimension3D;  // Maximum 3D image dimension supported

  // --- flags --- //
  int supportsGeometryShader;     // -1 if not set,  0 if geometry shader is supported,     1 if geometry shader is not supported
  int supportsTessellationShader; // -1 if not set,  0 if tessellation shader is supported, 1 if tessellation shader is not supported
  int supportsComputeShader;      // -1 if not set,  0 if compute shader is supported,      1 if compute shader is not supported
  int supportsRayTracing;         // -1 if not set,  0 if ray tracing is supported,         1 if ray tracing is not supported
  int supportsMeshShader;         // -1 if not set,  0 if mesh shader is supported,         1 if mesh shader is not supported

  // TODO: Probably add more
  // TODO: Extensions need to be supported
};


struct pgraphics {
  // select physical device
  poly_err_t (*get_physical_devices)(struct pgraphics*, struct pg_physical_device**);

  // create logical device
  // * a logical device is part of the internal api used by the backend graphics api
  poly_err_t (*create_logical_device)(struct pgraphics*, struct pg_physical_device*);

  // surface
  // * annoyingly os dependent but should be the only section that is
  poly_err_t (*attach_to_surface_win32)(struct pgraphics*);
  poly_err_t (*attach_to_surface_wayland)(struct pgraphics*);
  poly_err_t (*attach_to_surface_x11)(struct pgraphics*);
  poly_err_t (*attach_to_surface_cocoa)(struct pgraphics*);

  // shader pipeline
  // * vert and frag pipeline creation etc...

  // command buffers
  // * record a section of commands
  // * (i believe opengl doesn't support this but you will still use a command buffer...)
  // * (it will just have to be translated at runtime)
  // * (a sacrifice for speed will unfortunately have to be made for these legacy APIs)

  // synchronization
  // synchronize the device
  // * (i believe opengl also hides away sync functionality so this will probably just be ignored for opengl)
  // * (another small sacrifice for a cross graphics api api)

  // data
  struct pg_device *devices; // list of available devices
  struct pg_logical_device *device; // current rendering device

  struct pg_pipe *pipes;
  struct pg_command *commands;

  // flags
  int _debug; // i.e. vulkan validation layers will be used on true
};

// initalization
// Open GL
size_t     pg_sizeof_opengl(); 
poly_err_t pg_create_opengl(struct pgraphics *ctx);
void       pg_delete_opengl(struct pgraphics *ctx);

// Vulkan
size_t     pg_sizeof_vulkan(); 
poly_err_t pg_create_vulkan(struct pgraphics *ctx);
void       pg_delete_vulkan(struct pgraphics *ctx);


// -------------------------------- GUI API -------------------------------- //


#ifdef __cplusplus
}
#endif
#endif // __DEFINED_HEADER_GUARD_POLY_GL_MAIN__


