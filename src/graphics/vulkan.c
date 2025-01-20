#include "polygl.h"

struct pgraphics_vulkan {
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


size_t pg_sizeof_vulkan() {
  return sizeof(pgraphics_vulkan)
}

poly_err_t pg_create_vulkan(struct pgraphics *voidctx) {
  struct pgraphics_vulkan *ctx = (struct pgraphics_vulkan*)voidctx;

  // assign functions
  ctx->get_physical_devices = vulkan_get_physical_devices;

  ctx->create_logical_device = vulkan_create_logical_device;

  ctx->attach_to_surface_win32 = vulkan_attach_to_surface_win32;
  ctx->attach_to_surface_wayland = vulkan_attach_to_surface_wayland;
  ctx->attach_to_surface_x11 = vulkan_attach_to_surface_x11;
  ctx->attach_to_surface_cocoa = vulkan_attach_to_surface_cocoa;

  // assign default data

  // create vk instance


  // enumerate physical devices


  return POLY_ERR_GOOD;
}
void pg_delete_vulkan(struct pgraphics *ctx) {

}