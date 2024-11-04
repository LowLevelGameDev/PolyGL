#ifndef __POLYGL_IMPL
#define __POLYGL_IMPL

#ifndef POLY_API
  #define POLY_API
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct poly_ctx_t;
typedef struct poly_ctx_t poly_ctx_t;

struct poly_pipe_t;
typedef struct poly_pipe_t poly_pipe_t;

struct poly_rctx_t;
typedef struct poly_rctx_t poly_rctx_t;

// ------------ Error Handling ------------ //
  typedef size_t poly_error_t;
  #define POLY_GOOD 0
  #define POLY_INIT_INSTANCE_FAILED 1
  #define POLY_INIT_DEBUG_MESSENGER_FAILED 2
  #define POLY_INTERNAL_MALLOC_ERROR 3
  #define POLY_ERROR_VKCREATEDEVICE 4
  #define POLY_NO_PHYSICAL_DEVICES 5
  #define POLY_NO_SURFACE_SUPPORT 6
  #define POLY_FILE_NOT_FOUND 7
  const char *strpolyerr(poly_error_t);
// ------------ Initalizer && CleanUp Functions ------------ //
// init
  POLY_API poly_error_t poly_instance(
    poly_ctx_t **ctx,
    const char *ApplicationName,
    const char *EngineName,
    bool debug
  );
  POLY_API poly_error_t poly_destroy(poly_ctx_t *ctx);
// device
  POLY_API poly_error_t pdev_create(poly_ctx_t *ctx, int sort);
  POLY_API poly_error_t pdev_delete(poly_ctx_t *ctx);
  POLY_API poly_error_t pdev_idle(poly_ctx_t *ctx);
// window
  POLY_API poly_error_t pwin_create(poly_ctx_t *ctx, long Width, long Height, uint32_t ImageCount);
  POLY_API poly_error_t pwin_delete(poly_ctx_t *ctx);
// pipeline
  poly_error_t ppipe_create(
    poly_ctx_t *ctx, poly_pipe_t **pipe,
    const char *vertexShaderFileName, const char *fragmentShaderFileName
  );
  poly_error_t ppipe_delete(poly_ctx_t *ctx, poly_pipe_t *pipe);
// renderer
  POLY_API poly_error_t prctx_create(poly_rctx_t **rctx, poly_pipe_t *pipe, poly_ctx_t *ctx, uint32_t maxFrames);
  POLY_API poly_error_t prctx_delete(poly_rctx_t *rctx, poly_pipe_t *pipe, poly_ctx_t *ctx);
// ------------ Manipulation Functions ------------ //
// window
  POLY_API int pwin_should_close(poly_ctx_t *ctx);
  POLY_API poly_error_t pwin_set_ctx(poly_ctx_t *ctx);
  POLY_API poly_error_t pwin_poll();
// render
  void poly_aquire_frame(poly_rctx_t *rctx, poly_pipe_t *pipe, poly_ctx_t *ctx, uint32_t currentFrame);
// END

#ifdef __cplusplus
}
#endif
#endif // Header Guard