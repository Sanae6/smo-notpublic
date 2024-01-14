#include <al/Library/Controller/JoyPadUtil.h>
#include <al/Library/Demo/DemoFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/Math/MathUtil.h>
#include <al/Library/Math/MathVectorUtil.h>
#include <game/Player/IUsePlayerPuppet.h>
#include <logger/Params.h>
#include <rs/util.hpp>
#include <spook/FirstPersonState.hpp>
#include <spook/SpookState.hpp>

namespace sp {
    sp::FirstPersonState::FirstPersonState(SpookState* state, const al::ActorInitInfo& initInfo) {
        spookState = state;
        subjectiveCameraTicket = al::initSubjectiveCameraNoSave(state->player, "Subjective");
    }
    void FirstPersonState::playerMovement() const {
        auto* player = spookState->player;
        if (alDemoUtilTmp::isActiveDemo(player))
            return;

        auto* puppet = player->getPlayerPuppet();
        puppet->hide();

        if (!al::isActiveCamera(subjectiveCameraTicket))
            al::startCamera(player, subjectiveCameraTicket, -1);
        if (al::isActiveCamera(subjectiveCameraTicket) && par::get("EnableFirstPerson", true))

                    al::endCamera(player, subjectiveCameraTicket, -1, false);

        auto s = al::getLeftStick(al::getMainControllerPort());
        if (!par::get("DisableFrontFacing", false) && al::isNearZero(s, 0.001) && rs::isPlayerOnGround(player)) {
            sead::Vector3f facing, up;
            rs::calcGroundNormalOrUpDir(&up, player, player->getPlayerCollision());

            if (rs::isPlayerInWater(player))
                up = -al::getGravity(player);

            al::calcCameraLookDir(&facing, player, 0);
            al::verticalizeVec(&facing, up, facing);

            sead::Quatf finalRot;
            al::makeQuatFrontUp(&finalRot, facing, up);
            puppet->setPose(finalRot);
        }
    }
} // namespace sm
