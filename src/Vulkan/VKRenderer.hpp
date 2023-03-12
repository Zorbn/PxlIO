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
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator);

struct VulkanState {
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VmaAllocator allocator;
    Swapchain swapchain;
    Commands commands;
    uint32_t maxFramesInFlight;
};

class VKRenderer {
public:
    void
    Run(const std::string& windowTitle, const uint32_t windowWidth, const uint32_t windowHeight,
        const uint32_t maxFramesInFlight,
        std::function<void(VulkanState& vulkanState, SDL_Window* window, int32_t width,
                           int32_t height)>
            initCallback,
        std::function<void(VulkanState& vulkanState)> updateCallback,
        std::function<void(VulkanState& vulkanState, VkCommandBuffer commandBuffer,
                           uint32_t imageIndex, uint32_t currentFrame)>
            renderCallback,
        std::function<void(VulkanState& vulkanState, int32_t width, int32_t height)> resizeCallback,
        std::function<void(VulkanState& vulkanState)> cleanupCallback);

private:
    SDL_Window* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkQueue presentQueue;

    VulkanState vulkanState;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    bool framebufferResized = false;

    void InitWindow(const std::string& windowTitle, const uint32_t windowWidth,
                    const uint32_t windowHeight);

    static void FramebufferResizeCallback(SDL_Window* window, int width, int height);

    void InitVulkan(const uint32_t maxFramesInFlight,
                    std::function<void(VulkanState& vulkanState, SDL_Window* window, int32_t width,
                                       int32_t height)>
                        initCallback);
    void CreateInstance();
    void CreateAllocator();
    void createLogicalDevice();

    void MainLoop(std::function<void(VulkanState& vulkanState, VkCommandBuffer commandBuffer,
                                     uint32_t imageIndex, uint32_t currentFrame)>
                      renderCallback,
                  std::function<void(VulkanState& vulkanState)> updateCallback,
                  std::function<void(VulkanState& vulkanState, int32_t width, int32_t height)>
                      resizeCallback);
    void DrawFrame(std::function<void(VulkanState& vulkanState, VkCommandBuffer commandBuffer,
                                      uint32_t imageIndex, uint32_t currentFrame)>
                       renderCallback,
                   std::function<void(VulkanState& vulkanState, int32_t width, int32_t height)>
                       resizeCallback);
    void WaitWhileMinimized();

    void Cleanup(std::function<void(VulkanState& vulkanState)> cleanupCallback);

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
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
    std::vector<const char*> GetRequiredExtensions();
    bool CheckValidationLayerSupport();

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};