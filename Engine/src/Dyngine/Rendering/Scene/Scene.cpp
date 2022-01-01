#include "Dyngine/Rendering/Scene/Scene.hpp"

Scene::Scene(ICamera &camera) : camera(camera) {
}

uint64_t Scene::addLight(Light light) {
    auto lightId = nextLightId++;
    light.id = lightId;
    auto sharedPtr = std::make_shared<Light>(light);
    lights[lightId] = sharedPtr;
    lightsChanged = true;
    return lightId;
}

uint64_t Scene::addAsset(std::unique_ptr<Asset> &asset) {
    auto assetId = nextAssetId++;
    asset->id = assetId;
    std::shared_ptr<Asset> sharedPtr = std::move(asset);
    assets[assetId] = sharedPtr;
    newAssets.push(sharedPtr);
    return assetId;
}

std::shared_ptr<Light> Scene::findLightById(uint64_t lightId) {
    auto it = lights.find(lightId);
    if (it == lights.end()) {
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<Asset> Scene::findAssetById(uint64_t assetId) {
    auto it = assets.find(assetId);
    if (it == assets.end()) {
        return nullptr;
    }
    return it->second;
}

std::queue<std::shared_ptr<Asset>> &Scene::getNewAssets() {
    return newAssets;
}

ICamera &Scene::getCamera() const {
    return camera;
}

const std::unordered_map<uint64_t, std::shared_ptr<Light>> &Scene::getLights() const {
    return lights;
}

bool Scene::haveLightsChanged() const {
    return lightsChanged;
}
