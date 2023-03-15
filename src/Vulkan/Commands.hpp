#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

#include "../Error.hpp"
#include "QueueFamilyIndices.hpp"

class Commands
{
  public:
    VkCommandBuffer BeginSingleTime(VkQueue graphicsQueue, VkDevice device);
    void EndSingleTime(VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkDevice device);

    void CreatePool(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface);

    void CreateBuffers(VkDevice device, size_t maxFramesInFlight);
    void ResetBuffer(const uint32_t imageIndex, const uint32_t currentFrame);
    void BeginBuffer(const uint32_t currentFrame);
    void EndBuffer(const uint32_t currentFrame);
    const VkCommandBuffer &GetBuffer(const uint32_t currentFrame);

    void Destroy(VkDevice device);

  private:
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> buffers;
};