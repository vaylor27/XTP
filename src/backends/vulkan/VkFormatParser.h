
#ifndef VKFORMATPARSER_H
#define VKFORMATPARSER_H
#include <string>
#include <vulkan/vulkan.h>

class VkFormatParser {
public:
    static std::string toStringVkFormat(VkFormat format);
};



#endif //VKFORMATPARSER_H
