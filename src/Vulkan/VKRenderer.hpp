#pragma once

#include "../Renderer.hpp"
#include <SDL2/SDL_vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
#include <vector>

#include "Buffer.hpp"
#include "Commands.hpp"
#include "Model.hpp"
#include "Pipeline.hpp"
#include "QueueFamilyIndices.hpp"
#include "Swapchain.hpp"
#include "UniformBuffer.hpp"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);

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

class VKRenderer : Renderer
{
public:
    VKRenderer(const std::string &windowName, int32_t windowWidth, int32_t windowHeight,
               int32_t viewWidth, int32_t viewHeight, bool enableVsync = true);
    ~VKRenderer();

    void ResizeWindow(int width, int height) override;
    SDL_Window *GetWindowPtr() override;

    void SetBackgroundColor(float r, float g, float b) override;
    void SetScreenBackgroundColor(float r, float g, float b) override;
    void BeginDrawing() override;
    void EndDrawing() override;

    SpriteBatch CreateSpriteBatch(const std::string &texturePath, uint32_t maxSprites) override;
    void DrawSpriteBatch(SpriteBatch &spriteBatch) override;
    void DestroySpriteBatch(SpriteBatch &spriteBatch) override;

private:
    SDL_Window *window;

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

    void InitWindow(const std::string &windowTitle, const uint32_t windowWidth,
                    const uint32_t windowHeight);

    void InitVulkan(const uint32_t maxFramesInFlight);
    void CreateInstance();
    void CreateAllocator();
    void CreateLogicalDevice();

    void MainLoop();
    void DrawFrame();
    void WaitWhileMinimized();

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

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData); // TODO: I don't prefix names with p
};