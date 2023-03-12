#pragma once

#include <vk_mem_alloc.h>

#include <stdexcept>
#include <vector>

#include "Commands.hpp"
#include "QueueFamilyIndices.hpp"
#include "../Error.hpp"

class Buffer {
public:
    template <typename T>
    static Buffer FromIndices(VmaAllocator allocator, Commands& commands, VkQueue graphicsQueue,
                              VkDevice device, const std::vector<T>& indices) {
        size_t indexSize = sizeof(indices[0]);

        // Only accept 16 or 32 bit types.
        if (indexSize != 2 && indexSize != 4) {
            RUNTIME_ERROR(
                "Incorrect size when creating index buffer, indices should be 16 or 32 bit!");
        }

        VkDeviceSize bufferByteSize = indexSize * indices.size();

        Buffer stagingBuffer(allocator, bufferByteSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
        stagingBuffer.SetData(indices.data());

        Buffer indexBuffer(allocator, bufferByteSize,
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                           false);

        stagingBuffer.CopyTo(allocator, graphicsQueue, device, commands, indexBuffer);
        stagingBuffer.Destroy(allocator);

        return indexBuffer;
    }

    template <typename T>
    static Buffer FromVertices(VmaAllocator allocator, Commands& commands, VkQueue graphicsQueue,
                               VkDevice device, const std::vector<T>& vertices) {
        VkDeviceSize bufferByteSize = sizeof(vertices[0]) * vertices.size();

        Buffer stagingBuffer(allocator, bufferByteSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
        stagingBuffer.SetData(vertices.data());

        Buffer vertexBuffer(allocator, bufferByteSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                            false);

        stagingBuffer.CopyTo(allocator, graphicsQueue, device, commands, vertexBuffer);
        stagingBuffer.Destroy(allocator);

        return vertexBuffer;
    }

    Buffer();
    Buffer(VmaAllocator allocator, VkDeviceSize byteSize, VkBufferUsageFlags usage,
           bool cpuAccessible);
    void Destroy(VmaAllocator& allocator);
    void SetData(const void* data);
    void CopyTo(VmaAllocator& allocator, VkQueue graphicsQueue, VkDevice device, Commands& commands,
                Buffer& dst);
    const VkBuffer& GetBuffer();
    size_t GetSize();
    void Map(VmaAllocator allocator, void** data);
    void Unmap(VmaAllocator allocator);

private:
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocInfo;
    size_t byteSize = 0;
};
