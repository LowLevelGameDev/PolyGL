#ifndef __POLYGL_IMPL
#define __POLYGL_IMPL

#ifndef POLY_API
  #define POLY_API
#endif

struct poly_ctx_t;
typedef struct poly_ctx_t poly_ctx_t;

struct poly_queue_t;
typedef struct poly_queue_t poly_queue_t;

struct poly_swapchain_t;
typedef struct poly_swapchain_t poly_swapchain_t;

struct poly_render_pass_t;
struct poly_pipe_t;
typedef struct poly_pipe_t poly_pipe_t;

struct poly_commandpool_t;
typedef struct poly_commandpool_t poly_commandpool_t;

struct poly_host_memory_t;
struct poly_device_memory_t;
struct poly_command_buffer_t;
struct poly_descriptor_pool_t;

struct poly_signal_t;
struct poly_fence_t;

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// error handling
  typedef size_t poly_error_t;
  #define POLY_GOOD 0
  #define POLY_INIT_INSTANCE_FAILED 1
  #define POLY_INIT_DEBUG_MESSENGER_FAILED 2
  #define POLY_INTERNAL_MALLOC_ERROR 3
  #define POLY_ERROR_VKCREATEDEVICE 4
  const char *strpolyerr(poly_error_t);
// init
  POLY_API poly_error_t poly_instance(
    poly_ctx_t **ctx,
    const char *ApplicationName,
    const char *EngineName,
    bool debug
  );
  POLY_API poly_error_t poly_destroy(poly_ctx_t *ctx);
// window init
  POLY_API poly_error_t poly_win_create(poly_ctx_t *ctx, long Width, long Height);
  POLY_API poly_error_t poly_win_delete(poly_ctx_t *ctx);
// device init
  POLY_API poly_error_t poly_dev_create(poly_ctx_t *ctx, int sort);
  POLY_API poly_error_t poly_dev_delete(poly_ctx_t *ctx);
// END

#endif