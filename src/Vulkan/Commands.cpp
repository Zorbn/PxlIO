#include "Commands.hpp"

VkCommandBuffer Commands::BeginSingleTime(VkQueue graphicsQueue, VkDevice device)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Commands::EndSingleTime(VkCommandBuffer commandBuffer, VkQueue graphicsQueue,
                             VkDevice device)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void Commands::CreatePool(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices queueFamilyIndices =
        QueueFamilyIndices::FindQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to create graphics command pool!");
    }
}

void Commands::CreateBuffers(VkDevice device, size_t maxFramesInFlight)
{
    buffers.resize(maxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)buffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, buffers.data()) != VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to allocate command buffers!");
    }
}

void Commands::ResetBuffer(const uint32_t imageIndex, const uint32_t currentFrame)
{
    vkResetCommandBuffer(buffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
}

void Commands::BeginBuffer(const uint32_t currentFrame)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(buffers[currentFrame], &beginInfo) != VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to begin recording command buffer!");
    }
}

void Commands::EndBuffer(const uint32_t currentFrame)
{
    if (vkEndCommandBuffer(buffers[currentFrame]) != VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to record command buffer!");
    }
}

const VkCommandBuffer &Commands::GetBuffer(const uint32_t currentFrame)
{
    return buffers[currentFrame];
}

void Commands::Destroy(VkDevice device) { vkDestroyCommandPool(device, commandPool, nullptr); }