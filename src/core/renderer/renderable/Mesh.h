//
// Created by Viktor Aylor on 23/07/2024.
//

#ifndef MESH_H
#define MESH_H
#include "buffer/AllocatedBuffer.h"

struct Vertex {};

class Mesh {
public:
    virtual ~Mesh() = default;

    virtual void init() = 0;

    virtual bool initialized() = 0;
    
    virtual bool destroyed() = 0;

    virtual void destroy() = 0;

    virtual Vertex getFirstVertex() = 0;

    virtual AllocatedBuffer getVertexBuffer() = 0;

    virtual AllocatedBuffer getIndexBuffer() = 0;

    virtual uint32_t getVertexCount() = 0;

    virtual uint32_t getIndexCount() = 0;

    virtual void tick() {}
};



#endif //MESH_H
