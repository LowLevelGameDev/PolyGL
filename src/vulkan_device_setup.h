#define VULKAN_INIT_GOOD 0
#define VULKAN_INIT_VAL_LAYERS 1
#define VULKAN_INIT_BAD_INSTANCE 2
#define VULCAN_GLFW_NO_EXTENSION 3

typedef struct {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  bool graphicsFamilyHasValue;
  bool presentFamilyHasValue;

  VkSurfaceCapabilitiesKHR *capabilities;
  VkSurfaceFormatKHR *formats;
  VkPresentModeKHR *presentModes;
} VulkanInfo;
void createVulkanInfo(VulkanInfo *info) {
  info->graphicsFamilyHasValue = false;
  info->presentFamilyHasValue = false;
  info->capabilities = NULL;
  info->formats = NULL;
  info->presentModes = NULL;
}
void deleteVulkanInfo(VulkanInfo *info) {
  if (info->capabilities != NULL) free(info->capabilities);
  if (info->formats != NULL) free(info->formats);
  if (info->presentModes != NULL) free(info->presentModes);
}
bool vulkanInfoIsComplete(VulkanInfo *info) {
  return info->graphicsFamilyHasValue && info->presentFamilyHasValue;
}

const const char * validationLayers[] = {
  "VK_LAYER_KHRONOS_validation"
};
const size_t validationLayersSize = sizeof(validationLayers) / sizeof(validationLayers[0]);
const const char * deviceExtensions[] = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
const size_t deviceExtensionsSize = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
  void *pUserData
) {
  ERR("validation layer: %s", pCallbackData->pMessage);
  return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
  VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
  const VkAllocationCallbacks *pAllocator,
  VkDebugUtilsMessengerEXT *pDebugMessenger
) {
  PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
    instance,
    "vkCreateDebugUtilsMessengerEXT"
  );
  if (func == NULL) return VK_ERROR_EXTENSION_NOT_PRESENT;
  else return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

void DestroyDebugUtilsMessengerEXT(
  VkInstance instance,
  VkDebugUtilsMessengerEXT debugMessenger,
  const VkAllocationCallbacks *pAllocator
) {
  PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
    instance,
    "vkDestroyDebugUtilsMessengerEXT"
  );
  if (func != NULL) {
    func(instance, debugMessenger, pAllocator);
  }
}

bool vulcanCheckValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, NULL);

  VkLayerProperties *availableLayers = malloc(layerCount*sizeof(VkLayerProperties));
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

const char** vulcanGetRequiredExtensions(uint32_t* extensionCount, int enableValidationLayers) {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  uint32_t additionalExtensions = enableValidationLayers ? 1 : 0;
  *extensionCount = glfwExtensionCount + additionalExtensions;
  
  const char** extensions = (const char**)malloc(*extensionCount * sizeof(const char*));
  if (!extensions) {
    ERR("memory error allocating %zu",*extensionCount * sizeof(const char*));
    return NULL;
  }

  for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
    extensions[i] = glfwExtensions[i];
  }

  if (enableValidationLayers) {
    extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  }

  return extensions;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT *createInfo) {
  createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo->pfnUserCallback = debugCallback;
  createInfo->pUserData = NULL;  // Optional
}

int hasGflwRequiredInstanceExtensions(int enableValidationLayers) {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
  VkExtensionProperties* extensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
  if (extensions == NULL) {
    ERR("Error allocating extensions of size %zu", sizeof(VkExtensionProperties) * extensionCount);
    return MEMORY_ERROR;
  }
  vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

  uint32_t requiredExtensionsCount = 0;
  const char **requiredExtensions = vulcanGetRequiredExtensions(&requiredExtensionsCount,enableValidationLayers);
  if (requiredExtensions == NULL) {
    ERR("Error allocating required extensions of size %zu", sizeof(VkExtensionProperties) * extensionCount);
    return MEMORY_ERROR;
  }

  for (uint32_t i = 0; i < requiredExtensionsCount; ++i) {
    bool found = false;
    for (uint32_t j = 0; j < extensionCount; ++j) {
      if (strcmp(extensions[j].extensionName, requiredExtensions[i]) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      ERR("Missing required glfw extenstion %s", requiredExtensions[i]);
      return VULCAN_GLFW_NO_EXTENSION;
    }
  }
}

typedef struct {
  bool enableValidationLayers;

  VkPhysicalDeviceProperties properties;

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;
  VkPhysicalDevice physicalDevice;
  VkCommandPool commandPool;

  VkDevice device;
  VkSurfaceKHR surface;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
} VulkanCtx;

int vulkanCreateInstance(VulkanCtx *ctx);

int vulkanInit(VulkanCtx *ctx) { 
  #ifdef NDEBUG
    ctx->enableValidationLayers = false;
  #else
    ctx->enableValidationLayers = true;
  #endif

  ctx->physicalDevice = VK_NULL_HANDLE;

  // create instance
  int status = vulkanCreateInstance(ctx);
  if (status != VULKAN_INIT_GOOD) return status;

  // setup debug messenger

  // create surface

  // select physical device

  // create logical device
  
  // create command pool

  return VULKAN_INIT_GOOD;
}
void vulkanTerminate(VulkanCtx *ctx) {
  // vkDestroyCommandPool(ctx->device, ctx->commandPool, NULL);
  // vkDestroyDevice(ctx->device, NULL);
  // if (ctx->enableValidationLayers) {
  //   DestroyDebugUtilsMessengerEXT(ctx->instance, ctx->debugMessenger, NULL);
  // }
  // vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);
  vkDestroyInstance(ctx->instance, NULL);
}

int vulkanCreateInstance(VulkanCtx *ctx) {
  if (ctx->enableValidationLayers && !vulcanCheckValidationLayerSupport()) {
    LOG("validation layers requested, but not available! %d",0);
    ctx->enableValidationLayers = false;
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Poly GL App";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  uint32_t extensionsSize = 0;
  const char **extensions = vulcanGetRequiredExtensions(&extensionsSize,ctx->enableValidationLayers);
  if (extensions == NULL) { return MEMORY_ERROR; }
  createInfo.enabledExtensionCount = extensionsSize;
  createInfo.ppEnabledExtensionNames = extensions;

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (ctx->enableValidationLayers) {
    createInfo.enabledLayerCount = validationLayersSize;
    createInfo.ppEnabledLayerNames = validationLayers;

    populateDebugMessengerCreateInfo(&debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = NULL;
  }


  if (vkCreateInstance(&createInfo, NULL, &ctx->instance) != VK_SUCCESS) {
    ERR("failed to create instance! %d", 0);
    return VULKAN_INIT_BAD_INSTANCE;
  }
/*
  hasGflwRequiredInstanceExtensions(ctx->enableValidationLayers);
*/
  free(extensions);
}