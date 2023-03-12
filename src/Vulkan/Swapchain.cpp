#include "Swapchain.hpp"

void Swapchain::Create(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
                       int32_t windowWidth, int32_t windowHeight,
                       VkPresentModeKHR preferredPresentMode)
{
    SwapchainSupportDetails swapchainSupport = QuerySupport(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode =
        ChoosePresentMode(swapchainSupport.presentModes, preferredPresentMode);
    extent = ChooseExtent(swapchainSupport.capabilities, windowWidth, windowHeight);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapchainSupport.capabilities.maxImageCount)
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to create swap chain!");
    }

    imageFormat = surfaceFormat.format;
}

SwapchainSupportDetails Swapchain::QuerySupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                                  details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR
Swapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR
Swapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes,
                             VkPresentModeKHR preferredPresentMode)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == preferredPresentMode)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                                   int32_t windowWidth, int32_t windowHeight)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {static_cast<uint32_t>(windowWidth),
                                   static_cast<uint32_t>(windowHeight)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Swapchain::Cleanup(VmaAllocator allocator, VkDevice device)
{
    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Swapchain::Recreate(VmaAllocator allocator, VkDevice device, VkPhysicalDevice physicalDevice,
                         VkSurfaceKHR surface, int32_t windowWidth, int32_t windowHeight)
{
    vkDeviceWaitIdle(device);

    Cleanup(allocator, device);

    Create(device, physicalDevice, surface, windowWidth, windowHeight);
}

VkResult Swapchain::GetNextImage(VkDevice device, VkSemaphore semaphore, uint32_t &imageIndex)
{
    return vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE,
                                 &imageIndex);
}

const VkSwapchainKHR &Swapchain::GetSwapchain() { return swapchain; }

const VkFormat &Swapchain::GetImageFormat() { return imageFormat; }

const VkExtent2D &Swapchain::GetExtent() { return extent; }