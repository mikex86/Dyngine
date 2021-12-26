#include "Rendering/Shader/ShaderCache.hpp"
#include "Rendering/Shader/ShaderUtil.hpp"

bool ShaderHandle::operator==(const ShaderHandle &rhs) const {
    if (resourcePath != rhs.resourcePath) {
        return false;
    }
    if (vertexInputAttributes != rhs.vertexInputAttributes) {
        return false;
    }
    for (size_t i = 0; i < fragmentOutputAttributes.size(); i++) {
        if (fragmentOutputAttributes[i].name != rhs.fragmentOutputAttributes[i].name ||
            fragmentOutputAttributes[i].format != rhs.fragmentOutputAttributes[i].format ||
            fragmentOutputAttributes[i].systemValue != rhs.fragmentOutputAttributes[i].systemValue ||
            fragmentOutputAttributes[i].location != rhs.fragmentOutputAttributes[i].location) {
            return false;
        }
    }
    return true;
}

bool ShaderHandle::operator!=(const ShaderHandle &rhs) const {
    return !(rhs == *this);
}

std::size_t ShaderHandleHasher::operator()(const ShaderHandle &handle) const {
    std::size_t seed = 0;
    seed += 31 * std::hash<std::string>()(handle.resourcePath);
    for (const auto &item: handle.vertexInputAttributes) {
        seed += 31 * std::hash<std::string>()(item.name);
        seed += 31 * static_cast<uint32_t>(item.format);
        seed += 31 * static_cast<uint32_t>(item.systemValue);
        seed += 31 * item.slot;
        seed += 31 * item.offset;
        seed += 31 * item.location;
        seed += 31 * item.stride;
        seed += 31 * item.instanceDivisor;
    }
    for (const auto &item: handle.fragmentOutputAttributes) {
        seed += 31 * std::hash<std::string>()(item.name);
        seed += 31 * std::hash<LLGL::Format>()(item.format);
        seed += 31 * std::hash<LLGL::SystemValue>()(item.systemValue);
        seed += 31 * std::hash<std::uint32_t>()(item.location);
    }
    return seed;
}

ShaderUsageHandle::ShaderUsageHandle(std::shared_ptr<LLGL::RenderSystem> renderSystem, uint64_t &shaderReferenceCount,
                                     LLGL::ShaderProgram *shaderProgram)
        : renderSystem(renderSystem), shaderProgramReferenceCount(shaderReferenceCount), shaderProgram(shaderProgram) {
    shaderProgramReferenceCount++;
    handleReferenceCount = new uint64_t { 1 };
}

ShaderUsageHandle::ShaderUsageHandle(ShaderUsageHandle &handle) :
        renderSystem(handle.renderSystem),
        handleReferenceCount(handle.handleReferenceCount),
        shaderProgramReferenceCount(handle.shaderProgramReferenceCount),
        shaderProgram(handle.shaderProgram) {
    (*handleReferenceCount)++;
}

ShaderUsageHandle::~ShaderUsageHandle() {
    (*handleReferenceCount)--;
    if (*handleReferenceCount == 0) {
        delete handleReferenceCount;

        shaderProgramReferenceCount--;
        if (shaderProgramReferenceCount == 0) {
            renderSystem->Release(*shaderProgram);
        }
    }
}

ShaderUsageHandle ShaderCache::getOrCompile(const ShaderHandle &handle) {
    auto it = cache.find(handle);
    LLGL::ShaderProgram *shaderProgram;
    if (it != cache.end()) {
        shaderProgram = it->second;
    } else {
        shaderProgram = compile(handle);
        cache.insert({handle, shaderProgram});
    }
    return ShaderUsageHandle(renderSystem, referenceCounts[shaderProgram], shaderProgram);
}

LLGL::ShaderProgram *ShaderCache::compile(const ShaderHandle &handle) {
    return ShaderUtil::LoadDShaderPackage(*renderSystem, archive.getEntryStream(handle.resourcePath),
                                          handle.vertexInputAttributes,
                                          handle.fragmentOutputAttributes);
}

ShaderCache::ShaderCache(const std::shared_ptr<LLGL::RenderSystem> &renderSystem, const Dpac::ReadOnlyArchive &archive)
        : renderSystem(renderSystem), archive(archive) {
}