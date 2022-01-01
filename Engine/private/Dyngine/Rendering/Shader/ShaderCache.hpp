#pragma once

#include <string>
#include <map>
#include <LLGL/LLGL.h>
#include "Dpac/Dpac.hpp"

struct ShaderHandle {
    std::string resourcePath;
    const std::vector<LLGL::VertexAttribute> &vertexInputAttributes;
    const std::vector<LLGL::FragmentAttribute> &fragmentOutputAttributes;

    bool operator==(const ShaderHandle &rhs) const;

    bool operator!=(const ShaderHandle &rhs) const;
};

class ShaderHandleHasher {
public:
    std::size_t operator()(const ShaderHandle &handle) const;
};

struct ShaderUsageHandle {
protected:
    std::shared_ptr<LLGL::RenderSystem> renderSystem;
    uint64_t &shaderProgramReferenceCount;
    uint64_t *handleReferenceCount;
public:
    LLGL::ShaderProgram *shaderProgram;

    ShaderUsageHandle(std::shared_ptr<LLGL::RenderSystem> renderSystem, uint64_t &shaderReferenceCount, LLGL::ShaderProgram *shaderProgram);

    ShaderUsageHandle(ShaderUsageHandle &handle);

    virtual ~ShaderUsageHandle();

};

class ShaderCache {

private:
    std::unordered_map<ShaderHandle, LLGL::ShaderProgram *, ShaderHandleHasher> cache;
    std::unordered_map<LLGL::ShaderProgram *, uint64_t> referenceCounts;
    std::shared_ptr<LLGL::RenderSystem> renderSystem;
    Dpac::ReadOnlyArchive archive;

public:

    ShaderCache(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, const Dpac::ReadOnlyArchive &archive);

    ShaderUsageHandle getOrCompile(const ShaderHandle &handle);

private:
    LLGL::ShaderProgram *compile(const ShaderHandle &handle);
};