
#ifndef ALLOCATEDIMAGE_H
#define ALLOCATEDIMAGE_H

#include <unordered_map>

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include "XTPVulkan.h"


struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VkImageView imageView;
    VkSampler sampler;
};



#endif //ALLOCATEDIMAGE_H
