#include <../include/polygl.h>
#include <string.h>

#define MEMORY_ERROR -1

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define CTX(ctx) ((GLFWwindow*)ctx)

#include <stdlib.h>
#include <stdio.h>
#define LOG(str, ...) printf("INFO: "str"\n", __VA_ARGS__)
#define ERR(str, ...) fprintf(stderr,"ERROR: "str"\n", __VA_ARGS__)

#include "vulkan_device_setup.h"

VulkanCtx globalVulcanCtx;

// init
  void poly_init() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // for vulcan
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    vulkanInit(&globalVulcanCtx);
  }
  void poly_deinit() {
    vulkanTerminate(&globalVulcanCtx);
    glfwTerminate();
  }
// context init
  POLY_CTX *poly_create_ctx(int height, int width, const char *name) {
    return (void*)glfwCreateWindow(width, height, name, NULL, NULL);
  }
  void poly_delete_ctx(POLY_CTX *ctx) { glfwDestroyWindow(CTX(ctx)); }
// context manipulation
  bool poly_ctx_should_close(POLY_CTX *ctx) { return glfwWindowShouldClose(CTX(ctx)); }
  void poly_ctx_close(POLY_CTX *ctx) { glfwSetWindowShouldClose(CTX(ctx),true); }
// shaders
  int poly_graphic_pipeline(POLY_CTX *ctx, const char *vertFilepath, const char *fragFilepath) {
    FILE *vertFile = fopen(vertFilepath, "rb");
    if (vertFile == NULL) {
      ERR("vert file path caused an error, %s", vertFilepath);
      return 1;
    }
    FILE *fragFile = fopen(fragFilepath, "rb");
    if (fragFile == NULL) {
      ERR("frag file path caused an error, %s", fragFilepath);
      return 2;
    }

    fseek(vertFile, 0, SEEK_END);
    fseek(fragFile, 0, SEEK_END);

    size_t vertSize = ftell(vertFile);
    size_t fragSize = ftell(fragFile);

    fseek(vertFile, 0, SEEK_SET);
    fseek(fragFile, 0, SEEK_SET);

    char *vertBuffer = malloc(vertSize);
    if (vertBuffer == NULL) {
      ERR("vert File Buffer Couldn't be Allocated for size %zu", vertSize);
      return 3;
    }
    char *fragBuffer = malloc(fragSize);
    if (fragBuffer == NULL) {
      ERR("frag File Buffer Couldn't be Allocated for size %zu", fragSize);
      return 4;
    }

    fread(vertBuffer, vertSize, 1, vertFile);
    fread(fragBuffer, fragSize, 1, fragFile);

    LOG("size of vert file : %zu", vertSize);
    LOG("size of frag file : %zu", fragSize);

    free(vertBuffer);
    free(fragBuffer);
    
    fclose(vertFile);
    fclose(fragFile);
    return 0;
  }
// drawing 2d
// drawing 3d
// handlers
  void poly_poll() { glfwPollEvents(); }
// end