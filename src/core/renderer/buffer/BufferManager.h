#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H
#include <vector>

#include "VulkanRenderInfo.h"
#include "XTPVulkan.h"


template <class T> class BufferManager {
public:
    T bufferValue {};

    void markBuffersDirty() {
        for (auto &&shouldUpdateBuffer : shouldUpdateBuffers) {
            shouldUpdateBuffer = true;
        }
    }

    void onFrame(const uint32_t frameIndex) {
        if (shouldUpdateBuffers[frameIndex]) {
            //Recreate buffer if the size grew.
            if (size > buffers[frameIndex].info.size) {
                XTPVulkan::destroyAllocatedBuffer(&buffers[frameIndex]);
                buffers[frameIndex] = XTPVulkan::createSimpleBuffer(size, bufferUsage, memoryUsage, false);
            }
            memcpy(buffers[frameIndex].info.pMappedData, &bufferValue, size);
        }
    }

    void cleanUp() const {
        for (auto buffer : buffers) {
            XTPVulkan::destroyAllocatedBuffer(&buffer);
        }
    }

    [[nodiscard]] AllocatedBuffer* getBuffer(const uint32_t frameIndex) {
        return &buffers[frameIndex];
    }

    BufferManager(): size(0), bufferUsage(0), memoryUsage() {}

    explicit BufferManager(const uint32_t size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, uint32_t binding, uint32_t set): bufferUsage(bufferUsage), memoryUsage(memoryUsage), size(size) {
        const uint32_t maxFramesInFlight = VulkanRenderInfo::INSTANCE->getMaxFramesInFlight();

        shouldUpdateBuffers = std::vector<bool>(maxFramesInFlight);
        buffers = std::vector<AllocatedBuffer>(maxFramesInFlight);

        for (int i = 0; i < maxFramesInFlight; ++i) {
            shouldUpdateBuffers[i] = true;

            buffers[i] = XTPVulkan::createSimpleBuffer(size, bufferUsage, memoryUsage, false);
        }
    }

private:
    std::vector<AllocatedBuffer> buffers {};
    uint32_t size;
    VkBufferUsageFlags bufferUsage;
    VmaMemoryUsage memoryUsage;
    std::vector<bool> shouldUpdateBuffers {};
};



#endif //BUFFERMANAGER_H
