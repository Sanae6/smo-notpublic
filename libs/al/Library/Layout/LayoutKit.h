#pragma once

#include <agl/DrawContext.h>
#include <agl/RenderBuffer.h>

namespace nn::ui2d {
    class DrawInfo;
}

namespace al {
    class FontHolder;
    class ExecuteDirector;
    class EffectSystem;
    class LayoutSystem;

    class LayoutKit {
    private:
        al::FontHolder* mFontHolder;
        al::ExecuteDirector* mExecuteDirector;
        al::EffectSystem* mEffectSystem;
        al::LayoutSystem* mLayoutSystem;
        agl::DrawContext* mDrawContext;
        nn::ui2d::DrawInfo* mDrawInfo;

    public:
        LayoutKit(al::FontHolder*);
        ~LayoutKit();
        void createCameraParamForIcon();
        void createExecuteDirector(s32);
        void createEffectSystem();
        void endInit();
        void update();
        void draw(const char*) const;
        void drawList(const char*, const char*) const;
        void setLayoutSystem(al::LayoutSystem*);
        void setDrawContext(agl::DrawContext*);
    };

    void setRenderBuffer(LayoutKit* kit, const agl::RenderBuffer* renderBuffer);
    void executeUpdate(LayoutKit*);
    void executeUpdateList(LayoutKit*, const char*, const char*);
    void executeUpdateEffect(LayoutKit*);
    void executeDraw(const LayoutKit*, const char*);
    void executeDrawEffect(const LayoutKit*);
} // namespace al