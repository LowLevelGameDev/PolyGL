#include "polygl.h"

int main() {
  struct pwctx ctx;
  pwctx_create_win32(&ctx, "Poly Examples");

  size_t window_index;
  struct pwinconfig winconfig = {
    .width = 0, .height = 0,
    .text = "PolyGL Example",
    .parent = NULL
  };
  pwctx_create_window(&ctx, &window_index, &winconfig);

  struct pwin *window = pwctx_get_window(&ctx, window_index);

  while (!ctx.should_close(window)) {
    ctx.poll(window);
  }

  pwctx_delete(&ctx);
  return 0;
}