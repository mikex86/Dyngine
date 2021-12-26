#include "Rendering/Material/Material.hpp"

Material::Material(const std::shared_ptr<LLGL::RenderSystem> &renderSystem) : renderSystem(renderSystem) {
}

Material::~Material() {
    renderSystem->Release(*albedoTexture);
    renderSystem->Release(*normalTexture);
    renderSystem->Release(*rmaTexture);

    renderSystem->Release(*albedoSampler);
    renderSystem->Release(*normalSampler);
    renderSystem->Release(*rmaSampler);
}