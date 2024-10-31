#include <../include/polygl.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <string.h>
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
  "Failed to create logical device!"
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
  VkPhysicalDevice *physicalDevices;
  VkDevice device;
  VkQueue drawingQueue;
  VkQueue presentingQueue;
  // window & surface
  VkSurfaceKHR surface;
  poly_window_t window;
};

// ------------ Initalizer && CleanUp Functions ------------ //
  poly_error_t poly_full_destroy(poly_ctx_t *ctx) {
    poly_dev_delete(ctx);
    poly_win_delete(ctx);
    poly_destroy(ctx);
  }
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
    uint32_t getPhysicalDeviceNumber(VkInstance *pInstance){
      uint32_t physicalDeviceNumber = 0;
      vkEnumeratePhysicalDevices(*pInstance, &physicalDeviceNumber, VK_NULL_HANDLE);
      return physicalDeviceNumber;
    }
    VkPhysicalDevice *getPhysicalDevices(VkInstance *pInstance, uint32_t physicalDeviceNumber){
      VkPhysicalDevice *physicalDevices = (VkPhysicalDevice *)malloc(physicalDeviceNumber * sizeof(VkPhysicalDevice));
      vkEnumeratePhysicalDevices(*pInstance, &physicalDeviceNumber, physicalDevices);

      return physicalDevices;
    }
    void deletePhysicalDevices(VkPhysicalDevice **ppPhysicalDevices){
      free(*ppPhysicalDevices);
    }
    uint32_t getPhysicalDeviceTotalMemory(VkPhysicalDeviceMemoryProperties *pPhysicalDeviceMemoryProperties){
      uint32_t physicalDeviceTotalMemory = 0;
      for(int i = 0; i < pPhysicalDeviceMemoryProperties->memoryHeapCount; i++){
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

      for(uint32_t i = 0; i < physicalDeviceNumber; i++){
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
        for(uint32_t i = 0; i < discreteGPUNumber; i++){
          if(bestPhysicalDeviceMemory < getPhysicalDeviceTotalMemory(&physicalDeviceMemoryProperties[discreteGPUIndices[i]])){
            bestPhysicalDeviceMemory = getPhysicalDeviceTotalMemory(&physicalDeviceMemoryProperties[discreteGPUIndices[i]]);
            bestPhysicalDeviceIndex = discreteGPUIndices[i];
          }
        }
      }else if(integratedGPUNumber != 0){
        for(uint32_t i = 0; i < integratedGPUNumber; i++){
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
  poly_error_t poly_dev_create(poly_ctx_t *ctx, int sort) {
    uint32_t physicalDeviceNumber = getPhysicalDeviceNumber(&instance);
    VkPhysicalDevice *physicalDevices = getPhysicalDevices(&instance, physicalDeviceNumber);
    uint32_t bestPhysicalDeviceIndex = getBestPhysicalDeviceIndex(physicalDevices, physicalDeviceNumber);
    VkPhysicalDevice *pBestPhysicalDevice = &physicalDevices[bestPhysicalDeviceIndex];

    uint32_t queueFamilyNumber = getqueueFamilyNumber(pBestPhysicalDevice);
    VkQueueFamilyProperties *queueFamilyProperties = getQueueFamilyProperties(pBestPhysicalDevice, queueFamilyNumber);

    VkDevice device = createDevice(pBestPhysicalDevice, queueFamilyNumber, queueFamilyProperties);

    uint32_t bestGraphicsQueueFamilyindex = getBestGraphicsQueueFamilyindex(queueFamilyProperties, queueFamilyNumber);
    uint32_t graphicsQueueMode = getGraphicsQueueMode(queueFamilyProperties, bestGraphicsQueueFamilyindex);
    VkQueue drawingQueue = getDrawingQueue(&device, bestGraphicsQueueFamilyindex);
    VkQueue presentingQueue = getPresentingQueue(&device, bestGraphicsQueueFamilyindex, graphicsQueueMode);
    deleteQueueFamilyProperties(&queueFamilyProperties);
  }

  poly_error_t poly_dev_delete(poly_ctx_t *ctx) { // delete vk logical device
    vkDeviceWaitIdle(ctx->device);
    vkDestroyDevice(ctx->device, NULL);
    return POLY_GOOD;
  }
// window init (and surface)
  poly_error_t poly_win_create(poly_ctx_t *ctx, long Width, long Height) {
    // create window and surface
    return POLY_GOOD;
  }
  poly_error_t poly_win_delete(poly_ctx_t *ctx) {
    // destroy window and surface
    return POLY_GOOD;
  }
// ------------ Manipulation Functions ------------ //
// END
