
#ifndef SIMPLERENDERABLE_H
#define SIMPLERENDERABLE_H
#include "Renderable.h"
#include <vector>
#include <XTPVulkan.h>

#include "VulkanRenderInfo.h"


enum BufferUsage {
    INDEX_BUFFER,
    VERTEX_BUFFER,
    UNSUPPORTED
};

struct AllocatedBuffer {
    VkBuffer internalBuffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    BufferUsage usage;

    void destroy() const {
        vmaDestroyBuffer(XTPVulkan::allocator, internalBuffer, allocation);
    }

    void bind(const VkCommandBuffer& commandBuffer, int numBindings = 1, int firstBinding = 0) const {
        const VkBuffer vertexBuffers[] = {internalBuffer};
        constexpr VkDeviceSize offsets[] = {0};
        switch (usage) {
            case VERTEX_BUFFER : {
                vkCmdBindVertexBuffers(commandBuffer, firstBinding, numBindings, vertexBuffers, offsets);
                break;
            };
            case INDEX_BUFFER: {
            vkCmdBindIndexBuffer(commandBuffer, internalBuffer, 0, VK_INDEX_TYPE_UINT32);
            break;
            }
            case UNSUPPORTED: {
                throw std::runtime_error("");
            }
        }
    }
};

class SimpleRenderable: public Renderable {
public:

    bool initialized = false;

    ~SimpleRenderable() override = default;

    void init() override {
        createBuffers();
        initialized = true;
    }

    virtual void createBuffers() = 0;

    template <class T> static AllocatedBuffer createBuffer(std::vector<T> data, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
        return createBuffer(sizeof(data[0]) * data.size(), usage, memoryUsage);
    }

    static AllocatedBuffer createBuffer(const VkDeviceSize bufferSize, const VkBufferUsageFlags usage, const VmaMemoryUsage memoryUsage) {

        VkBufferCreateInfo bufferInfo {};

        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo vmaAllocInfo = {};
        vmaAllocInfo.usage = memoryUsage;
        vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        AllocatedBuffer vertexBuffer {};
        if(vmaCreateBuffer(XTPVulkan::allocator, &bufferInfo, &vmaAllocInfo, &vertexBuffer.internalBuffer, &vertexBuffer.allocation, &vertexBuffer.info) != VK_SUCCESS) {
            XTPVulkan::logger->logCritical("Unable To Create Buffer!");
        }
        vertexBuffer.usage = usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ? VERTEX_BUFFER: INDEX_BUFFER;

        return vertexBuffer;
    }

    template <class T> static AllocatedBuffer createBufferWithData(const std::vector<T>& data, VkBufferUsageFlagBits usage) {
        AllocatedBuffer buf {};
        const VkDeviceSize bufferSize = sizeof(data[0]) * data.size();

        buf = createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        const AllocatedBuffer staging = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
        void* map = staging.info.pMappedData;
        memcpy(map, data.data(), staging.info.size);

        copyBuffer(staging.internalBuffer, buf.internalBuffer, bufferSize);

        staging.destroy();

        return buf;
    }

    static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = XTPVulkan::commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(XTPVulkan::device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(XTPVulkan::graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(XTPVulkan::graphicsQueue); //TODO

        vkFreeCommandBuffers(XTPVulkan::device, XTPVulkan::commandPool, 1, &commandBuffer);

    }

    // ReSharper disable once CppNotAllPathsReturnValue
    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(XTPVulkan::gpu, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if (typeFilter & 1 << i && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        XTPVulkan::logger->logCritical("failed to find suitable memory type!");
    }

    bool hasInitialized() override {
        return initialized;
    }
};



#endif //SIMPLERENDERABLE_H
