#include "../polyvk.h"

GLFWwindow *createVulkanWindow(int width, int height, const char *title){
	glfwWindowHint(GLFW_CLIENT_API,GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);
	GLFWwindow *window = glfwCreateWindow(width, height, title, VK_NULL_HANDLE, VK_NULL_HANDLE);
	return window;
}

void deleteWindow(GLFWwindow *pWindow){
	glfwDestroyWindow(pWindow);
}

VkSurfaceKHR createSurface(GLFWwindow *pWindow, VkInstance *pInstance){
	VkSurfaceKHR surface;
	glfwCreateWindowSurface(*pInstance, pWindow, VK_NULL_HANDLE, &surface);
	return surface;
}

void deleteSurface(VkSurfaceKHR *pSurface, VkInstance *pInstance){
	vkDestroySurfaceKHR(*pInstance, *pSurface, VK_NULL_HANDLE);
}

VkBool32 getSurfaceSupport(VkSurfaceKHR *pSurface, VkPhysicalDevice *pPhysicalDevice, uint32_t graphicsQueueFamilyindex){
	VkBool32 surfaceSupport = 0;
	vkGetPhysicalDeviceSurfaceSupportKHR(*pPhysicalDevice, graphicsQueueFamilyindex, *pSurface, &surfaceSupport);
	return surfaceSupport;
}

VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkSurfaceKHR *pSurface, VkPhysicalDevice *pPhysicalDevice){
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*pPhysicalDevice, *pSurface, &surfaceCapabilities);
	return surfaceCapabilities;
}

VkSurfaceFormatKHR getBestSurfaceFormat(VkSurfaceKHR *pSurface, VkPhysicalDevice *pPhysicalDevice){
	uint32_t surfaceFormatNumber = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(*pPhysicalDevice, *pSurface, &surfaceFormatNumber, VK_NULL_HANDLE);
	VkSurfaceFormatKHR *surfaceFormats = (VkSurfaceFormatKHR *)malloc(surfaceFormatNumber * sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(*pPhysicalDevice, *pSurface, &surfaceFormatNumber, surfaceFormats);
	VkSurfaceFormatKHR bestSurfaceFormat = surfaceFormats[0];

	free(surfaceFormats);
	return bestSurfaceFormat;
}

VkPresentModeKHR getBestPresentMode(VkSurfaceKHR *pSurface, VkPhysicalDevice *pPhysicalDevice){
	uint32_t presentModeNumber = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(*pPhysicalDevice, *pSurface, &presentModeNumber, VK_NULL_HANDLE);
	VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeNumber * sizeof(VkPresentModeKHR));
	vkGetPhysicalDeviceSurfacePresentModesKHR(*pPhysicalDevice, *pSurface, &presentModeNumber, presentModes);

	VkPresentModeKHR bestPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	for(uint32_t i = 0; i < presentModeNumber; i++){
		if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR){
			bestPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		}
	}

	free(presentModes);
	return bestPresentMode;
}

VkExtent2D getBestSwapchainExtent(VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, GLFWwindow *window){
	int FramebufferWidth = 0, FramebufferHeight = 0;
	glfwGetFramebufferSize(window, &FramebufferWidth, &FramebufferHeight);

	VkExtent2D bestSwapchainExtent;

	if(pSurfaceCapabilities->currentExtent.width < FramebufferWidth){
		bestSwapchainExtent.width = pSurfaceCapabilities->currentExtent.width;
	}else{
		bestSwapchainExtent.width = FramebufferWidth;
	}

	if(pSurfaceCapabilities->currentExtent.height < FramebufferHeight){
		bestSwapchainExtent.height = pSurfaceCapabilities->currentExtent.height;
	}else{
		bestSwapchainExtent.height = FramebufferHeight;
	}

	return bestSwapchainExtent;
}

VkSwapchainKHR createSwapChain(VkDevice *pDevice, VkSurfaceKHR *pSurface, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, VkSurfaceFormatKHR *pSurfaceFormat, VkExtent2D *pSwapchainExtent, VkPresentModeKHR *pPresentMode, uint32_t imageArrayLayers, uint32_t graphicsQueueMode){
	VkSharingMode imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uint32_t queueFamilyIndexCount = 0, *pQueueFamilyIndices = VK_NULL_HANDLE;
	uint32_t queueFamilyIndices[] = {0, 1};
	if(graphicsQueueMode == 1){
		imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		queueFamilyIndexCount = 2;
		pQueueFamilyIndices = queueFamilyIndices;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		VK_NULL_HANDLE,
		0,
		*pSurface,
		pSurfaceCapabilities->minImageCount + 1,
		pSurfaceFormat->format,
		pSurfaceFormat->colorSpace,
		*pSwapchainExtent,
		imageArrayLayers,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		imageSharingMode,
		queueFamilyIndexCount,
		pQueueFamilyIndices,
		pSurfaceCapabilities->currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		*pPresentMode,
		VK_TRUE,
		VK_NULL_HANDLE
	};

	VkSwapchainKHR swapchain;
	vkCreateSwapchainKHR(*pDevice, &swapchainCreateInfo, VK_NULL_HANDLE, &swapchain);
	return swapchain;
}

void deleteSwapchain(VkDevice *pDevice, VkSwapchainKHR *pSwapchain){
	vkDestroySwapchainKHR(*pDevice, *pSwapchain, VK_NULL_HANDLE);
}
