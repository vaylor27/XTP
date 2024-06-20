
#ifndef XTPRENDERBACKEND_H
#define XTPRENDERBACKEND_H

class XTPRenderBackend {
public:

    ~XTPRenderBackend() = default;

    virtual void initializeBackend() = 0;

    virtual void cleanUp() = 0;
};

#endif //XTPRENDERBACKEND_H
