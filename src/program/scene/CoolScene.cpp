#include <al/Library/Layout/LayoutKit.h>
#include <al/Library/Nerve/NerveSetupUtil.h>
#include <game/GameData/GameDataHolder.h>
#include <game/Scene/SceneObjFactory.h>
#include <logger/Logger.hpp>
#include <scene/CoolScene.hpp>

namespace seen {
    namespace {
        NERVE_DEF(CoolScene, Wait);
    }

    CoolScene::CoolScene(const char* name) : Scene(name) {

    }
    void CoolScene::init(const al::SceneInitInfo& initInfo) {
        Logger::log("Initialized\n");
        initNerve(&CoolSceneNrvWait::sInstance, 0);
        initDrawSystemInfo(initInfo);
        initSceneMsgCtrl();
        initScreenCoverCtrl();
        initSceneStopCtrl();
        initLayoutKit(initInfo);
        mLayoutKit->createExecuteDirector(0x80);
        initAndLoadStageResource(initInfo.mStageName, initInfo.mScenarioNo);
        initSceneObjHolder(SceneObjFactory::createSceneObjHolder());
        al::setSceneObj(this, (GameDataHolder*)initInfo.mGameDataHolderBase, 0x12);
        initLiveActorKit(initInfo, 0x400, 1, 2);
    }
    void CoolScene::drawMain() {

    }
    void CoolScene::exeWait() {}
} // namespace seen
