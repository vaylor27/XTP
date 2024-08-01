#ifndef SIMPLERENDERABLE_H
#define SIMPLERENDERABLE_H
#include "Renderable.h"

class SimpleRenderable: public Renderable {
public:

    bool initialized = false;
    bool uniformsDirty = true;

    ~SimpleRenderable() override = default;

    void init() override {
        createBuffers();
        initialized = true;
    }

    virtual void createBuffers() = 0;

    void markUniformBuffersDirty() {
        uniformsDirty = true;
    }

    bool hasInitialized() override {
        return initialized;
    }
};



#endif //SIMPLERENDERABLE_H
