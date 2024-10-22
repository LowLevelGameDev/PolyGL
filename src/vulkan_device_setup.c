#include "vulkan_device_setup.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#define LOGW(str) printf("INFO: " str "\n");
#define LOG(str, ...) printf("INFO: " str "\n", __VA_ARGS__)
#define ERRW(str) fprintf(stderr,"ERROR: " str "\n");
#define ERR(str, ...) fprintf(stderr,"ERROR: " str "\n", __VA_ARGS__)

struct VulkanCtx {
  VkInstance instance;
  VkDevice device;
  VkSurfaceKHR surface;
  VkCommandPool commandPool;
};

void vulkanInit(VulkanCtx *ctx, void *window) {
  vulkanCreateInstance(ctx);
  vulkanSelectDevice(ctx);
  vulkanCreateSurface(ctx, (GLFWwindow*)window);
}
void vulcanTerminate(VulkanCtx *ctx) {
  vkDeviceWaitIdle(ctx->device);
  //
	//destroy surface and window
	//
	vkDestroySurfaceKHR(ctx->instance,ctx->surface,NULL);
	LOGW("surface destroyed.");
	//
	//destroy device
	//
	vkDestroyDevice(ctx->device,NULL);
	LOGW("logical device destroyed.");
	//
	//destroy instance
	//
	vkDestroyInstance(ctx->instance,NULL);
	LOGW("instance destroyed.");
}

int vulkanCreateInstance(VulkanCtx *ctx) {
  // create application info
  VkApplicationInfo app_info;

  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = NULL;
  char app_name[VK_MAX_EXTENSION_NAME_SIZE];
  strcpy(app_name,"vulkan_project");
  app_info.pApplicationName = app_name;
  app_info.applicationVersion = VK_MAKE_VERSION(0,0,1);
  char app_engine_name[VK_MAX_EXTENSION_NAME_SIZE];
  strcpy(app_engine_name,"vulkan_engine");
  app_info.pEngineName = app_engine_name;
  app_info.engineVersion = VK_MAKE_VERSION(0,0,1);
  app_info.apiVersion = VK_API_VERSION_1_0;

  // create instance create info
  VkInstanceCreateInfo inst_cre_info;

  inst_cre_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_cre_info.pNext = NULL;
  inst_cre_info.flags = 0;
  inst_cre_info.pApplicationInfo = &app_info;
  uint32_t inst_layer_count = 1;
  inst_cre_info.enabledLayerCount = inst_layer_count;
  char pp_inst_layers[inst_layer_count][VK_MAX_EXTENSION_NAME_SIZE];
  strcpy(pp_inst_layers[0],"VK_LAYER_KHRONOS_validation");
  char *pp_inst_layer_names[inst_layer_count];
  for(uint32_t i = 0; i < inst_layer_count; ++i){
    pp_inst_layer_names[i]=  pp_inst_layers[i];
  }
  inst_cre_info.ppEnabledLayerNames = (char const* const*)pp_inst_layer_names;
  uint32_t inst_ext_count = 0;
  char const* const* pp_inst_ext_names = glfwGetRequiredInstanceExtensions(&inst_ext_count);
  inst_cre_info.enabledExtensionCount=inst_ext_count;
  inst_cre_info.ppEnabledExtensionNames=pp_inst_ext_names;

  // create instance
  VkInstance inst;
  vkCreateInstance(&inst_cre_info,NULL,&inst);

  ctx->instance = inst;

  return true;
}

int vulkanSelectDevice(VulkanCtx *ctx) {
  //enum physical device
	//
	uint32_t phy_devi_count=0;
	vkEnumeratePhysicalDevices(ctx->instance,&phy_devi_count,NULL);

	VkPhysicalDevice *phy_devis = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * phy_devi_count);
	vkEnumeratePhysicalDevices(ctx->instance,&phy_devi_count,phy_devis);
	//
	//select physical device
	//
	VkPhysicalDeviceProperties *phy_devis_props = (VkPhysicalDeviceProperties*)malloc(sizeof(VkPhysicalDeviceProperties) * phy_devi_count);
	uint32_t *discrete_gpu_list = (uint32_t*)malloc(sizeof(uint32_t) * phy_devi_count);
	uint32_t discrete_gpu_count = 0;
	uint32_t *intergrated_gpu_list = (uint32_t*)malloc(sizeof(uint32_t) * phy_devi_count);
	uint32_t intergrated_gpu_count = 0;

	VkPhysicalDeviceMemoryProperties *phy_devis_mem_props = (VkPhysicalDeviceMemoryProperties*)malloc(sizeof(VkPhysicalDeviceMemoryProperties) * phy_devi_count);
	uint32_t *phy_devis_mem_count = (uint32_t*)malloc(sizeof(uint32_t) * phy_devi_count);
	VkDeviceSize *phy_devis_mem_total = (VkDeviceSize*)malloc(sizeof(VkDeviceSize) * phy_devi_count);

	for(uint32_t i = 0; i < phy_devi_count; ++i){
		vkGetPhysicalDeviceProperties(phy_devis[i],&phy_devis_props[i]);
		if(phy_devis_props[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			discrete_gpu_list[discrete_gpu_count] = i;
			++discrete_gpu_count;
		}else if(phy_devis_props[i].deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
			intergrated_gpu_list[intergrated_gpu_count]=i;
			++intergrated_gpu_count;
		}

		vkGetPhysicalDeviceMemoryProperties(phy_devis[i],&phy_devis_mem_props[i]);
		phy_devis_mem_count[i] = phy_devis_mem_props[i].memoryHeapCount;
		phy_devis_mem_total[i] = 0;
		for(uint32_t j = 0; j < phy_devis_mem_count[i]; ++j){
			phy_devis_mem_total[i]+=
				phy_devis_mem_props[i].memoryHeaps[j].size;
		}
	}

	VkDeviceSize max_mem_size = 0;
	uint32_t phy_devi_best_index = 0;

	if (discrete_gpu_count!=0) {
		for (uint32_t i = 0; i < discrete_gpu_count; ++i){
			if(phy_devis_mem_total[i]>max_mem_size){
				phy_devi_best_index=discrete_gpu_list[i];
				max_mem_size=phy_devis_mem_total[i];
			}
		}
	}else if(intergrated_gpu_count!=0){
		for(uint32_t i = 0; i < intergrated_gpu_count; ++i){
			if(phy_devis_mem_total[i]>max_mem_size){
				phy_devi_best_index=intergrated_gpu_list[i];
				max_mem_size=phy_devis_mem_total[i];
			}
		}
	}

	LOG("best device index:%u",phy_devi_best_index);
	LOG("device name:%s",phy_devis_props
		[phy_devi_best_index].deviceName);
	LOGW("device type:");
	if(discrete_gpu_count!=0){
		LOGW("discrete gpu");
	}else if(intergrated_gpu_count!=0){
		LOGW("intergrated gpu");
	}else{
		LOGW("unknown");
	}
	LOG("memory total:%llu",phy_devis_mem_total[phy_devi_best_index]);
	VkPhysicalDevice *phy_best_devi = &(phy_devis[phy_devi_best_index]);
	//
	//query queue families
	//
	uint32_t qf_prop_count=0;
	vkGetPhysicalDeviceQueueFamilyProperties(*phy_best_devi,&qf_prop_count,NULL);
	VkQueueFamilyProperties qf_props[qf_prop_count];
	vkGetPhysicalDeviceQueueFamilyProperties(*phy_best_devi,&qf_prop_count,qf_props);
  uint32_t qf_q_count[qf_prop_count];
	for(uint32_t i=0;i<qf_prop_count;i++){
		qf_q_count[i]=qf_props[i].queueCount;
	}
	//
	//create logical device
	//
	VkDeviceQueueCreateInfo dev_q_cre_infos[qf_prop_count];
	for(uint32_t i=0;i<qf_prop_count;i++){
		dev_q_cre_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		dev_q_cre_infos[i].pNext = NULL;
		dev_q_cre_infos[i].flags = 0;
		dev_q_cre_infos[i].queueFamilyIndex = i;
		dev_q_cre_infos[i].queueCount=qf_q_count[i];
		float q_prior[1] = {1.0f};
		dev_q_cre_infos[i].pQueuePriorities = q_prior;
	}
	LOG("using %d queue families.",qf_prop_count);

	VkDeviceCreateInfo dev_cre_info;
	dev_cre_info.sType=
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_cre_info.pNext=NULL;
	dev_cre_info.flags=0;
	dev_cre_info.queueCreateInfoCount=qf_prop_count;
	dev_cre_info.pQueueCreateInfos=dev_q_cre_infos;
	dev_cre_info.enabledLayerCount=0;
	dev_cre_info.ppEnabledLayerNames=NULL;

	uint32_t dev_ext_count=1;
	dev_cre_info.enabledExtensionCount=dev_ext_count;
	char pp_dev_exts[dev_ext_count][VK_MAX_EXTENSION_NAME_SIZE];
	strcpy(pp_dev_exts[0],"VK_KHR_swapchain");
	char *pp_dev_ext_names[dev_ext_count];
	for(uint32_t i = 0; i < dev_ext_count; ++i){
		pp_dev_ext_names[i] = pp_dev_exts[i];
	}
	dev_cre_info.ppEnabledExtensionNames = (const char * const *)pp_dev_ext_names;

	VkPhysicalDeviceFeatures phy_devi_feat;
	vkGetPhysicalDeviceFeatures(*phy_best_devi,&phy_devi_feat);
	dev_cre_info.pEnabledFeatures = &phy_devi_feat;

	VkDevice dev;
	vkCreateDevice(*phy_best_devi,&dev_cre_info,NULL,&dev);
  ctx->device = dev;
	LOGW("logical device created.");
  return VULKAN_GOOD;
}

int vulkanCreateSurface(VulkanCtx *ctx, void *window) {
  //create window and surface
	//
	LOGW("window created.");
	VkSurfaceKHR surf;
	glfwCreateWindowSurface(ctx->instance,window,NULL,&surf);
  ctx->surface = surf;
	LOGW("surface created.");
}

int vulkanCreateCommandPool(VulkanCtx *ctx) {
  //create command pool
	//
	VkCommandPoolCreateInfo cmd_pool_cre_info;
	cmd_pool_cre_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_cre_info.pNext = NULL;
	cmd_pool_cre_info.flags = 0;
	// cmd_pool_cre_info.queueFamilyIndex = qf_best_index;

	VkCommandPool cmd_pool;
	vkCreateCommandPool(ctx->device,&cmd_pool_cre_info,NULL,&cmd_pool);
  ctx->commandPool = cmd_pool;
	printf("command pool created.\n");
}