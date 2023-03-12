#pragma once

#include <cinttypes>

template <typename V, typename I, typename D>
class Model
{
public:
    static Model<V, I, D> FromVerticesAndIndices(const std::vector<V> &vertices,
                                                 const std::vector<I> indices,
                                                 const size_t maxInstances, VmaAllocator allocator,
                                                 Commands &commands, VkQueue graphicsQueue,
                                                 VkDevice device)
    {
        Model model = Create(maxInstances, allocator, commands, graphicsQueue, device);
        model.size = indices.size();

        model.indexBuffer =
            Buffer::FromIndices(allocator, commands, graphicsQueue, device, indices);
        model.vertexBuffer =
            Buffer::FromVertices(allocator, commands, graphicsQueue, device, vertices);

        return model;
    }

    static Model<V, I, D> Create(const size_t maxInstances, VmaAllocator allocator,
                                 Commands &commands, VkQueue graphicsQueue, VkDevice device)
    {
        Model model;

        size_t instanceByteSize = maxInstances * sizeof(D);
        model.instanceStagingBuffer =
            Buffer(allocator, instanceByteSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true);
        model.instanceBuffer =
            Buffer(allocator, instanceByteSize,
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, false);

        return model;
    };

    void Draw(VkCommandBuffer commandBuffer)
    {
        if (vertexBuffer.GetSize() == 0 || instanceBuffer.GetSize() == 0 ||
            indexBuffer.GetSize() == 0)
            return;

        if (instanceCount < 1)
            return;

        VkIndexType indexType = VK_INDEX_TYPE_UINT16;

        if (sizeof(I) == 4)
            indexType = VK_INDEX_TYPE_UINT32;

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.GetBuffer(), offsets);
        vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instanceBuffer.GetBuffer(), offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.GetBuffer(), 0, indexType);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(size),
                         static_cast<uint32_t>(instanceCount), 0, 0, 0);
    }

    void Update(const std::vector<V> &vertices, const std::vector<I> &indices, Commands &commands,
                VmaAllocator allocator, VkQueue graphicsQueue, VkDevice device)
    {
        size = indices.size();

        vkDeviceWaitIdle(device);

        indexBuffer.Destroy(allocator);
        vertexBuffer.Destroy(allocator);

        indexBuffer = Buffer::FromIndices(allocator, commands, graphicsQueue, device, indices);
        vertexBuffer = Buffer::FromVertices(allocator, commands, graphicsQueue, device, vertices);
    }

    // TODO: If this works back the vector version with this one
    void Update(const V *vertices, const I *indices, size_t vertexCount, size_t indexCount, Commands &commands,
                VmaAllocator allocator, VkQueue graphicsQueue, VkDevice device)
    {
        size = indexCount;

        vkDeviceWaitIdle(device);

        indexBuffer.Destroy(allocator);
        vertexBuffer.Destroy(allocator);

        indexBuffer = Buffer::FromIndices(allocator, commands, graphicsQueue, device, indices, indexCount);
        vertexBuffer = Buffer::FromVertices(allocator, commands, graphicsQueue, device, vertices, vertexCount);
    }

    void UpdateInstances(const std::vector<D> &instances, Commands &commands,
                         VmaAllocator allocator, VkQueue graphicsQueue, VkDevice device)
    {
        instanceCount = instances.size();
        instanceStagingBuffer.SetData(instances.data());
        instanceStagingBuffer.CopyTo(allocator, graphicsQueue, device, commands, instanceBuffer);
    }

    void Destroy(VmaAllocator allocator)
    {
        vertexBuffer.Destroy(allocator);
        indexBuffer.Destroy(allocator);
        instanceStagingBuffer.Destroy(allocator);
        instanceBuffer.Destroy(allocator);
    }

private:
    Buffer vertexBuffer;
    Buffer indexBuffer;
    Buffer instanceBuffer;
    Buffer instanceStagingBuffer;
    size_t size = 0;
    size_t instanceCount = 0;
};