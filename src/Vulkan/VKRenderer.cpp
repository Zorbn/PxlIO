#include "VKRenderer.hpp"

const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const uint32_t maxFramesInFlight = 2;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

VKRenderer::VKRenderer(const std::string &windowName, int32_t windowWidth, int32_t windowHeight,
                       int32_t viewWidth, int32_t viewHeight, bool enableVsync)
    : windowWidth(windowWidth), windowHeight(windowHeight), viewWidth(viewWidth), viewHeight(viewHeight), enableVsync(enableVsync)
{
    InitWindow(windowName);
    InitVulkan(maxFramesInFlight);
    ResizeWindow(windowWidth, windowHeight);
}

void VKRenderer::ResizeWindow(int windowWidth, int windowHeight)
{
    this->windowWidth = windowWidth;
    this->windowHeight = windowHeight;

    framebufferResized = true;
}

SDL_Window *VKRenderer::GetWindowPtr()
{
    return window;
}

void VKRenderer::SetBackgroundColor(float r, float g, float b)
{
    backgroundR = r;
    backgroundG = g;
    backgroundB = b;
}

void VKRenderer::SetScreenBackgroundColor(float r, float g, float b)
{
    screenBackgroundR = r;
    screenBackgroundG = g;
    screenBackgroundB = b;
}

void VKRenderer::HandleResize()
{
    renderPass.Recreate(vulkanState.physicalDevice, vulkanState.device, vulkanState.allocator,
                        vulkanState.swapchain);
    screenRenderPass.Recreate(vulkanState.physicalDevice, vulkanState.device, vulkanState.allocator,
                              vulkanState.swapchain);
    screenPipeline.Recreate<VertexData, InstanceData>(
        vulkanState.device, vulkanState.maxFramesInFlight, screenRenderPass);

    ViewTransform viewTransform = Renderer::CalcViewTransform(windowWidth, windowHeight,
        viewWidth, viewHeight);

    ScreenUniformBufferData screenUboData{};
    screenUboData.proj = VkOrtho(0.0f, static_cast<float>(windowWidth), 0.0f,
                                 static_cast<float>(windowHeight), zNear, zFar);
    screenUboData.viewSize = glm::vec2(viewTransform.scaledViewWidth, viewTransform.scaledViewHeight);
    screenUboData.offset = glm::vec2(viewTransform.offsetX, viewTransform.offsetY);

    screenUbo.Update(screenUboData);
}

void VKRenderer::BeginDrawing()
{
    vkWaitForFences(vulkanState.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vulkanState.swapchain.GetNextImage(
        vulkanState.device, imageAvailableSemaphores[currentFrame], currentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        WaitWhileMinimized();
        int32_t width;
        int32_t height;
        SDL_Vulkan_GetDrawableSize(window, &width, &height);
        vulkanState.swapchain.Recreate(vulkanState.allocator, vulkanState.device,
                                       vulkanState.physicalDevice, vulkanState.surface, width,
                                       height);
        HandleResize();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        RUNTIME_ERROR("Failed to acquire swap chain image!");
    }

    vkResetFences(vulkanState.device, 1, &inFlightFences[currentFrame]);

    vulkanState.commands.ResetBuffer(currentImageIndex, currentFrame);

    const VkExtent2D &extent = vulkanState.swapchain.GetExtent();
    const VkCommandBuffer &currentBuffer = vulkanState.commands.GetBuffer(currentFrame);

    vulkanState.commands.BeginBuffer(currentFrame);

    clearValues[0].color = ConvertClearColor(backgroundR, backgroundG, backgroundB,
        vulkanState.swapchain.GetImageFormat());
    renderPass.Begin(currentImageIndex, currentBuffer, extent, clearValues);
}

void VKRenderer::EndDrawing()
{
    const VkExtent2D &extent = vulkanState.swapchain.GetExtent();
    const VkCommandBuffer &currentBuffer = vulkanState.commands.GetBuffer(currentFrame);

    renderPass.End(currentBuffer);

    clearValues[0].color = ConvertClearColor(screenBackgroundR, screenBackgroundG,
        screenBackgroundB, vulkanState.swapchain.GetImageFormat());
    screenRenderPass.Begin(currentImageIndex, currentBuffer, extent, clearValues);
    screenPipeline.Bind(currentBuffer, currentFrame);

    screenModel.Draw(currentBuffer);

    screenRenderPass.End(currentBuffer);

    vulkanState.commands.EndBuffer(currentFrame);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentBuffer;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(vulkanState.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) !=
        VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vulkanState.swapchain.GetSwapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &currentImageIndex;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        WaitWhileMinimized();
        int32_t width;
        int32_t height;
        SDL_Vulkan_GetDrawableSize(window, &width, &height);
        vulkanState.swapchain.Recreate(vulkanState.allocator, vulkanState.device,
                                       vulkanState.physicalDevice, vulkanState.surface, width,
                                       height);
        HandleResize();
    }
    else if (result != VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % vulkanState.maxFramesInFlight;
}

SpriteBatch VKRenderer::CreateSpriteBatch(const std::string &texturePath, uint32_t maxSprites)
{
    Image textureImage = Image::CreateTexture(texturePath, vulkanState.allocator, vulkanState.commands, vulkanState.graphicsQueue, vulkanState.device, false);
    VkImageView textureImageView = textureImage.CreateTextureView(vulkanState.device);
    VkSampler textureSampler = textureImage.CreateTextureSampler(vulkanState.physicalDevice, vulkanState.device, VK_FILTER_NEAREST, VK_FILTER_NEAREST);

    int32_t textureWidth = static_cast<uint32_t>(textureImage.GetWidth());
    int32_t textureHeight = static_cast<uint32_t>(textureImage.GetHeight());

    auto spriteBatch = SpriteBatch(textureWidth, textureHeight, maxSprites);

    Pipeline pipeline;
    pipeline.CreateDescriptorSetLayout(
        vulkanState.device, [&](std::vector<VkDescriptorSetLayoutBinding> &bindings)
        {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = 1;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 1;
            samplerLayoutBinding.descriptorCount = 1;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.pImmutableSamplers = nullptr;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            bindings.push_back(uboLayoutBinding);
            bindings.push_back(samplerLayoutBinding); });
    pipeline.CreateDescriptorPool(
        vulkanState.maxFramesInFlight, vulkanState.device,
        [&](std::vector<VkDescriptorPoolSize> poolSizes)
        {
            poolSizes.resize(2);
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
        });
    pipeline.CreateDescriptorSets(
        vulkanState.maxFramesInFlight, vulkanState.device,
        [&](std::vector<VkWriteDescriptorSet> &descriptorWrites, VkDescriptorSet descriptorSet,
            uint32_t i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = ubo.GetBuffer(i);
            bufferInfo.offset = 0;
            bufferInfo.range = ubo.GetDataSize();

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;

            descriptorWrites.resize(2);

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSet;
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSet;
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(vulkanState.device,
                                   static_cast<uint32_t>(descriptorWrites.size()),
                                   descriptorWrites.data(), 0, nullptr);
        });
    pipeline.Create<VertexData, InstanceData>("res/VKSprite.vert.spv", "res/VKSprite.frag.spv",
                                              vulkanState.device, renderPass, false);

    Model<VertexData, uint32_t, InstanceData> model = Model<VertexData, uint32_t, InstanceData>::Create(1, vulkanState.allocator,
                                                                                                        vulkanState.commands, vulkanState.graphicsQueue, vulkanState.device);
    model.UpdateInstances({InstanceData{}}, vulkanState.commands, vulkanState.allocator, vulkanState.graphicsQueue, vulkanState.device);

    VKSpriteBatchData spriteBatchData{
        textureImage,
        textureImageView,
        textureSampler,
        pipeline,
        model,
    };

    spriteBatchDatas.insert(std::make_pair(spriteBatch.GetId(), spriteBatchData));

    return spriteBatch;
}

void VKRenderer::DrawSpriteBatch(SpriteBatch &spriteBatch)
{
    if (spriteBatchDatas.find(spriteBatch.GetId()) == spriteBatchDatas.end())
    {
        return;
    }

    auto &spriteBatchData = spriteBatchDatas.at(spriteBatch.GetId());

    const VertexData *vertices = reinterpret_cast<const VertexData *>(&spriteBatch.GetVertices()[0]);
    size_t vertexCount = spriteBatch.GetSpriteCount() * verticesPerSprite;
    size_t indexCount = spriteBatch.GetSpriteCount() * indicesPerSprite;

    spriteBatchData.model.Update(vertices, &spriteBatch.GetIndices()[0], vertexCount, indexCount, vulkanState.commands, vulkanState.allocator, vulkanState.graphicsQueue, vulkanState.device);

    const VkCommandBuffer &currentBuffer = vulkanState.commands.GetBuffer(currentFrame);

    spriteBatchData.pipeline.Bind(currentBuffer, currentFrame);

    spriteBatchData.model.Draw(currentBuffer);
}

void VKRenderer::DestroySpriteBatch(SpriteBatch &spriteBatch)
{
    if (spriteBatchDatas.find(spriteBatch.GetId()) == spriteBatchDatas.end())
    {
        return;
    }

    auto &spriteBatchData = spriteBatchDatas.at(spriteBatch.GetId());

    vkDeviceWaitIdle(vulkanState.device);
    spriteBatchData.Cleanup(vulkanState.device, vulkanState.allocator);

    spriteBatchDatas.erase(spriteBatch.GetId());
}

void VKRenderer::InitWindow(const std::string &windowName)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        RUNTIME_ERROR("Unable to initialize SDL!");
    }

    if (SDL_Vulkan_LoadLibrary(NULL))
    {
        RUNTIME_ERROR("Unable to initialize Vulkan!");
    }

    window = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              windowWidth, windowHeight,
                              SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
}

void VKRenderer::InitVulkan(const uint32_t maxFramesInFlight)
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateAllocator();

    int32_t width;
    int32_t height;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);

    vulkanState.maxFramesInFlight = maxFramesInFlight;

    vulkanState.swapchain.Create(vulkanState.device, vulkanState.physicalDevice,
                                 vulkanState.surface, width, height, enableVsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR);
    vulkanState.commands.CreatePool(vulkanState.physicalDevice, vulkanState.device,
                                    vulkanState.surface);
    vulkanState.commands.CreateBuffers(vulkanState.device, vulkanState.maxFramesInFlight);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.maxLod = 1.0f;

    if (vkCreateSampler(vulkanState.device, &samplerInfo, nullptr, &screenColorSampler) !=
        VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to create color sampler!");
    }

    ubo.Create(vulkanState.maxFramesInFlight, vulkanState.allocator);

    UniformBufferData uboData{};
    uboData.proj = VkOrtho(0.0f, static_cast<float>(viewWidth), 0.0f,
                           static_cast<float>(viewHeight), zNear, zFar);

    ubo.Update(uboData);

    screenUbo.Create(vulkanState.maxFramesInFlight, vulkanState.allocator);

    renderPass.CreateCustom(
        vulkanState.device, vulkanState.swapchain,
        [&]
        {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkFormat depthFormat = renderPass.FindDepthFormat(vulkanState.physicalDevice);
            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = depthFormat;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthAttachmentRef{};
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            std::array<VkSubpassDependency, 2> dependencies;

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[0].dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            VkRenderPass renderPass;

            if (vkCreateRenderPass(vulkanState.device, &renderPassInfo, nullptr, &renderPass) !=
                VK_SUCCESS)
            {
                RUNTIME_ERROR("Failed to create render pass!");
            }

            return renderPass;
        },
        [&](const VkExtent2D &extent)
        {
            screenColorImage = Image(vulkanState.allocator, extent.width, extent.height,
                                     VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                     VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            screenColorImageView =
                screenColorImage.CreateView(VK_IMAGE_ASPECT_COLOR_BIT, vulkanState.device);

            VkFormat depthFormat = renderPass.FindDepthFormat(vulkanState.physicalDevice);
            screenDepthImage = Image(vulkanState.allocator, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            screenDepthImageView = screenDepthImage.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT, vulkanState.device);
        },
        [=]
        {
            vkDestroyImageView(vulkanState.device, screenColorImageView, nullptr);
            screenColorImage.Destroy(vulkanState.allocator);

            vkDestroyImageView(vulkanState.device, screenDepthImageView, nullptr);
            screenDepthImage.Destroy(vulkanState.allocator);
        },
        [&](std::vector<VkImageView> &attachments, VkImageView imageView)
        {
            attachments.push_back(screenColorImageView);
            attachments.push_back(screenDepthImageView);
        });

    screenRenderPass.Create(vulkanState.physicalDevice, vulkanState.device, vulkanState.allocator, vulkanState.swapchain, true, false);

    screenPipeline.CreateDescriptorSetLayout(
        vulkanState.device, [&](std::vector<VkDescriptorSetLayoutBinding> &bindings)
        {
                VkDescriptorSetLayoutBinding uboLayoutBinding{};
                uboLayoutBinding.binding = 0;
                uboLayoutBinding.descriptorCount = 1;
                uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uboLayoutBinding.pImmutableSamplers = nullptr;
                uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

                VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                samplerLayoutBinding.binding = 1;
                samplerLayoutBinding.descriptorCount = 1;
                samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                samplerLayoutBinding.pImmutableSamplers = nullptr;
                samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                VkDescriptorSetLayoutBinding depthSamplerLayoutBinding{};
                depthSamplerLayoutBinding.binding = 2;
                depthSamplerLayoutBinding.descriptorCount = 1;
                depthSamplerLayoutBinding.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                depthSamplerLayoutBinding.pImmutableSamplers = nullptr;
                depthSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

                bindings.push_back(uboLayoutBinding);
                bindings.push_back(samplerLayoutBinding);
                bindings.push_back(depthSamplerLayoutBinding); });
    screenPipeline.CreateDescriptorPool(
        vulkanState.maxFramesInFlight, vulkanState.device,
        [&](std::vector<VkDescriptorPoolSize> poolSizes)
        {
            poolSizes.resize(3);
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
            poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[2].descriptorCount = static_cast<uint32_t>(vulkanState.maxFramesInFlight);
        });
    screenPipeline.CreateDescriptorSets(
        vulkanState.maxFramesInFlight, vulkanState.device,
        [&](std::vector<VkWriteDescriptorSet> &descriptorWrites, VkDescriptorSet descriptorSet,
            uint32_t i)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = screenUbo.GetBuffer(i);
            bufferInfo.offset = 0;
            bufferInfo.range = screenUbo.GetDataSize();

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = screenColorImageView;
            imageInfo.sampler = screenColorSampler;

            descriptorWrites.resize(2);

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSet;
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSet;
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(vulkanState.device,
                                   static_cast<uint32_t>(descriptorWrites.size()),
                                   descriptorWrites.data(), 0, nullptr);
        });
    screenPipeline.Create<VertexData, InstanceData>("res/VKScreen.vert.spv",
                                                    "res/VKScreen.frag.spv",
                                                    vulkanState.device, screenRenderPass, false);

    clearValues.resize(2);
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    screenModel = Model<VertexData, uint32_t, InstanceData>::FromVerticesAndIndices(screenVertices, screenIndices, 1, vulkanState.allocator, vulkanState.commands, vulkanState.graphicsQueue, vulkanState.device);
    screenModel.UpdateInstances({InstanceData{}}, vulkanState.commands, vulkanState.allocator, vulkanState.graphicsQueue, vulkanState.device);

    CreateSyncObjects();
}

void VKRenderer::CreateAllocator()
{
    VmaVulkanFunctions vkFuncs = {};

    vkFuncs.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vkFuncs.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo aci = {};
    aci.vulkanApiVersion = VK_API_VERSION_1_2;
    aci.physicalDevice = vulkanState.physicalDevice;
    aci.device = vulkanState.device;
    aci.instance = instance;
    aci.pVulkanFunctions = &vkFuncs;

    vmaCreateAllocator(&aci, &vulkanState.allocator);
}

void VKRenderer::WaitWhileMinimized()
{
    int32_t width = 0;
    int32_t height = 0;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);
    while (width == 0 || height == 0)
    {
        SDL_Vulkan_GetDrawableSize(window, &width, &height);
        SDL_WaitEvent(nullptr);
    }
}

VKRenderer::~VKRenderer()
{
    vkDeviceWaitIdle(vulkanState.device);

    vulkanState.swapchain.Cleanup(vulkanState.allocator, vulkanState.device);

    for (auto it = spriteBatchDatas.begin(); it != spriteBatchDatas.end(); it++)
    {
        it->second.Cleanup(vulkanState.device, vulkanState.allocator);
    }

    screenPipeline.Cleanup(vulkanState.device);
    renderPass.Cleanup(vulkanState.allocator, vulkanState.device);
    screenRenderPass.Cleanup(vulkanState.allocator, vulkanState.device);

    ubo.Destroy(vulkanState.allocator);
    screenUbo.Destroy(vulkanState.allocator);

    vkDestroySampler(vulkanState.device, screenColorSampler, nullptr);

    screenModel.Destroy(vulkanState.allocator);

    vmaDestroyAllocator(vulkanState.allocator);

    for (size_t i = 0; i < vulkanState.maxFramesInFlight; i++)
    {
        vkDestroySemaphore(vulkanState.device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(vulkanState.device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(vulkanState.device, inFlightFences[i], nullptr);
    }

    vulkanState.commands.Destroy(vulkanState.device);

    vkDestroyDevice(vulkanState.device, nullptr);

    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, vulkanState.surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

void VKRenderer::CreateInstance()
{
    if (enableValidationLayers && !CheckValidationLayerSupport())
    {
        RUNTIME_ERROR("Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "PxlRnd";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "PxlRnd";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to create instance!");
    }
}

void VKRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

void VKRenderer::SetupDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) !=
        VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to set up debug messenger!");
    }
}

void VKRenderer::CreateSurface()
{
    if (!SDL_Vulkan_CreateSurface(window, instance, &vulkanState.surface))
    {
        RUNTIME_ERROR("Failed to create window surface!");
    }
}

void VKRenderer::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        RUNTIME_ERROR("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &device : devices)
    {
        if (IsDeviceSuitable(device))
        {
            vulkanState.physicalDevice = device;
            break;
        }
    }

    if (vulkanState.physicalDevice == VK_NULL_HANDLE)
    {
        RUNTIME_ERROR("Failed to find a suitable GPU!");
    }
}

void VKRenderer::CreateLogicalDevice()
{
    QueueFamilyIndices indices =
        QueueFamilyIndices::FindQueueFamilies(vulkanState.physicalDevice, vulkanState.surface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(vulkanState.physicalDevice, &createInfo, nullptr, &vulkanState.device) !=
        VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to create logical device!");
    }

    vkGetDeviceQueue(vulkanState.device, indices.graphicsFamily.value(), 0,
                     &vulkanState.graphicsQueue);
    vkGetDeviceQueue(vulkanState.device, indices.presentFamily.value(), 0, &presentQueue);
}

bool VKRenderer::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t VKRenderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vulkanState.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    RUNTIME_ERROR("Failed to find suitable memory type!");
}

void VKRenderer::CreateSyncObjects()
{
    imageAvailableSemaphores.resize(vulkanState.maxFramesInFlight);
    renderFinishedSemaphores.resize(vulkanState.maxFramesInFlight);
    inFlightFences.resize(vulkanState.maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < vulkanState.maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(vulkanState.device, &semaphoreInfo, nullptr,
                              &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(vulkanState.device, &semaphoreInfo, nullptr,
                              &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(vulkanState.device, &fenceInfo, nullptr, &inFlightFences[i]) !=
                VK_SUCCESS)
        {
            RUNTIME_ERROR("Failed to create synchronization objects for a frame!");
        }
    }
}

bool VKRenderer::IsDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = QueueFamilyIndices::FindQueueFamilies(device, vulkanState.surface);

    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapchainSupportDetails swapchainSupport =
            vulkanState.swapchain.QuerySupport(device, vulkanState.surface);
        swapChainAdequate =
            !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.IsComplete() && extensionsSupported && swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

bool VKRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

std::vector<const char *> VKRenderer::GetRequiredExtensions()
{
    uint32_t extensionCount = 0;
    std::vector<const char *> extensions;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr))
    {
        RUNTIME_ERROR("Unable to get Vulkan extensions!");
    }
    extensions.resize(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, &extensions[0]))
    {
        RUNTIME_ERROR("Unable to get Vulkan extensions!");
    }

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VKRenderer::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VKRenderer::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData)
{
    std::cerr << "Validation layer: " << callbackData->pMessage << std::endl;

    return VK_FALSE;
}

// An alternative to glm::ortho that is designed to work with Vulkan.
// The normal glm projection matrix functions have unexpected behavior.
glm::mat4 VKRenderer::VkOrtho(float left, float right, float bottom, float top,
                              float near, float far)
{
    glm::mat4 ortho;
    ortho[0][0] = 2.0f / (right - left);
    ortho[0][1] = 0.0f;
    ortho[0][2] = 0.0f;
    ortho[0][3] = 0.0f;

    ortho[1][0] = 0.0f;
    ortho[1][1] = 2.0f / (bottom - top);
    ortho[1][2] = 0.0f;
    ortho[1][3] = 0.0f;

    ortho[2][0] = 0.0f;
    ortho[2][1] = 0.0f;
    ortho[2][2] = -1.0f / (near - far);
    ortho[2][3] = 0.0f;

    ortho[3][0] = -(right + left) / (right - left);
    ortho[3][1] = -(bottom + top) / (bottom - top);
    ortho[3][2] = near / (near - far);
    ortho[3][3] = 1.0f;

    return ortho;
}

VkClearColorValue VKRenderer::ConvertClearColor(float r, float g, float b, VkFormat format)
{
    if (format == VK_FORMAT_B8G8R8A8_SRGB || format == VK_FORMAT_R8G8B8_SRGB)
    {
        return {{glm::pow(r, 2.2f), glm::pow(g, 2.2f), glm::pow(b, 2.2f), 1.0f}};
    }

    return {{r, g, b, 1.0f}};
}