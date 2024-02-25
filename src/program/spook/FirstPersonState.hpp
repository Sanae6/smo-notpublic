#pragma once

#include <al/Library/Camera/CameraTicket.h>
#include <al/Library/LiveActor/ActorInitInfo.h>
namespace sp {
    struct FirstPersonState {
        class SpookState* spookState;
        al::CameraTicket* subjectiveCameraTicket;
        FirstPersonState(SpookState* state, const al::ActorInitInfo& initInfo);

        void playerMovement() const;
        void disable();
    };
} // namespace sm
