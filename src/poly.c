/*
* Implementes the global api of polygl
* 
*
*/
#include "polygl.h"

#include <stdlib.h>
#include <string.h>

const char *errmap[] = {
  "Window Creation Failure",
  "Internal Memory Allocation Failure",
  "Everything is fine", // GOOD offset 2
  "WARNING"
};
const char **errmap_offset = &errmap[2]; // offset of good to support negative values

const char *strpolyerr(poly_err_t err) {
  return errmap_offset[err];
}

// -------------------------------- Window API -------------------------------- //
void pwctx_gdlt(struct pwctx *ctx) { 
  for (size_t i = 0; i < ctx->items; ++i) {
    ctx->_dlt((struct pwin*)&ctx->buffer[i * ctx->item_size]);
  }
  ctx->items = 0;
}

poly_err_t pwctx_gpoll(struct pwctx *ctx) { 
  poly_err_t err = POLY_ERR_GOOD;
  for (size_t i = 0; i < ctx->items; ++i) {
    err = ctx->poll((struct pwin*)&ctx->buffer[i * ctx->item_size]);
    if (err < 0) {
      return err;
    }
  }
  return err;
}

poly_err_t pwctx_gcallback(struct pwctx *ctx, poly_callback_type_t type, void *callback) { 
  poly_err_t err = POLY_ERR_GOOD;
  for (size_t i = 0; i < ctx->items; ++i) {
    err = ctx->callback((struct pwin*)&ctx->buffer[i * ctx->item_size], type, callback);
    if (err < 0) {
      return err;
    }
  }
  return err;
}

poly_err_t pwctx_set_buffer(struct pwctx *ctx, size_t byte_size) {
  if (ctx->buffer == NULL) {
    ctx->buffer = (char*)malloc(byte_size);
    if (ctx->buffer == NULL) return POLY_ERR_MEMORY_ALLOCATION_FAILURE;
    ctx->buffer_size = byte_size;
  } else {
    char *temp_buffer = (char*)realloc(ctx->buffer, byte_size);
    if (temp_buffer == NULL) return POLY_ERR_MEMORY_ALLOCATION_FAILURE;
    ctx->buffer = temp_buffer;
    ctx->buffer_size = byte_size;
  }
  return POLY_ERR_GOOD;
}

poly_err_t pwctx_create_window(struct pwctx *ctx, size_t *index, struct pwinconfig *conf) {
  size_t buffer_size_left = ctx->buffer_size - ctx->items * ctx->item_size;
  if (buffer_size_left < ctx->item_size) {
    poly_err_t err = pwctx_set_buffer(ctx, ctx->buffer_size * 2);
    if (err != POLY_ERR_GOOD) return err;
  }

  struct pwin *window = pwctx_get_window(ctx, ctx->items);
  poly_err_t err = ctx->_create(window, conf);
  window->class = ctx;
  *index = ctx->items;
  ++ctx->items;
  return err;
}

void pwctx_delete_window(struct pwctx *ctx, size_t index) {
  struct pwin *window = pwctx_get_window(ctx, index);
  ctx->_dlt(window);

  if (ctx->items > 1 && index < ctx->items - 1) {
    memmove(window, pwctx_get_window(ctx, index + 1), (ctx->items - (index + 1)) * ctx->item_size);
  }
  --ctx->items;
}

struct pwin *pwctx_get_window(struct pwctx* ctx, size_t index) {
  return (struct pwin*)(&ctx->buffer[ctx->item_size * index]);
}

void pwctx_delete(struct pwctx* ctx) {
  if (ctx->items != 0) {
    pwctx_gdlt(ctx);
  }
  if (ctx->buffer != NULL) {
    free(ctx->buffer);
    ctx->buffer = NULL;
  }
}
