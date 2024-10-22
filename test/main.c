#include "polygl.h"

int main() {
  // initalize library
  poly_init();

  // open context
  POLY_CTX *ctx = poly_create_ctx(400, 600, "Welcome to Poly Graphics");

  int status = poly_graphic_pipeline(ctx,"shaders/simple_shader.vert.spv","shaders/simple_shader.frag.spv");

  // main context loop
  while (!poly_ctx_should_close(ctx)) {
    poly_poll();
  }

  // close context
  poly_delete_ctx(ctx);

  // de-initalize library
  poly_deinit();
}