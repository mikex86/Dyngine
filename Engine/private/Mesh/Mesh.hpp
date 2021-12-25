#pragma once

#include <LLGL/LLGL.h>
#include <optional>

enum MeshRenderMode {
    POINTS,
    LINES,
    LINE_LOOP,
    LINE_STRIP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
};


class Mesh {
private:
    std::shared_ptr<LLGL::RenderSystem> renderSystem;
public:
    MeshRenderMode meshRenderMode;
    uint32_t numVertices;
    uint32_t numIndices;
    // nullable
    LLGL::Buffer *indexBuffer;
    std::vector<LLGL::Buffer *> buffers;
    LLGL::BufferArray *bufferArray;
    std::vector<LLGL::VertexFormat> vertexFormats;

    Mesh(MeshRenderMode meshRenderMode, std::shared_ptr<LLGL::RenderSystem> renderSystem, std::optional<uint32_t> numVertices, std::optional<uint32_t> numIndices,
         LLGL::Buffer *indexBuffer, const std::vector<LLGL::Buffer *> &buffers, LLGL::BufferArray *bufferArray,
         const std::vector<LLGL::VertexFormat> &vertexFormats);

    virtual ~Mesh();
};

