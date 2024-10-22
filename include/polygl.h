
#ifndef POLY_API
  #define POLY_API
#endif

#define POLY_CTX void

#include <stdbool.h>

// init
  POLY_API void poly_init();
  POLY_API void poly_deinit();
// context init
  POLY_API POLY_CTX *poly_create_ctx(int height, int width, const char *name);
  POLY_API void poly_delete_ctx(POLY_CTX *ctx);
// context manipulation
  POLY_API bool poly_ctx_should_close(POLY_CTX *ctx); // check if close instruction has been sent
  POLY_API void poly_ctx_close(POLY_CTX *ctx); // send close instruction to window
// shaders
  POLY_API int poly_graphic_pipeline(POLY_CTX *ctx, const char *vertFilepath, const char *fragFilepath);
// drawing 2d
// drawing 3d
// handlers
  POLY_API void poly_poll();
// end

