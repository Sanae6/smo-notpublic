#include <agl/utl.h>
#include <al/Library/Memory/HeapUtil.h>
#include <bonk/ForwardDecls.hpp>
#include <bonk/mods/Subscribe.hpp>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <helpers/fsHelper.h>
#include <logger/Logger.hpp>
#include <nvn.h>
#include <nvn_FuncPtrInline.h>

namespace bm {
    Subscribe::Subscribe() {
        agl::TextureData texData;
        texData.setDebugLabel("Subscribe");
        texData.initialize_(agl::TextureType::UNORM, static_cast<agl::TextureFormat>(0x1d), 98, 98, 1, 1,
                            static_cast<agl::TextureAttribute>(0), static_cast<agl::MultiSampleType>(0), true);
        Logger::log("Created and initialized texture %x, %x\n", texData.mSurface.storageSize,
                    texData.mSurface.storageAlignment);
        auto heap = al::getSequenceHeap();
        Logger::log("SequenceHeap: %zx/%zx\n", heap->getFreeSize(), heap->getSize());
        subscribeMemoryBlock.tryAllocBuffer_(texData.mSurface.storageSize, al::getSequenceHeap(),
                                             texData.mSurface.storageAlignment, static_cast<agl::MemoryAttribute>(8));
        auto addrBase = agl::GPUMemAddrBase(subscribeMemoryBlock, 0);
        Logger::log("Allocated texture memory %p %p\n", addrBase.mMemoryPool, subscribeMemoryBlock.mMemoryPool);
        auto mappedMemory = nvnMemoryPoolMap(&addrBase.mMemoryPool->pool);
        Logger::log("Mapped memory\n");
        auto loadData = FsHelper::LoadData{"content:/BonkData/Subscribe.raw", mappedMemory, 98 * 98 * 4};
        FsHelper::loadFileFromPath(loadData);
        agl::TextureDataInitializerRAW::initialize(
            &texData, agl::GPUMemVoidAddr(&addrBase), subscribeMemoryBlock.mMemBlockBufferSize,
            static_cast<agl::TextureFormat>(0x1d), 98, 98, al::getSequenceHeap());
        sampler.applyTextureData(texData);
        Logger::log("Initialized texture memory!\n");
    }

    void drawCursorMatrix(sead::Matrix34f& out, agl::TextureSampler& sampler, const sead::Vector2f& screenSize, const sead::Vector2f position,
                          float scaleMul = 0.5f) {
        u16 width = sampler.mTextureData.mSurface.mWidth;
        u16 height = sampler.mTextureData.mSurface.mHeight;
        out = sead::Matrix34f::ident;
        sead::Vector2f size(1.0f / (screenSize.x * 0.5f), 1.0f / (screenSize.y * 0.5f));
        out.m[0][0] = width * scaleMul * size.x;
        out.m[1][1] = height * scaleMul * size.y;
        out.m[0][3] = position.x + (width * 0.5f - 5.0f) * scaleMul * size.x;
        out.m[0][3] = position.y - (height * 0.5f - 3.0f) * scaleMul * size.y;
    }
    void Subscribe::renderLayout(StageScene* scene, agl::DrawContext* drawContext) {
        Logger::log("Rendering!\n");
        auto renderBuffer = scene->mDrawSystemInfo->getRenderBuffer();
        sead::Viewport viewport(*renderBuffer);

        sead::Matrix34f mat;
        sead::Vector2f screenSize = sead::Vector2f(viewport.getSizeX(), viewport.getSizeY());
        drawCursorMatrix(mat, sampler, screenSize,
                         sead::Vector2f(0.0f, 0.0f),
                         0.5f);

        agl::utl::DevTools::beginDrawImm(drawContext, sead::Matrix34<float>::ident, sead::Matrix44<float>::ident);
        agl::utl::DevTools::drawTexture(drawContext, sampler, mat, sead::Matrix44f::ident, sead::Color4f::cWhite);
    }

} // namespace bm
