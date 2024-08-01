
#ifndef XTPDISPLAY_H
#define XTPDISPLAY_H
#include <memory>
#include <mutex>

#include "XTPWindowBackend.h"
#include "XTPWindowData.h"
#include "glm/vec2.hpp"

class XTPWindowing {
public:
    static std::unique_ptr<XTPWindowBackend> windowBackend;
    static std::unique_ptr<XTPWindowData> data;

    static void setWindowBackend(std::unique_ptr<XTPWindowBackend> backend);

    static void setWindowData(std::unique_ptr<XTPWindowData> backend);
};



#endif //XTPDISPLAY_H
