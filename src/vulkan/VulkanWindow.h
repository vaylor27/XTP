
#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include <vulkan/vulkan.h>

#include "XTPWindowBackend.h"

class VulkanWindow: public XTPWindowBackend {
public:

    virtual bool createVulkanSurface(VkInstance instance, VkSurfaceKHR* surface) = 0;

    virtual const char ** getRequiredInstanceExtensions(uint32_t& count) = 0;
};



#endif //VULKANWINDOW_H
