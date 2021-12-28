#pragma once

#include "Rendering/Scene/Light/Light.hpp"
#include "Rendering/Scene/Camera/Camera.hpp"
#include "Rendering/Scene/Asset/Asset.hpp"
#include <map>
#include <queue>

class Scene {

private:
    ICamera &camera;

    uint64_t nextLightId = 0;
    std::unordered_map<uint64_t, std::shared_ptr<Light>> lights;
    std::queue<std::shared_ptr<Light>> newLights;

    uint64_t nextAssetId = 0;
    std::unordered_map<uint64_t, std::shared_ptr<Asset>> assets;
    std::queue<std::shared_ptr<Asset>> newAssets;

public:
    explicit Scene(ICamera &camera);

    uint64_t addLight(Light light);

    // nullable
    std::shared_ptr<Light> findLightById(uint64_t lightId);

    uint64_t addAsset(std::unique_ptr<Asset> &asset);

    // nullable
    std::shared_ptr<Asset> findAssetById(uint64_t assetId);

    std::queue<std::shared_ptr<Asset>> &getNewAssets();

    ICamera &getCamera() const;
};