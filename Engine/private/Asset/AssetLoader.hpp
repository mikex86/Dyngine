#pragma once

#include <memory>
#include <LLGL/LLGL.h>
#include <Asset/Asset.hpp>
#include <Stream/DataReadStream.hpp>
#include <DAsset/Asset.hpp>

namespace AssetLoader {

    Asset *LoadAsset(std::shared_ptr<LLGL::RenderSystem> renderSystem, const std::unique_ptr<Stream::DataReadStream> &assetDataStream);

}