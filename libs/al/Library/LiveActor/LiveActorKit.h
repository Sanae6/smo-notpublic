#pragma once

#include <al/Library/HostIO/HioNode.h>
#include <basis/seadTypes.h>

namespace al {
    class ActorResourceHolder;
    class AreaObjDirector;
    class ExecuteDirector;
    class GravityHolder;
    class EffectSystem;
    class GraphicsSystemInfo;
    class ModelDrawBufferCounter;
    class ModelDrawBufferUpdater;
    class ExecuteAsyncExecutor;
    class ExecuteAsyncExecutor;
    class ModelDisplayListController;
    class LiveActorGroup;
    class CameraDirector;
    class ClippingDirector;
    class CollisionDirector;
    class ItemDirectorBase;
    class PlayerHolder;
    class HitSensorDirector;
    class ScreenPointDirector;
    class ShadowDirector;
    class StageSwitchDirector;
    class SwitchAreaDirector;
    class LiveActorGroup;
    class ProjectDemoDirector;
    class GamePadSystem;
    class PadRumbleDirector;
    class NatureDirector;
    class ModelGroup;
    class GraphicsInitArg;

    struct ExecuteSystemInitInfo {
        int anInt;
        u8 thing[4];
    };
    class LiveActorKit : public al::HioNode {
    private:
        s32 mMaxActorCount;
        al::ActorResourceHolder* mActorResourceHolder;
        al::AreaObjDirector* mAreaObjDirector;
        al::ExecuteDirector* mExecuteDirector;
        al::GravityHolder* mGravityHolder;
        al::EffectSystem* mEffectSystem;
        al::GraphicsSystemInfo* mGraphicsSystemInfo;
        al::ModelDrawBufferCounter* modelDrawBufferCounter;
        al::ModelDrawBufferUpdater* mModelDrawBufferUpdater;
        al::ExecuteAsyncExecutor* mExecuteAsyncExecutorCore1;
        al::ExecuteAsyncExecutor* mExecuteAsyncExecutorCore2;
        al::ModelDisplayListController* mModelDisplayListController;
        al::ExecuteSystemInitInfo mExecuteSystemInfo;
        al::LiveActorGroup* actorDrawingGroup;
        al::CameraDirector* mCameraDirector;
        al::ClippingDirector* mClippingDirector;
        al::CollisionDirector* mCollisionDirector;
        al::ItemDirectorBase* itemDirector;
        al::PlayerHolder* mPlayerHolder;
        al::HitSensorDirector* mHitSensorDirector;
        al::ScreenPointDirector* screenPointDirector;
        al::ShadowDirector* mShadowDirector;
        al::StageSwitchDirector* mStageSwitchDirector;
        al::SwitchAreaDirector* mSwitchAreaDirector;
        al::LiveActorGroup* allActorsGroup;
        al::ProjectDemoDirector* demoDirector;
        al::GamePadSystem* gamePadSystem;
        al::PadRumbleDirector* mPadRumbleDirector;
        al::NatureDirector* mNatureDirector;
        al::ModelGroup* mModelGroup;

    public:
        LiveActorKit(s32, s32);
        ~LiveActorKit();
        void init(s32);
        void initGraphics(const al::GraphicsInitArg&);
        void initHitSensorDirector();
        void initGravityHolder();
        void initShadowDirector();
        void initEffectSystem();
        void initSwitchAreaDirector(s32, s32);
        void initNatureDirector();
        void endInit();
        void update(const char*);
        void clearGraphicsRequest();
        void updateGraphics();
        void preDrawGraphics();
    };
} // namespace al