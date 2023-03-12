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

// TODO: Support view width/view height
// TODO: Support vsync toggle
VKRenderer::VKRenderer(const std::string &windowName, int32_t windowWidth, int32_t windowHeight,
                       int32_t viewWidth, int32_t viewHeight, bool enableVsync)
{
    InitWindow(windowName, windowWidth, windowHeight);
    InitVulkan(maxFramesInFlight, viewWidth, viewHeight);
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
    // TODO
}

void VKRenderer::SetScreenBackgroundColor(float r, float g, float b)
{
    // TODO
}

void VKRenderer::HandleResize()
{
    renderPass.Recreate(vulkanState.physicalDevice, vulkanState.device, vulkanState.allocator,
                        vulkanState.swapchain);
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
        // resizeCallback(vulkanState, width, height); CALLBACK: RESIZE
        HandleResize();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        RUNTIME_ERROR("Failed to acquire swap chain image!");
    }

    vkResetFences(vulkanState.device, 1, &inFlightFences[currentFrame]);

    vulkanState.commands.ResetBuffer(currentImageIndex, currentFrame);
}

void VKRenderer::EndDrawing()
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    const VkCommandBuffer &currentBuffer = vulkanState.commands.GetBuffer(currentFrame);

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
        // resizeCallback(vulkanState, width, height); CALLBACK: RESIZE
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
    // TODO

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

    VKSpriteBatchData spriteBatchData{
        textureImage,
        textureImageView,
        textureSampler,
        pipeline,
    };

    spriteBatchDatas.insert(std::make_pair(spriteBatch.Id(), spriteBatchData));

    return spriteBatch;
}

void VKRenderer::DrawSpriteBatch(SpriteBatch &spriteBatch)
{
    // TODO
    if (spriteBatchDatas.find(spriteBatch.Id()) == spriteBatchDatas.end())
    {
        return;
    }

    auto spriteBatchData = spriteBatchDatas.at(spriteBatch.Id());

    const VertexData *vertices = reinterpret_cast<const VertexData *>(&spriteBatch.Vertices()[0]);
    size_t vertexCount = spriteBatch.SpriteCount() * verticesPerSprite;
    size_t indexCount = spriteBatch.SpriteCount() * indicesPerSprite;

    spriteModel.Update(vertices, &spriteBatch.Indices()[0], vertexCount, indexCount, vulkanState.commands, vulkanState.allocator, vulkanState.graphicsQueue, vulkanState.device);

    // TODO: Move this command buffer stuff to begin/end draw.
    const VkExtent2D &extent = vulkanState.swapchain.GetExtent();
    const VkCommandBuffer &currentBuffer = vulkanState.commands.GetBuffer(currentFrame);

    vulkanState.commands.BeginBuffer(currentFrame);

    renderPass.Begin(currentImageIndex, currentBuffer, extent, clearValues);
    spriteBatchData.pipeline.Bind(currentBuffer, currentFrame);

    spriteModel.Draw(currentBuffer);

    renderPass.End(currentBuffer);

    vulkanState.commands.EndBuffer(currentFrame);
}

void VKRenderer::DestroySpriteBatch(SpriteBatch &spriteBatch)
{
    // TODO
}

void VKRenderer::InitWindow(const std::string &windowName, const uint32_t windowWidth,
                            const uint32_t windowHeight)
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

// An alternative to glm::ortho that is designed to work with Vulkan.
// The normal glm projection matrix functions have unexpected behavior.
glm::mat4 VkOrtho(float left, float right, float bottom, float top,
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

void VKRenderer::InitVulkan(const uint32_t maxFramesInFlight, int32_t viewWidth, int32_t viewHeight)
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

    // CALLBACK: Init here
    // initCallback(vulkanState, window, width, height);
    vulkanState.swapchain.Create(vulkanState.device, vulkanState.physicalDevice,
                                 vulkanState.surface, width, height);
    vulkanState.commands.CreatePool(vulkanState.physicalDevice, vulkanState.device,
                                    vulkanState.surface);
    vulkanState.commands.CreateBuffers(vulkanState.device, vulkanState.maxFramesInFlight);

    renderPass.Create(vulkanState.physicalDevice, vulkanState.device, vulkanState.allocator, vulkanState.swapchain, true, true);

    clearValues.resize(2);
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    ubo.Create(vulkanState.maxFramesInFlight, vulkanState.allocator);

    UniformBufferData uboData{};
    uboData.proj = VkOrtho(0.0f, 640.0f, 0.0f,
                                480.0f, zNear, zFar);

    ubo.Update(uboData);

    spriteModel = Model<VertexData, uint32_t, InstanceData>::Create(1, vulkanState.allocator, vulkanState.commands, vulkanState.graphicsQueue, vulkanState.device);
    std::vector<InstanceData> instances = {InstanceData{}};
    spriteModel.UpdateInstances(instances, vulkanState.commands, vulkanState.allocator, vulkanState.graphicsQueue, vulkanState.device);

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

    // CALLBACK: Cleanup here.
    // cleanupCallback(vulkanState);

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
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}