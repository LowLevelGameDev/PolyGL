#define VULKAN_GOOD 0
#define VULKAN_BAD_INSTANCE 1
#define VULCAN_GLFW_NO_EXTENSION 2

struct VulkanCtx;
typedef struct VulkanCtx VulkanCtx;

void vulkanInit(VulkanCtx *ctx, void *window);
void vulcanTerminate(VulkanCtx *ctx);

int vulkanCreateInstance(VulkanCtx *ctx);

int vulkanSelectDevice(VulkanCtx *ctx);

int vulkanCreateSurface(VulkanCtx *ctx, void *window);

int vulkanCreateCommandPool(VulkanCtx *ctx);