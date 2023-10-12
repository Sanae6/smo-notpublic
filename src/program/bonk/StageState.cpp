#include <al/Library/Player/PlayerHolder.h>
#include <bonk/ForwardDecls.hpp>
#include <bonk/StageState.hpp>
#include <game/Player/PlayerActorHakoniwa.h>
#include <utils/Helpers.h>

namespace bm {
    bool StageState::isSceneWithMario(const al::ActorInitInfo& initInfo) {
        al::PlayerHolder* playerHolder = initInfo.mActorSceneInfo.mPlayerHolder;
        return playerHolder->getPlayerNum() > 0 && isSameType<PlayerActorHakoniwa>(playerHolder->tryGetPlayer(0));
    }

    void StageState::initAfterPlacementSceneObj(const al::ActorInitInfo&) {}
    void StageState::update() {

    }

    struct StageStateCreate : public exl::hook::impl::TrampolineHook<StageStateCreate> {
        static al::SceneObjHolder* Callback() {
            auto* holder = Orig();
            holder->setSceneObj(new StageState(), 0x40);
            return holder;
        }
    };

    void stageStatePatches() {
        StageStateCreate::InstallAtSymbol("_ZN15SceneObjFactory20createSceneObjHolderEv");
    }

} // namespace bm