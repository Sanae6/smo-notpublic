#include "StageState.h"
#include "ForwardDecls.h"
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/Placement/PlacementInfo.h>
#include <al/Library/Player/PlayerHolder.h>
#include <al/Library/Resource/ResourceHolder.h>
#include <game/Player/PlayerActorHakoniwa.h>
#include <lib.hpp>

bool StageState::isSceneWithMario(const al::ActorInitInfo& initInfo) {
    al::PlayerHolder* playerHolder = initInfo.mActorSceneInfo.mPlayerHolder;
    return playerHolder->getPlayerNum() > 0 && isSameType<PlayerActorHakoniwa>(playerHolder->tryGetPlayer(0));
}

void StageState::initAfterPlacementSceneObj(const al::ActorInitInfo& initInfo) {
    // Check for mario. If he's not here, then nothing matters.
    if (!isSceneWithMario(initInfo))
        return;

    al::ActorInitInfo myInitInfo;
    auto* res = al::findOrCreateResource("SystemData/FishPlacement", nullptr);

    al::ByamlIter iter(res->tryGetByml("Fish"));
    al::PlacementInfo placementInfo;
    placementInfo.set(iter, iter);
    myInitInfo.initViewIdSelf(&placementInfo, initInfo);

    fish = new (new u8[0x308]) Pukupuku("Pukupuku");
    al::initCreateActorWithPlacementInfo(fish, initInfo);
    fish->kill();
    al::setNerve(fish, &PukupukuNrvDoNothing::sInstance);

    fishSensor = al::getHitSensor(fish, "Body");

    auto* stateCtrl = fish->getNerveKeeper()->mStateCtrl;
    for (int i = 0; i < stateCtrl->mStateCount; i++) {
        auto state = stateCtrl->mStates[i];
        if (isSameType<PukupukuNrvBlowDownFromCapture>(state.nerve)) {
            fishBlowDownState = state.state;
        }
    }

    EXL_ASSERT(fishBlowDownState != nullptr, "Fish blow down nerve shouldn't be null!");
}
al::AreaObjGroup* StageState::getFishEnableAreaGroup(al::LiveActor* player) {
    return al::tryFindAreaObjGroup(player, "FishEnableArea");
}
bool StageState::isInFishEnableArea(al::AreaObjGroup* group, al::LiveActor* player) {
    auto& trans = al::getTrans(player);
    for (int i = 0; i < group->count; i++) {
        if (group->getAreaObj(i)->isInVolume(trans))
            return true;
    }
    return false;
}
al::AreaObj* StageState::getFishEnableAreaObj(al::AreaObjGroup* group, const sead::Vector3f& trans) {
    for (int i = 0; i < group->count; i++) {
        auto* area = group->getAreaObj(i);
        if (area->isInVolume(trans))
            return area;
    }
    return nullptr;
}

al::SceneObjHolder* StageStateCreate::Callback() {
    auto* holder = Orig();
    holder->setSceneObj(new StageState(), 0x40);
    return holder;
}
