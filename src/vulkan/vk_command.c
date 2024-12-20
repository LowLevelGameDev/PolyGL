#include "../polyvk.h"

VkCommandPool createCommandPool(VkDevice *pDevice, uint32_t queueFamilyIndex){
	VkCommandPoolCreateInfo commandPoolCreateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		queueFamilyIndex
	};

	VkCommandPool commandPool;
	vkCreateCommandPool(*pDevice, &commandPoolCreateInfo, VK_NULL_HANDLE, &commandPool);
	return commandPool;
}

void deleteCommandPool(VkDevice *pDevice, VkCommandPool *pCommandPool){
	vkDestroyCommandPool(*pDevice, *pCommandPool, VK_NULL_HANDLE);
}

VkCommandBuffer *createCommandBuffers(VkDevice *pDevice, VkCommandPool *pCommandPool, uint32_t commandBufferNumber){
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		VK_NULL_HANDLE,
		*pCommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		commandBufferNumber
	};

	VkCommandBuffer *commandBuffers = (VkCommandBuffer *)malloc(commandBufferNumber * sizeof(VkCommandBuffer));
	vkAllocateCommandBuffers(*pDevice, &commandBufferAllocateInfo, commandBuffers);
	return commandBuffers;
}

void deleteCommandBuffers(VkDevice *pDevice, VkCommandBuffer **ppCommandBuffers, VkCommandPool *pCommandPool, uint32_t commandBufferNumber){
	vkFreeCommandBuffers(*pDevice, *pCommandPool, commandBufferNumber, *ppCommandBuffers);
	free(*ppCommandBuffers);
}

void recordCommandBuffers(VkCommandBuffer **ppCommandBuffers, VkRenderPass *pRenderPass, VkFramebuffer **ppFramebuffers, VkExtent2D *pExtent, VkPipeline *pPipeline, uint32_t commandBufferNumber){
	VkCommandBufferBeginInfo *commandBufferBeginInfos = (VkCommandBufferBeginInfo *)malloc(commandBufferNumber * sizeof(VkCommandBufferBeginInfo));
	VkRenderPassBeginInfo *renderPassBeginInfos = (VkRenderPassBeginInfo *)malloc(commandBufferNumber *sizeof(VkRenderPassBeginInfo));
	VkRect2D renderArea = {
		{0, 0},
		{pExtent->width, pExtent->height}
	};
	VkClearValue clearValue = {0.6f, 0.2f, 0.8f, 0.0f};

	for(uint32_t i = 0; i < commandBufferNumber; i++){
		commandBufferBeginInfos[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfos[i].pNext = VK_NULL_HANDLE;
		commandBufferBeginInfos[i].flags = 0;
		commandBufferBeginInfos[i].pInheritanceInfo = VK_NULL_HANDLE;

		renderPassBeginInfos[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfos[i].pNext = VK_NULL_HANDLE;
		renderPassBeginInfos[i].renderPass = *pRenderPass;
		renderPassBeginInfos[i].framebuffer = (*ppFramebuffers)[i];
		renderPassBeginInfos[i].renderArea = renderArea;
		renderPassBeginInfos[i].clearValueCount = 1;
		renderPassBeginInfos[i].pClearValues = &clearValue;

		vkBeginCommandBuffer((*ppCommandBuffers)[i], &commandBufferBeginInfos[i]);
		vkCmdBeginRenderPass((*ppCommandBuffers)[i], &renderPassBeginInfos[i], VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline((*ppCommandBuffers)[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *pPipeline);
		vkCmdDraw((*ppCommandBuffers)[i], 3, 1, 0, 0);
		vkCmdEndRenderPass((*ppCommandBuffers)[i]);
		vkEndCommandBuffer((*ppCommandBuffers)[i]);
	}

	free(renderPassBeginInfos);
	free(commandBufferBeginInfos);
}
