
#ifndef XTPRENDERING_H
#define XTPRENDERING_H
#include "XTPRenderInfo.h"
#include "XTPRenderBackend.h"

class XTPRendering {
public:
    static XTPRenderBackend* renderBackend;
    static XTPRenderInfo* renderInfo;

    static void setRenderBackend(XTPRenderBackend* backend);
};



#endif //XTPRENDERING_H
