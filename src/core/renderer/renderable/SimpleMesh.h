//
// Created by Viktor Aylor on 24/07/2024.
//

#ifndef SIMPLEMESH_H
#define SIMPLEMESH_H
#include "Mesh.h"
#include "vector"
#include "XTPVulkan.h"


template <class VERTEX_TYPE> class SimpleMesh final : public Mesh {
public:
    AllocatedBuffer vertexBuffer;
    AllocatedBuffer indexBuffer;
    std::vector<VERTEX_TYPE> vertices;
    std::vector<uint32_t> indices;
    bool hasInitialized = false;

    SimpleMesh(std::vector<VERTEX_TYPE> vertices, const std::vector<uint32_t> &indices): vertexBuffer(), indexBuffer(),
                                                                                         vertices(vertices),
                                                                                         indices(indices) {
    }

    void init() override {
        vertexBuffer = XTPVulkan::createBufferWithDataStaging(vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
        indexBuffer = XTPVulkan::createBufferWithDataStaging(indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        hasInitialized = true;
    }

    bool initialized() override {
        return hasInitialized;
    }

    bool destroyed() override {
        return hasInitialized;
    }

    void destroy() override {
    }

    Vertex getFirstVertex() override {
        return vertices[0];
    }

    AllocatedBuffer getVertexBuffer() override {
        return vertexBuffer;
    }

    AllocatedBuffer getIndexBuffer() override {
        return indexBuffer;
    }

    uint32_t getVertexCount() override {
        return vertices.size();
    }

    uint32_t getIndexCount() override {
        return indices.size();
    }
};



#endif //SIMPLEMESH_H
