/*
* Implementes the global api of polygl
* 
*
*/
#include "polygl.h"
#include "src/log.h"

#include <stdlib.h>
#include <string.h>

// -------------------------------- Internal API -------------------------------- //
poly_err_t __allocate_space(struct pwctx *ctx) {
  if (ctx->buf == NULL) {
    ctx->buf = (char*)malloc(ctx->total * ctx->itemsize);
    if (ctx->buf == NULL) return POLY_ERR_MEMORY_ALLOCATION;
  } else {
    ctx->total *= 2;
    ctx->buf = (char*)realloc(ctx->buf, ctx->total * ctx->itemsize);
    if (ctx->buf == NULL) return POLY_ERR_MEMORY_ALLOCATION;
  }
  return POLY_ERR_GOOD;
}

// -------------------------------- Global API -------------------------------- //
const char *errmap[] = {
  "Everything is fine",
  "Internal Memory Allocation Failure",
  "Class Creation Failure",
  "Window Creation Failure",
  "Out of Bounds 0",
  "Out of Bounds 1",
  "Out of Bounds 2"
};
const char *strpolyerr(poly_err_t err) {
  return errmap[err];
}

// -------------------------------- Window API -------------------------------- //
void pwctx_gdlt(struct pwctx *ctx) { 
  for (size_t i = 0; i < ctx->items; ++i) {
    ctx->_dlt((struct pwin*)&ctx->buf[i * ctx->itemsize]);
  }
  ctx->items = 0;
}

int pwctx_should_close(struct pwctx *ctx) {
  for (size_t i = 0; i < ctx->items; ++i) {
    // check if even a single window isn't closed and stop program from closing
    // if all windows are closed then return true
    if (!ctx->should_close((struct pwin*)&ctx->buf[i * ctx->itemsize])) {
      return 0;
    }
  }
  return 1;
}

poly_err_t pwctx_gcallback(struct pwctx *ctx, poly_callback_type_t type, void *callback) { 
  poly_err_t err = POLY_ERR_GOOD;
  for (size_t i = 0; i < ctx->items; ++i) {
    err = ctx->callback((struct pwin*)&ctx->buf[i * ctx->itemsize], type, callback);
    if (err < 0) {
      return err;
    }
  }
  return err;
}

poly_err_t pwctx_create_window(struct pwctx *ctx, size_t *index, struct pwinconfig *conf) {
  if (ctx->total <= ctx->items) {
    if (__allocate_space(ctx) != POLY_ERR_GOOD)
      return POLY_ERR_MEMORY_ALLOCATION;
  }

  struct pwin *window = pwctx_get_window(ctx, ctx->items);
  window->class = ctx;
  poly_err_t err = ctx->_create(window, conf);
  
  *index = ctx->items;
  ++ctx->items;
  return err;
}

void pwctx_delete_window(struct pwctx *ctx, size_t index) {
  struct pwin *window = pwctx_get_window(ctx, index);
  ctx->_dlt(window);

  if (index == ctx->items) {
    --ctx->items;
    return;
  }

  memmove(
    window, pwctx_get_window(ctx, index + 1), 
    (ctx->items - index) * ctx->itemsize
  );
}

struct pwin *pwctx_get_window(struct pwctx* ctx, size_t index) {
  return (struct pwin*)(&ctx->buf[index * ctx->itemsize]);
}

void pwctx_delete(struct pwctx* ctx) {
  if (ctx->items != 0) {
    pwctx_gdlt(ctx);
  }
  if (ctx->buf != NULL) {
    free(ctx->buf);
    ctx->buf = NULL;
  }
}
