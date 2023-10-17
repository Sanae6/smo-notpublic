#pragma once

#include <agl/TextureData.h>
#include <agl/TextureSampler.h>
#include <agl/gpu.h>
#include <bonk/Mod.hpp>
namespace bm {
    struct Subscribe : public Mod {
        agl::TextureSampler sampler;
        agl::GPUMemBlockBase subscribeMemoryBlock;
        Subscribe();
        void renderLayout(StageScene* scene, agl::DrawContext* drawContext) override;
    };
} // namespace bm
