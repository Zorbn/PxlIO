#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Buffer.hpp"

template <typename T> class UniformBuffer
{
  public:
    void Create(const uint32_t maxFramesInFlight, VmaAllocator allocator)
    {
        VkDeviceSize bufferByteSize = sizeof(T);

        buffers.resize(maxFramesInFlight);
        buffersMapped.resize(maxFramesInFlight);

        for (size_t i = 0; i < maxFramesInFlight; i++)
        {
            buffers[i] = Buffer(allocator, bufferByteSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true);
            buffers[i].Map(allocator, &buffersMapped[i]);
        }
    }

    void Update(const T &data)
    {
        size_t bufferCount = buffersMapped.size();
        for (size_t i = 0; i < bufferCount; i++)
        {
            memcpy(buffersMapped[i], &data, sizeof(T));
        }
    }

    const VkBuffer &GetBuffer(uint32_t i)
    {
        return buffers[i].GetBuffer();
    }

    size_t GetDataSize()
    {
        return sizeof(T);
    }

    void Destroy(VmaAllocator allocator)
    {
        size_t bufferCount = buffers.size();
        for (size_t i = 0; i < bufferCount; i++)
        {
            buffers[i].Unmap(allocator);
            buffers[i].Destroy(allocator);
        }
    }

  private:
    std::vector<Buffer> buffers;
    std::vector<void *> buffersMapped;
};