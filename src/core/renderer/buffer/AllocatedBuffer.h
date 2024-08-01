//
// Created by Viktor Aylor on 14/07/2024.
//

#ifndef ALLOCATEDBUFFER_H
#define ALLOCATEDBUFFER_H

#include "vk_mem_alloc.h"

enum BindableBufferUsage {
    INDEX_BUFFER,
    VERTEX_BUFFER,
    UNSUPPORTED
};

struct AllocatedBuffer {
    BindableBufferUsage usage;
    VkBuffer internalBuffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    bool alive = true;
    VkDeviceAddress gpuAddress;

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
#endif //ALLOCATEDBUFFER_H
