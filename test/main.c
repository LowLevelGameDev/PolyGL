#include "polygl.h"
#include <stdio.h>

int main() {
  poly_error_t err;
  
  // initalize library
  poly_ctx_t *ctx;
  err = poly_instance(&ctx, "PolyGL Testing", "No Engine", true);
  if (err != POLY_GOOD) {
    printf("%s    ", strpolyerr(err));
    poly_destroy(ctx);
    return 1;
  }

  // select device and create interface

  // initalize window

  // main
  

  // destroy window

  // destroy device interface

  // de-initalize library
  poly_destroy(ctx);
}