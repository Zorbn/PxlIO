#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../Renderer.hpp"
#include <SDL2/SDL_vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vk_mem_alloc.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "Buffer.hpp"
#include "Commands.hpp"
#include "Model.hpp"
#include "Pipeline.hpp"
#include "QueueFamilyIndices.hpp"
#include "Swapchain.hpp"
#include "UniformBuffer.hpp"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);

struct VertexData
{
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec4 color;
    float tint;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(VertexData);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(VertexData, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(VertexData, texCoord);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(VertexData, color);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(VertexData, tint);

        return attributeDescriptions;
    }
};

struct InstanceData
{
    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(InstanceData);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        return attributeDescriptions;
    }
};

struct UniformBufferData
{
    alignas(16) glm::mat4 proj;
};

struct ScreenUniformBufferData
{
    alignas(16) glm::mat4 proj;
    alignas(8) glm::vec2 viewSize;
    alignas(8) glm::vec2 offset;
};

const std::vector<VertexData> screenVertices = {
    {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f},
    {{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f},
    {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f},
    {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f},
};

const std::vector<uint32_t> screenIndices = {0, 1, 2, 0, 2, 3};

struct VulkanState
{
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VmaAllocator allocator;
    Swapchain swapchain;
    Commands commands;
    uint32_t maxFramesInFlight;
};

struct VKSpriteBatchData
{
    Image textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;
    Pipeline pipeline;
    Model<VertexData, uint32_t, InstanceData> model;

    void Cleanup(VkDevice device, VmaAllocator allocator)
    {
        pipeline.Cleanup(device);
        vkDestroySampler(device, textureSampler, nullptr);
        vkDestroyImageView(device, textureImageView, nullptr);
        textureImage.Destroy(allocator);
        model.Destroy(allocator);
    }
};

class VKRenderer : public Renderer
{
  public:
    VKRenderer(const std::string &windowName, int32_t windowWidth, int32_t windowHeight, int32_t viewWidth,
               int32_t viewHeight, bool enableVsync = true);
    ~VKRenderer();

    void ResizeWindow(int width, int height) override;
    SDL_Window *GetWindowPtr() override;

    void SetBackgroundColor(float r, float g, float b) override;
    void SetScreenBackgroundColor(float r, float g, float b) override;
    void BeginDrawing() override;
    void EndDrawing() override;

    SpriteBatch CreateSpriteBatch(const std::string &texturePath, uint32_t maxSprites, bool smooth = false,
                                  bool enableBlending = false) override;
    void DrawSpriteBatch(SpriteBatch &spriteBatch) override;
    void DestroySpriteBatch(SpriteBatch &spriteBatch) override;

  private:
    SDL_Window *window = nullptr;
    int32_t windowWidth = 0;
    int32_t windowHeight = 0;
    int32_t viewWidth = 0;
    int32_t viewHeight = 0;
    bool enableVsync = false;

    float backgroundR = 0.0f;
    float backgroundG = 0.0f;
    float backgroundB = 0.0f;
    float screenBackgroundR = 0.0f;
    float screenBackgroundG = 0.0f;
    float screenBackgroundB = 0.0f;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkQueue presentQueue;

    VulkanState vulkanState;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    uint32_t currentImageIndex = 0;

    bool framebufferResized = false;

    Image screenColorImage;
    VkImageView screenColorImageView;
    VkSampler screenColorSampler;

    Image screenDepthImage;
    VkImageView screenDepthImageView;

    RenderPass renderPass;
    RenderPass screenRenderPass;
    std::vector<VkClearValue> clearValues;

    Pipeline screenPipeline;

    UniformBuffer<UniformBufferData> ubo;
    UniformBuffer<ScreenUniformBufferData> screenUbo;
    Model<VertexData, uint32_t, InstanceData> screenModel;
    std::unordered_map<uint32_t, VKSpriteBatchData> spriteBatchDatas;

    void InitWindow(const std::string &windowTitle);

    void InitVulkan(const uint32_t maxFramesInFlight);
    void CreateInstance();
    void CreateAllocator();
    void CreateLogicalDevice();

    void MainLoop();
    void DrawFrame();
    void WaitWhileMinimized();
    void HandleResize();

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void SetupDebugMessenger();

    void CreateSurface();

    void PickPhysicalDevice();

    bool HasStencilComponent(VkFormat format);

    void CreateUniformBuffers();

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void CreateSyncObjects();

    bool IsDeviceSuitable(VkPhysicalDevice device);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    std::vector<const char *> GetRequiredExtensions();
    bool CheckValidationLayerSupport();

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                        void *userData);

    static glm::mat4 VkOrtho(float left, float right, float bottom, float top, float near, float far);

    static VkClearColorValue ConvertClearColor(float r, float g, float b, VkFormat format);
};