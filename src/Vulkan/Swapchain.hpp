#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <limits>
#include <stdexcept>
#include <vector>

#include "Image.hpp"
#include "QueueFamilyIndices.hpp"
#include "../Error.hpp"

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain
{
public:
    void Create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                int32_t windowWidth, int32_t windowHeight,
                VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR);
    void Cleanup(VmaAllocator allocator, VkDevice device);
    void Recreate(VmaAllocator allocator, VkDevice device, VkPhysicalDevice physicalDevice,
                  VkSurfaceKHR surface, int32_t windowWidth, int32_t windowHeight);

    SwapchainSupportDetails QuerySupport(VkPhysicalDevice device, VkSurfaceKHR surface);
    VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes,
                                       VkPresentModeKHR preferredPresentMode);
    VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities, int32_t windowWidth,
                            int32_t windowHeight);
    VkResult GetNextImage(VkDevice device, VkSemaphore semaphore, uint32_t &imageIndex);

    const VkSwapchainKHR &GetSwapchain();
    const VkExtent2D &GetExtent();
    const VkFormat &GetImageFormat();

private:
    VkSwapchainKHR swapchain;
    VkExtent2D extent;
    VkFormat imageFormat;
};