#include "RenderPass.hpp"
#include "Swapchain.hpp"

void RenderPass::CreateCustom(
    VkDevice device, Swapchain &swapchain, uint32_t width, uint32_t height,
    std::function<VkRenderPass()> setupRenderPass,
    std::function<void(const VkExtent2D &extent)> recreateCallback,
    std::function<void()> cleanupCallback,
    std::function<void(std::vector<VkImageView> &attachments, VkImageView imageView)>
        setupFramebuffer)
{

    imageFormat = swapchain.GetImageFormat();

    this->cleanupCallback = cleanupCallback;
    this->recreateCallback = recreateCallback;
    this->setupFramebuffer = setupFramebuffer;

    renderPass = setupRenderPass();

    CreateImages(device, swapchain);
    CreateImageViews(device);

    const VkExtent2D &extent = swapchain.GetExtent();
    recreateCallback(extent);
    CreateFramebuffers(device, width, height);
}

void RenderPass::Create(VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator allocator,
                        Swapchain &swapchain, bool enableDepth, bool enableMsaa)
{
    std::function<VkRenderPass()> setupRenderPass = [&]
    {
        depthEnabled = enableDepth;
        msaaSamples = enableMsaa ? GetMaxUsableSamples(physicalDevice) : VK_SAMPLE_COUNT_1_BIT;
        msaaEnabled = msaaSamples != VK_SAMPLE_COUNT_1_BIT;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = imageFormat;
        colorAttachment.samples = msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = msaaEnabled ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                                                  : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat(physicalDevice);
        depthAttachment.samples = msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = imageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        if (msaaEnabled)
        {
            subpass.pResolveAttachments = &colorAttachmentResolveRef;
        }

        if (depthEnabled)
        {
            subpass.pDepthStencilAttachment = &depthAttachmentRef;
        }

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};

        if (msaaEnabled)
        {
            attachments.push_back(colorAttachmentResolve);
        }

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkRenderPass renderPass;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            RUNTIME_ERROR("Failed to create render pass!");
        }

        return renderPass;
    };

    std::function<void(const VkExtent2D &)> recreateCallback = [=](const VkExtent2D &extent)
    {
        CreateColorResources(allocator, physicalDevice, device, extent);
        CreateDepthResources(allocator, physicalDevice, device, extent);
    };

    std::function<void()> cleanupCallback = [=]
    {
        vkDestroyImageView(device, depthImageView, nullptr);
        depthImage.Destroy(allocator);
        vkDestroyImageView(device, colorImageView, nullptr);
        colorImage.Destroy(allocator);
    };

    std::function<void(std::vector<VkImageView> &, VkImageView)> setupFramebuffer =
        [&](std::vector<VkImageView> &attachments, VkImageView imageView)
    {
        if (msaaEnabled)
        {
            attachments.push_back(colorImageView);
        }
        else
        {
            attachments.push_back(imageView);
        }

        attachments.push_back(depthImageView);

        if (msaaEnabled)
        {
            attachments.push_back(imageView);
        }
    };

    const VkExtent2D &extent = swapchain.GetExtent();
    CreateCustom(device, swapchain, extent.width, extent.height, setupRenderPass,
                 recreateCallback, cleanupCallback, setupFramebuffer);
}

void RenderPass::CreateImages(VkDevice device, Swapchain &swapchain)
{
    VkSwapchainKHR vkSwapchain = swapchain.GetSwapchain();
    VkFormat format = swapchain.GetImageFormat();
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(device, vkSwapchain, &imageCount, nullptr);
    std::vector<VkImage> imagesVk;
    imagesVk.resize(imageCount);
    images.clear();
    images.reserve(imageCount);
    vkGetSwapchainImagesKHR(device, vkSwapchain, &imageCount, imagesVk.data());

    for (VkImage vkImage : imagesVk)
    {
        images.push_back(Image(vkImage, format));
    }
}

void RenderPass::Begin(const uint32_t imageIndex, VkCommandBuffer commandBuffer,
                       uint32_t width, uint32_t height,
                       const std::vector<VkClearValue> &clearValues)
{
    VkExtent2D extent = {width, height};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = extent;

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void RenderPass::End(VkCommandBuffer commandBuffer) { vkCmdEndRenderPass(commandBuffer); }

const VkRenderPass &RenderPass::GetRenderPass() { return renderPass; }

const VkSampleCountFlagBits RenderPass::GetMsaaSamples() { return msaaSamples; }

const bool RenderPass::GetMsaaEnabled() { return msaaEnabled; }

void RenderPass::CreateImageViews(VkDevice device)
{
    imageViews.resize(images.size());

    for (uint32_t i = 0; i < images.size(); i++)
    {
        imageViews[i] = images[i].CreateView(VK_IMAGE_ASPECT_COLOR_BIT, device);
    }
}

void RenderPass::CreateFramebuffers(VkDevice device, uint32_t width, uint32_t height)
{
    framebuffers.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); i++)
    {
        std::vector<VkImageView> attachments;
        setupFramebuffer(attachments, imageViews[i]);

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) !=
            VK_SUCCESS)
        {
            RUNTIME_ERROR("Failed to create framebuffer!");
        }
    }
}

void RenderPass::CreateDepthResources(VmaAllocator allocator, VkPhysicalDevice physicalDevice,
                                      VkDevice device, VkExtent2D extent)
{
    VkFormat depthFormat = FindDepthFormat(physicalDevice);

    depthImage = Image(allocator, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 1, msaaSamples);
    depthImageView = depthImage.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT, device);
}

void RenderPass::CreateColorResources(VmaAllocator allocator, VkPhysicalDevice physicalDevice,
                                      VkDevice device, VkExtent2D extent)
{
    colorImage =
        Image(allocator, extent.width, extent.height, imageFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1, 1, msaaSamples);
    colorImageView = colorImage.CreateView(VK_IMAGE_ASPECT_COLOR_BIT, device);
}

void RenderPass::Recreate(VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator allocator,
                          Swapchain &swapchain, uint32_t width, uint32_t height)
{
    CleanupForRecreation(allocator, device);

    const VkExtent2D &extent = swapchain.GetExtent();

    CreateImages(device, swapchain);
    CreateImageViews(device);
    recreateCallback(extent);
    CreateFramebuffers(device, width, height);
}

VkFormat RenderPass::FindSupportedFormat(VkPhysicalDevice physicalDevice,
                                         const std::vector<VkFormat> &candidates,
                                         VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                 (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    RUNTIME_ERROR("Failed to find supported format!");
}

VkFormat RenderPass::FindDepthFormat(VkPhysicalDevice physicalDevice)
{
    return FindSupportedFormat(
        physicalDevice,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void RenderPass::CleanupForRecreation(VmaAllocator allocator, VkDevice device)
{
    cleanupCallback();

    for (auto framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (auto imageView : imageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }
}

void RenderPass::Cleanup(VmaAllocator allocator, VkDevice device)
{
    CleanupForRecreation(allocator, device);
    vkDestroyRenderPass(device, renderPass, nullptr);
}

const VkFramebuffer &RenderPass::GetFramebuffer(const uint32_t imageIndex)
{
    return framebuffers[imageIndex];
}

const VkSampleCountFlagBits RenderPass::GetMaxUsableSamples(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT)
        return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT)
        return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}