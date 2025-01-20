#include "polygl.h"

#include <stdio.h>




int main() {
  struct pwctx ctx;
  poly_err_t err = pwctx_create_win32(&ctx, "Poly Examples", 2);
  if (err != POLY_ERR_GOOD) {
    printf("Err creating context: %s\n", strpolyerr(err));
  }

  size_t window_index;
  struct pwinconfig winconfig = {
    .width = 0, .height = 0,
    .text = "PolyGL Example",
    .parent = NULL
  };
  err = pwctx_create_window(&ctx, &window_index, &winconfig);
  if (err != POLY_ERR_GOOD) {
    printf("Err creating context: %s\n", strpolyerr(err));
  }

  size_t second_window_index;
  err = pwctx_create_window(&ctx, &second_window_index, &winconfig);
  if (err != POLY_ERR_GOOD) {
    printf("Err creating context: %s\n", strpolyerr(err));
  }

  struct pwin *window = pwctx_get_window(&ctx, window_index);
  struct pwin *second_window = pwctx_get_window(&ctx, second_window_index);

  // while (any window is open) {check for events}
  while (!pwctx_should_close(&ctx)) {
    // do whatever else your program needs to do

    // poll for events for all windows
    // (if windows are created on a seperate thread they may not be polled for.)
    // (this has not been tested but from my limited knowledge of the win32 window api...)
    // (multithreaded programs that opens windows in seperate threads should watch out...)
    // (for this weird behavior)
    ctx.poll(&ctx);
  }

  pwctx_delete(&ctx);
  return 0;
}
