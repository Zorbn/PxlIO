#include "Buffer.hpp"

Buffer::Buffer()
{
}

Buffer::Buffer(VmaAllocator allocator, vk::DeviceSize byteSize, VkBufferUsageFlags usage, bool cpuAccessible)
    : byteSize(byteSize)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = byteSize;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    if (cpuAccessible)
    {
        allocCreateInfo.flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }

    if (byteSize != 0 &&
        vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo) != VK_SUCCESS)
    {
        RUNTIME_ERROR("Failed to create buffer!");
    }
}

void Buffer::CopyTo(VmaAllocator &allocator, VkQueue graphicsQueue, VkDevice device, Commands &commands, Buffer &dst)
{
    if (byteSize == 0 || dst.GetSize() == 0)
        return;

    VkCommandBuffer commandBuffer = commands.BeginSingleTime(graphicsQueue, device);

    VkBufferCopy copyRegion{};
    copyRegion.size = dst.byteSize;
    vkCmdCopyBuffer(commandBuffer, buffer, dst.buffer, 1, &copyRegion);

    commands.EndSingleTime(commandBuffer, graphicsQueue, device);
}

const VkBuffer &Buffer::GetBuffer()
{
    return buffer;
}

size_t Buffer::GetSize()
{
    return byteSize;
}

void Buffer::Map(VmaAllocator allocator, void **data)
{
    if (byteSize == 0)
        return;

    vmaMapMemory(allocator, allocation, data);
}

void Buffer::Unmap(VmaAllocator allocator)
{
    if (byteSize == 0)
        return;

    vmaUnmapMemory(allocator, allocation);
}

void Buffer::Destroy(VmaAllocator &allocator)
{
    if (byteSize == 0)
        return;

    vmaDestroyBuffer(allocator, buffer, allocation);
}

void Buffer::SetData(const void *data)
{
    if (byteSize == 0)
        return;

    memcpy(allocInfo.pMappedData, data, byteSize);
}