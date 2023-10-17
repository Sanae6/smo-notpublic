#pragma once

//#include "agl/DrawContext.h"
#include "agl/RenderBuffer.h"

namespace agl
{
    struct DrawContext;
} // namespace agl


namespace al
{
    struct DrawSystemInfo {
        agl::RenderBuffer* dockedRenderBuffer;
        agl::RenderBuffer* handheldRenderBuffer;
        bool isDocked;
        agl::DrawContext* drawContext;
        agl::RenderBuffer* getRenderBuffer() { return isDocked ? dockedRenderBuffer : handheldRenderBuffer; }
    };
} // namespace al
