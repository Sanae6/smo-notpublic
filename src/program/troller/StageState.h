#pragma once

#pragma region State
#include "ForwardDecls.h"
#include "Helpers.h"
#include "Pukupuku.h"
#include <al/Library/LiveActor/ActorFlagFunction.h>
#include <al/Library/Player/PlayerHolder.h>
#include <game/Player/PlayerActorHakoniwa.h>
#include <hook/trampoline.hpp>

struct StageState : public al::ISceneObj {
    Pukupuku* fish = nullptr;
    al::HitSensor* fishSensor;
    al::NerveStateBase* fishBlowDownState;

    static bool isSceneWithMario(const al::ActorInitInfo& initInfo);
    static al::AreaObjGroup* getFishEnableAreaGroup(al::LiveActor* actor);
    static al::AreaObj* getFishEnableAreaObj(al::AreaObjGroup* group, const sead::Vector3f& trans);
    static bool isInFishEnableArea(al::AreaObjGroup* group, al::LiveActor* actor);
    static bool isInFishEnableArea(al::LiveActor* actor) {
        al::LiveActor* player = al::getPlayerActor(actor, 0);
        auto* group = getFishEnableAreaGroup(player);
        return group && isInFishEnableArea(group, player);
    }

    void initAfterPlacementSceneObj(const al::ActorInitInfo& initInfo) override;


    bool hasFish() const { return fish; }
    bool isFishActive() const { return hasFish() && al::isAlive(fish); }
    bool isFishCapture() const { return isFishActive() ?: fish->mPlayerHack != nullptr; }
};
struct StageStateCreate : public exl::hook::impl::TrampolineHook<StageStateCreate> {
    static al::SceneObjHolder* Callback();
};

static StageState& stageState(al::IUseSceneObjHolder* sceneObjHolder) {
    return *(StageState*)al::getSceneObj(sceneObjHolder, 0x40);
}
