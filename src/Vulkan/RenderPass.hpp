#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <array>
#include <functional>
#include <vector>

#include "../Error.hpp"
#include "Image.hpp"
#include "Swapchain.hpp"

class RenderPass
{
  public:
    void CreateCustom(
        VkDevice device, Swapchain &swapchain, uint32_t width, uint32_t height,
        std::function<VkRenderPass()> setupRenderPass, std::function<void(const VkExtent2D &extent)> recreateCallback,
        std::function<void()> cleanupCallback,
        std::function<void(std::vector<VkImageView> &attachments, VkImageView imageView)> setupFramebuffer);
    void Create(VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator allocator, Swapchain &swapchain,
                bool enableDepth, bool enableMsaa);
    void Recreate(VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator allocator, Swapchain &swapchain,
                  uint32_t width, uint32_t height);

    void Begin(const uint32_t imageIndex, VkCommandBuffer commandBuffer, uint32_t width, uint32_t height,
               const std::vector<VkClearValue> &clearValues);
    void End(VkCommandBuffer commandBuffer);

    VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &candidates,
                                 VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

    const VkRenderPass &GetRenderPass();
    const VkFramebuffer &GetFramebuffer(const uint32_t imageIndex);
    const VkSampleCountFlagBits GetMsaaSamples();
    const bool GetMsaaEnabled();

    void Cleanup(VmaAllocator, VkDevice device);

  private:
    void CreateImages(VkDevice device, Swapchain &swapchain);
    void CreateFramebuffers(VkDevice device, uint32_t width, uint32_t height);
    void CreateDepthResources(VmaAllocator allocator, VkPhysicalDevice physicalDevice, VkDevice device,
                              VkExtent2D extent);
    void CreateColorResources(VmaAllocator allocator, VkPhysicalDevice physicalDevice, VkDevice device,
                              VkExtent2D extent);
    void CreateImageViews(VkDevice device);
    void CleanupForRecreation(VmaAllocator allocator, VkDevice device);

    const VkSampleCountFlagBits GetMaxUsableSamples(VkPhysicalDevice physicalDevice);

    std::function<void()> cleanupCallback;
    std::function<void(const VkExtent2D &)> recreateCallback;
    std::function<void(std::vector<VkImageView> &attachments, VkImageView imageView)> setupFramebuffer;

    VkRenderPass renderPass;

    std::vector<Image> images;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;

    Image depthImage;
    VkImageView depthImageView;
    Image colorImage;
    VkImageView colorImageView;
    VkFormat imageFormat;
    bool depthEnabled = false;
    bool msaaEnabled = false;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
};