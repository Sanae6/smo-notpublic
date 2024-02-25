#pragma once

#include "al/Library/Bgm/BgmDirector.h"
#include "al/Library/Se/SeDirector.h"
#include "spook/SePlayObjSpooky.hpp"
#include <al/Library/Scene/ISceneObj.h>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/StageScene/StageScene.h>
#include <spook/FirstPersonState.hpp>
#include <utils/ForwardDecls.hpp>

namespace sp {
    struct SpookState : al::ISceneObj {
        PlayerActorHakoniwa* player = nullptr;
        StageScene* scene = nullptr;
        al::PostProcessingFilter* postProcessingFilter;
        al::LppSpot* flashlight = nullptr;
        al::LiveActor* follower = nullptr;
        FirstPersonState* firstPerson;
        sead::Matrix34f noRotateMtx;
        al::AudioDirector* audioDirector;
        SePlayObjSpooky* spookyPlay;
        bool isSpookActive = false;
        void initAfterPlacementSceneObj(const al::ActorInitInfo&) override;
        void update();
        void startSpook();
        void stopSpook();

        void updateAreaSpooky();
        void toggleArea();
    };

    [[maybe_unused]] static SpookState& spookyState(al::IUseSceneObjHolder* sceneObjHolder) {
        return *(SpookState*)al::getSceneObj(sceneObjHolder, 0x40);
    }

    void spookyInit();
} // namespace sp
