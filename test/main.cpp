#include <polygl.h>
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
  pdev_create(ctx,0);

  // initalize window
  pwin_create(ctx,800,600,1);

  // initalize shader pipeline
  poly_pipe_t *pipe;
  ppipe_create(ctx,&pipe,"shaders/simple_shader.vert.spv","shaders/simple_shader.frag.spv");

  // initalize renderer
  poly_rctx_t *rctx;
  uint32_t maxFrames = 2;
  prctx_create(&rctx,pipe,ctx,maxFrames);

  // main
  uint32_t currentFrame = 0;
  while (!pwin_should_close(ctx)) {
    pwin_set_ctx(ctx);
    pwin_poll();
    poly_aquire_frame(rctx,pipe,ctx,currentFrame);
    currentFrame = (currentFrame + 1) % maxFrames;
  }

  // idle before destroying objects
  pdev_idle(ctx); // ensures all device threads are done

  // destroy renderer
  prctx_delete(rctx,pipe,ctx);

  // destroy shader pipeline
  ppipe_delete(ctx,pipe);

  // destroy window
  pwin_delete(ctx);

  // destroy device interface
  pdev_delete(ctx);

  // de-initalize library
  poly_destroy(ctx);
}