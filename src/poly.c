#include "../include/polygl.h"

#include "polyvk.h"

#define LOGW(str) printf("INFO: " str "\n");
#define LOG(str, ...) printf("INFO: " str "\n", __VA_ARGS__)
#define WARNW(str) printf("WARNING: " str "\n");
#define WARN(str, ...) printf("WARNING: " str "\n", __VA_ARGS__)
#define ERRW(str) fprintf(stderr,"ERROR: " str "\n");
#define ERR(str, ...) fprintf(stderr,"ERROR: " str "\n", __VA_ARGS__)

// --------- Static Data --------- //
char const* validationLayers[] = {
  "VK_LAYER_KHRONOS_validation"
};
uint32_t validationLayersSize = sizeof(validationLayers) / sizeof(char*);
char const* deviceExtensions[] = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
uint32_t deviceExtensionsSize = sizeof(deviceExtensions) / sizeof(char*);

// --------- Helper --------- //
const char *poly_error_code_map[] = {
  "Good!",
  "Failed to create a polygl instance!",
  "Failed to set up debug messenger!",
  "Failed to allocate memory!",
  "Failed to create logical device!",
  "Failed to find any physical devices!",
  "vulkan surface not supported!",
  "failed to open specified file!"
};
const char *strpolyerr(poly_error_t code) {
  return poly_error_code_map[code];
}

// --------- Internal Structures --------- //
typedef GLFWwindow* poly_window_t;
struct poly_ctx_t {
  // inital
  VkInstance instance;
  VkDebugUtilsMessengerEXT debug; // ?= NULL
  // device
  uint32_t physicalDeviceCount;
  VkPhysicalDevice *physicalDevices;
  VkPhysicalDevice *bestPhysicalDevice; // needs to be updated for device groups
  VkDevice device; // ?= VkDeviceGroup

  uint32_t bestGraphicsQueueFamilyindex;
  uint32_t graphicsQueueMode;
  VkQueue drawingQueue;
  VkQueue presentingQueue;

  // window & surface
  poly_window_t window;
  VkSurfaceKHR surface;
  VkExtent2D bestSwapchainExtent;
  VkSwapchainKHR swapchain;
  VkRenderPass renderPass;
  uint32_t imageCount;
  uint32_t swapchainImageCount;
  VkImage *swapchainImages;
  VkImageView *swapchainImageViews;
  VkFramebuffer *framebuffers;
};

struct poly_rctx_t {
  // command pool
  VkCommandPool commandPool;
  VkCommandBuffer *commandBuffers;

  // synchronize
  uint32_t maxFrames;
  VkSemaphore *waitSemaphores;
  VkSemaphore *signalSemaphores;
  VkFence *frontFences;
  VkFence *backFences;
};

struct poly_pipe_t {
  VkPipelineLayout layout;
  VkPipeline pipe;
};


// ------------ Initalizer && CleanUp Functions ------------ //
// init
  VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    ERR("validation layer -> %s", pCallbackData->pMessage);
    return VK_FALSE;
  }
  VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
      return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }
  void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
      func(instance, debugMessenger, pAllocator);
    }
  }

  bool __poly_internal_checkValidationLayerSupport() {
      uint32_t layerCount;
      vkEnumerateInstanceLayerProperties(&layerCount, NULL);

      VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(layerCount * sizeof(VkLayerProperties));
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

      for (uint32_t i = 0; i < validationLayersSize; ++i) {
        bool layerFound = false;
        for (uint32_t j = 0; j < layerCount; ++j) {
          if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
            layerFound = true;
            break;
          }
        }
        if (!layerFound) {
          free(availableLayers);
          return false;
        }
      }

      free(availableLayers);
      return true;
  }
  poly_error_t poly_instance(
    poly_ctx_t **p_ctx, 
    const char *ApplicationName,
    const char *EngineName,
    bool debug
  ) {
    glfwInit();
    
    // vulkan boiler plate
    poly_ctx_t *ctx = malloc(sizeof(poly_ctx_t));    
    if (debug && !__poly_internal_checkValidationLayerSupport()) { // check if debug is selected but no validation layer support
      WARNW("Debug mode selected but no validation layer support");
      LOGW("Turning off debug mode")
      debug = false;
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = ApplicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = EngineName;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    const char** extensions = (const char**)malloc((glfwExtensionCount + 1) * sizeof(const char*)); 
    memcpy(extensions, glfwExtensions, glfwExtensionCount * sizeof(const char*)); 
    if (debug) { 
      extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME; 
      glfwExtensionCount++;
    }

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = extensions;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
    if (debug) {
      createInfo.enabledLayerCount = validationLayersSize;
      createInfo.ppEnabledLayerNames = validationLayers;

      debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debugCreateInfo.pfnUserCallback = debugCallback;
      createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
      createInfo.enabledLayerCount = 0;
      createInfo.pNext = NULL;
      ctx->debug = VK_NULL_HANDLE;
    }

    if (vkCreateInstance(&createInfo, NULL, &ctx->instance) != VK_SUCCESS) {
      free(ctx);
      return POLY_INIT_INSTANCE_FAILED;
    }

    // clean up
    free(extensions);
    *p_ctx = ctx;

    // extra vulkan boiler plate (setup debug messenger)
    if (debug) { 
      VkDebugUtilsMessengerCreateInfoEXT debugMsgCreateInfo = {};
      debugMsgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debugMsgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debugMsgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debugMsgCreateInfo.pfnUserCallback = debugCallback;

      if (CreateDebugUtilsMessengerEXT(ctx->instance, &debugMsgCreateInfo, NULL, &ctx->debug) != VK_SUCCESS) {
        return POLY_INIT_DEBUG_MESSENGER_FAILED;
      }
    }

    return POLY_GOOD;
  }
  poly_error_t poly_destroy(poly_ctx_t *ctx) {
    if (ctx->debug != NULL) {
      DestroyDebugUtilsMessengerEXT(ctx->instance, ctx->debug, NULL);
    }
    vkDestroyInstance(ctx->instance, NULL);
    glfwTerminate();
    free(ctx);
    return POLY_GOOD;
  }
// device
  // device selection
    uint32_t getPhysicalDeviceTotalMemory(VkPhysicalDeviceMemoryProperties *pPhysicalDeviceMemoryProperties){
      uint32_t physicalDeviceTotalMemory = 0;
      for(int i = 0; i < pPhysicalDeviceMemoryProperties->memoryHeapCount; ++i){
        if((pPhysicalDeviceMemoryProperties->memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0){
          physicalDeviceTotalMemory += pPhysicalDeviceMemoryProperties->memoryHeaps[i].size;
        }
      }
      return physicalDeviceTotalMemory;
    }
    uint32_t getBestPhysicalDeviceIndex(VkPhysicalDevice *pPhysicalDevices, uint32_t physicalDeviceNumber){
      VkPhysicalDeviceProperties *physicalDeviceProperties = (VkPhysicalDeviceProperties *)malloc(physicalDeviceNumber * sizeof(VkPhysicalDeviceProperties));
      VkPhysicalDeviceMemoryProperties *physicalDeviceMemoryProperties = (VkPhysicalDeviceMemoryProperties *)malloc(physicalDeviceNumber * sizeof(VkPhysicalDeviceMemoryProperties));

      uint32_t discreteGPUNumber = 0, integratedGPUNumber = 0, 
        *discreteGPUIndices = (uint32_t *)malloc(physicalDeviceNumber * sizeof(uint32_t)), 
        *integratedGPUIndices = (uint32_t *)malloc(physicalDeviceNumber * sizeof(uint32_t))
      ;

      for(uint32_t i = 0; i < physicalDeviceNumber; ++i){
        vkGetPhysicalDeviceProperties(pPhysicalDevices[i], &physicalDeviceProperties[i]);
        vkGetPhysicalDeviceMemoryProperties(pPhysicalDevices[i], &physicalDeviceMemoryProperties[i]);

        if(physicalDeviceProperties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
          discreteGPUIndices[discreteGPUNumber] = i;
          discreteGPUNumber++;
        }
        if(physicalDeviceProperties[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
          integratedGPUIndices[integratedGPUNumber] = i;
          integratedGPUNumber++;
        }
      }

      uint32_t bestPhysicalDeviceIndex = 0;
      VkDeviceSize bestPhysicalDeviceMemory = 0;

      if(discreteGPUNumber != 0){
        for(uint32_t i = 0; i < discreteGPUNumber; ++i){
          if(bestPhysicalDeviceMemory < getPhysicalDeviceTotalMemory(&physicalDeviceMemoryProperties[discreteGPUIndices[i]])){
            bestPhysicalDeviceMemory = getPhysicalDeviceTotalMemory(&physicalDeviceMemoryProperties[discreteGPUIndices[i]]);
            bestPhysicalDeviceIndex = discreteGPUIndices[i];
          }
        }
      }else if(integratedGPUNumber != 0){
        for(uint32_t i = 0; i < integratedGPUNumber; ++i){
          if(bestPhysicalDeviceMemory < getPhysicalDeviceTotalMemory(&physicalDeviceMemoryProperties[integratedGPUIndices[i]])){
            bestPhysicalDeviceMemory = getPhysicalDeviceTotalMemory(&physicalDeviceMemoryProperties[integratedGPUIndices[i]]);
            bestPhysicalDeviceIndex = integratedGPUIndices[i];
          }
        }
      }

      free(discreteGPUIndices);
      free(integratedGPUIndices);
      free(physicalDeviceMemoryProperties);
      free(physicalDeviceProperties);

      return bestPhysicalDeviceIndex;
    }
  // END
  poly_error_t pdev_create(poly_ctx_t *ctx, int sort) {
    // get physical device count
    vkEnumeratePhysicalDevices(ctx->instance, &ctx->physicalDeviceCount, VK_NULL_HANDLE);
    if (ctx->physicalDeviceCount == 0) {
      return POLY_NO_PHYSICAL_DEVICES;
    }
    // get physical devices
    ctx->physicalDevices = (VkPhysicalDevice *)malloc(ctx->physicalDeviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(ctx->instance, &ctx->physicalDeviceCount, ctx->physicalDevices);

    // filter physical devices
    uint32_t bestPhysicalDeviceIndex = getBestPhysicalDeviceIndex(ctx->physicalDevices, ctx->physicalDeviceCount);
    ctx->bestPhysicalDevice = &ctx->physicalDevices[bestPhysicalDeviceIndex];

    // get device queue family properties count
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(*ctx->bestPhysicalDevice, &queueFamilyCount, VK_NULL_HANDLE);
    // get device queue family properties
    VkQueueFamilyProperties *queueFamilyProperties = (VkQueueFamilyProperties *)malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(*ctx->bestPhysicalDevice, &queueFamilyCount, queueFamilyProperties);

    // create device
    VkDeviceQueueCreateInfo *deviceQueueCreateInfo = (VkDeviceQueueCreateInfo *)malloc(queueFamilyCount * sizeof(VkDeviceQueueCreateInfo));
    float **queuePriorities = (float **)malloc(queueFamilyCount * sizeof(float *));

    for(uint32_t i = 0; i < queueFamilyCount; ++i){
      queuePriorities[i] = (float *)malloc(queueFamilyProperties[i].queueCount * sizeof(float));
      for(uint32_t j = 0; j < queueFamilyProperties[i].queueCount; j++){
        queuePriorities[i][j] = 1.0f;
      }

      deviceQueueCreateInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      deviceQueueCreateInfo[i].pNext = VK_NULL_HANDLE;
      deviceQueueCreateInfo[i].flags = 0;
      deviceQueueCreateInfo[i].queueFamilyIndex = i;
      deviceQueueCreateInfo[i].queueCount = queueFamilyProperties[i].queueCount;
      deviceQueueCreateInfo[i].pQueuePriorities = queuePriorities[i];
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    vkGetPhysicalDeviceFeatures(*ctx->bestPhysicalDevice, &physicalDeviceFeatures);

    VkDeviceCreateInfo deviceCreateInfo = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      VK_NULL_HANDLE,
      0,
      queueFamilyCount,
      deviceQueueCreateInfo,
      0,
      VK_NULL_HANDLE,
      1,
      deviceExtensions,
      &physicalDeviceFeatures
    };

    vkCreateDevice(*ctx->bestPhysicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &ctx->device);

    for(uint32_t i = 0; i < queueFamilyCount; ++i){
      free(queuePriorities[i]);
    }
    free(queuePriorities);
    free(deviceQueueCreateInfo);

    // create graphic queue
    ctx->bestGraphicsQueueFamilyindex = getBestGraphicsQueueFamilyindex(queueFamilyProperties, queueFamilyCount);
    if (queueFamilyProperties[ctx->bestGraphicsQueueFamilyindex].queueCount == 1) {
      ctx->graphicsQueueMode = 0;
    } else if (queueFamilyProperties[ctx->bestGraphicsQueueFamilyindex].queueCount > 1) {
      ctx->graphicsQueueMode = 1;
    } else {
      ctx->graphicsQueueMode = 2;
    }

    // create present queue
    vkGetDeviceQueue(ctx->device, ctx->bestGraphicsQueueFamilyindex, 0, &ctx->drawingQueue);
    if (ctx->graphicsQueueMode == 0) {
      vkGetDeviceQueue(ctx->device, ctx->bestGraphicsQueueFamilyindex, 0, &ctx->presentingQueue);
    } else if(ctx->graphicsQueueMode == 1) {
      vkGetDeviceQueue(ctx->device, ctx->bestGraphicsQueueFamilyindex, 1, &ctx->presentingQueue);
    }

    free(queueFamilyProperties);
    return POLY_GOOD;
  }
  poly_error_t pdev_delete(poly_ctx_t *ctx) { // delete vk logical device
    vkDeviceWaitIdle(ctx->device);
    free(ctx->physicalDevices);
    vkDestroyDevice(ctx->device, NULL);
    return POLY_GOOD;
  }
  poly_error_t pdev_idle(poly_ctx_t *ctx) { vkDeviceWaitIdle(ctx->device); }
// window init (and surface)
  poly_error_t pwin_create(poly_ctx_t *ctx, long width, long height, uint32_t imageCount) {
    // create window and surface
    glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
  	ctx->window = glfwCreateWindow(width, height, "Default PolyGL Title", VK_NULL_HANDLE, VK_NULL_HANDLE);
  	glfwCreateWindowSurface(ctx->instance, ctx->window, VK_NULL_HANDLE, &ctx->surface);
    VkBool32 surfaceSupport = 0;
  	vkGetPhysicalDeviceSurfaceSupportKHR(*ctx->bestPhysicalDevice, ctx->bestGraphicsQueueFamilyindex, ctx->surface, &surfaceSupport);
    if(!surfaceSupport){
      ERRW("vulkan surface not supported!\n");
      vkDestroySurfaceKHR(ctx->instance, ctx->surface, VK_NULL_HANDLE);
      glfwDestroyWindow(ctx->window);
      return POLY_NO_SURFACE_SUPPORT;
    }

    ctx->imageCount = imageCount;

    // Create Swapchain
    VkSurfaceCapabilitiesKHR surfaceCapabilities = getSurfaceCapabilities(&ctx->surface, ctx->bestPhysicalDevice);
    VkSurfaceFormatKHR bestSurfaceFormat = getBestSurfaceFormat(&ctx->surface, ctx->bestPhysicalDevice);
    VkPresentModeKHR bestPresentMode = getBestPresentMode(&ctx->surface, ctx->bestPhysicalDevice);
    ctx->bestSwapchainExtent = getBestSwapchainExtent(&surfaceCapabilities, ctx->window);
    ctx->swapchain = createSwapChain(&ctx->device, &ctx->surface, &surfaceCapabilities, &bestSurfaceFormat, &ctx->bestSwapchainExtent, &bestPresentMode, imageCount, ctx->graphicsQueueMode);

    // Create Images
    ctx->swapchainImageCount = getSwapchainImageNumber(&ctx->device, &ctx->swapchain);
    ctx->swapchainImages = getSwapchainImages(&ctx->device, &ctx->swapchain, ctx->swapchainImageCount);

    // Create Image Views
    ctx->swapchainImageViews = createImageViews(&ctx->device, &ctx->swapchainImages, &bestSurfaceFormat, ctx->swapchainImageCount, imageCount);

    // Create render pass
    ctx->renderPass = createRenderPass(&ctx->device, &bestSurfaceFormat);

    // frame buffers
    ctx->framebuffers = createFramebuffers(&ctx->device, &ctx->renderPass, &ctx->bestSwapchainExtent, &ctx->swapchainImageViews, ctx->swapchainImageCount);
    
    return POLY_GOOD;
  }
  poly_error_t pwin_delete(poly_ctx_t *ctx) {
    // delete surface helpers helpers    
    for (uint32_t i = 0; i < ctx->swapchainImageCount; ++i) {
		  vkDestroyImageView(ctx->device, ctx->swapchainImageViews[i], VK_NULL_HANDLE);
      vkDestroyFramebuffer(ctx->device, ctx->framebuffers[i], VK_NULL_HANDLE);
    }
    free(ctx->swapchainImages);
    free(ctx->swapchainImageViews);
    free(ctx->framebuffers);

    // delete surface helpers
    vkDestroyRenderPass(ctx->device, ctx->renderPass, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(ctx->device, ctx->swapchain, VK_NULL_HANDLE);
    
    // destroy surface and window
    vkDestroySurfaceKHR(ctx->instance, ctx->surface, VK_NULL_HANDLE);
    glfwDestroyWindow(ctx->window);
    return POLY_GOOD;
  }
// pipeline (shaders)
  poly_error_t ppipe_create(
    poly_ctx_t *ctx, poly_pipe_t **g_pipe,
    const char *vertexShaderFileName, const char *fragmentShaderFileName
  ) {
    *g_pipe = NULL;
    poly_pipe_t *pipe = malloc(sizeof(poly_pipe_t));

    uint32_t vertexShaderSize = 0;
    char *vertexShaderCode = getShaderCode(vertexShaderFileName, &vertexShaderSize);
    if(vertexShaderCode == VK_NULL_HANDLE){
      ERR("vertex shader '%s' not found!", vertexShaderFileName);
      return POLY_FILE_NOT_FOUND;
    }
    VkShaderModule vertexShaderModule = createShaderModule(&ctx->device, vertexShaderCode, vertexShaderSize);

    uint32_t fragmentShaderSize = 0;
    char *fragmentShaderCode = getShaderCode(fragmentShaderFileName, &fragmentShaderSize);
    if(fragmentShaderCode == VK_NULL_HANDLE){
      ERR("fragment shader '%s' not found!", vertexShaderFileName);
      return POLY_FILE_NOT_FOUND;
    }
    VkShaderModule fragmentShaderModule = createShaderModule(&ctx->device, fragmentShaderCode, fragmentShaderSize);

    pipe->layout = createPipelineLayout(&ctx->device);
    pipe->pipe = createGraphicsPipeline(&ctx->device, &pipe->layout, &vertexShaderModule, &fragmentShaderModule, &ctx->renderPass, &ctx->bestSwapchainExtent);

    deleteShaderModule(&ctx->device, &fragmentShaderModule);
    deleteShaderCode(&fragmentShaderCode);
    deleteShaderModule(&ctx->device, &vertexShaderModule);
    deleteShaderCode(&vertexShaderCode);

    *g_pipe = pipe;
    return POLY_GOOD;
  }
  poly_error_t ppipe_delete(poly_ctx_t *ctx, poly_pipe_t *pipe) {    
    vkDestroyPipeline(ctx->device, pipe->pipe, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(ctx->device, pipe->layout, VK_NULL_HANDLE);
    free(pipe);
    return POLY_GOOD;
  }
// renderer object
  poly_error_t prctx_create(poly_rctx_t **g_rctx, poly_pipe_t *pipe, poly_ctx_t *ctx, uint32_t maxFrames) {
    *g_rctx = NULL;
    poly_rctx_t *rctx = malloc(sizeof(poly_rctx_t));

    rctx->maxFrames = maxFrames;

    // create command pool
    rctx->commandPool = createCommandPool(&ctx->device, ctx->bestGraphicsQueueFamilyindex);
	  rctx->commandBuffers = createCommandBuffers(&ctx->device, &rctx->commandPool, ctx->swapchainImageCount);

    recordCommandBuffers(&rctx->commandBuffers, &ctx->renderPass, &ctx->framebuffers, &ctx->bestSwapchainExtent, &pipe->pipe, ctx->swapchainImageCount);

    // create sync objects
    rctx->waitSemaphores = createSemaphores(&ctx->device, maxFrames), 
    rctx->signalSemaphores = createSemaphores(&ctx->device, maxFrames);
  	rctx->frontFences = createFences(&ctx->device, maxFrames), 
		rctx->backFences = createEmptyFences(ctx->swapchainImageCount);

    *g_rctx = rctx;
    return POLY_GOOD;
  }
  poly_error_t prctx_delete(poly_rctx_t *rctx, poly_pipe_t *pipe, poly_ctx_t *ctx) {
    free(rctx->backFences);
	  
    for(uint32_t i = 0; i < rctx->maxFrames; ++i){
      vkDestroyFence(ctx->device, rctx->frontFences[i], VK_NULL_HANDLE);
    }
    free(rctx->frontFences);

    for (uint32_t i = 0; i < rctx->maxFrames; ++i) {
      vkDestroySemaphore(ctx->device, rctx->signalSemaphores[i], VK_NULL_HANDLE);
    }
    free(rctx->signalSemaphores);

    for (uint32_t i = 0; i < rctx->maxFrames; ++i) {
      vkDestroySemaphore(ctx->device, rctx->waitSemaphores[i], VK_NULL_HANDLE);
    }
    free(rctx->waitSemaphores);

    vkFreeCommandBuffers(ctx->device, rctx->commandPool, ctx->swapchainImageCount, rctx->commandBuffers);
	  free(rctx->commandBuffers);
    vkDestroyCommandPool(ctx->device, rctx->commandPool, VK_NULL_HANDLE);
    free(rctx);
    return POLY_GOOD;
  }
// ------------ Manipulation Functions ------------ //
// window
  int pwin_should_close(poly_ctx_t *ctx) {
    return glfwWindowShouldClose(ctx->window);
  }
  poly_error_t pwin_set_ctx(poly_ctx_t *ctx) {
    glfwMakeContextCurrent(ctx->window);
    return POLY_GOOD;
  }
  poly_error_t pwin_poll() {
    glfwPollEvents();
    return POLY_GOOD;
  }
// renderer
  void poly_aquire_frame(poly_rctx_t *rctx, poly_pipe_t *pipe, poly_ctx_t *ctx, uint32_t currentFrame) {
    vkWaitForFences(ctx->device, 1, &rctx->frontFences[currentFrame], VK_TRUE, UINT64_MAX);
		uint32_t imageIndex = 0;
		vkAcquireNextImageKHR(ctx->device, ctx->swapchain, UINT64_MAX, rctx->waitSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		if(rctx->backFences[imageIndex] != VK_NULL_HANDLE){
			vkWaitForFences(ctx->device, 1, &rctx->backFences[imageIndex], VK_TRUE, UINT64_MAX);
		}
		rctx->backFences[imageIndex] = rctx->frontFences[currentFrame];

    VkPipelineStageFlags pipelineStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submitInfo = {
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			VK_NULL_HANDLE,
			1,
			&rctx->waitSemaphores[currentFrame],
			&pipelineStage,
			1,
			&rctx->commandBuffers[imageIndex],
			1,
			&rctx->signalSemaphores[currentFrame]
		};
		vkResetFences(ctx->device, 1, &rctx->frontFences[currentFrame]);
		vkQueueSubmit(ctx->drawingQueue, 1, &submitInfo, rctx->frontFences[currentFrame]);

		VkPresentInfoKHR presentInfo = {
			VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			VK_NULL_HANDLE,
			1,
			&rctx->signalSemaphores[currentFrame],
			1,
			&ctx->swapchain,
			&imageIndex,
			VK_NULL_HANDLE
		};
		vkQueuePresentKHR(ctx->presentingQueue, &presentInfo);
  }
// END
