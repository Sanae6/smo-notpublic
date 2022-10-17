#include "al/camera/CameraDirector.h"
#include "al/camera/ICameraInput.h"
#include "al/util.hpp"
#include "rs/util.hpp"
#include "al/util/VectorUtil.h"
#include "fps/MouseCameraInput.hpp"
#include "game/Player/PlayerActorHakoniwa.h"
#include "lib.hpp"
#include "nn/hid.h"

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;

namespace {
    NERVE_HEADER(PlayerActorHakoniwa, Demo);
}

static al::CameraTicket* subjectiveCameraTicket;
namespace al {
    al::CameraTicket* initSubjectiveCameraNoSave(al::IUseCamera const*, char const*);
    bool isActiveCamera(al::CameraTicket const*);
    void startCamera(al::IUseCamera const*, al::CameraTicket*, int);
}

namespace rs {
    bool isActiveDemo(al::LiveActor const*);
    bool isPlayerInWater(al::LiveActor const*);
}

bool isNerve(al::IUseNerve* nerveUser, const al::Nerve* nerve) {
    return al::isEqualString(typeid(*nerveUser->getNerveKeeper()->getCurrentNerve()).name(), typeid(*nerve).name());
}

HOOK_DEFINE_TRAMPOLINE(SetCameraInputMouse) {
    static void Callback(al::CameraDirector* cameraDirector, al::ICameraInput const* cameraInput) {
        cameraDirector->mCameraInputHolder->cameraInput = new MouseCameraInput(cameraInput);
    }
};

HOOK_DEFINE_TRAMPOLINE(InterpoleStopper) {
    static void Callback(void* a, void* b, float c, int _) {
        Orig(a, b, c, 0);
    }
};

HOOK_DEFINE_TRAMPOLINE(PlayerInit) {
    static void Callback(PlayerActorHakoniwa* player, al::ActorInitInfo& initInfo, const PlayerInitInfo& playerInitInfo) {
        Orig(player, initInfo, playerInitInfo);
        subjectiveCameraTicket = al::initSubjectiveCameraNoSave(player, "Subjective");
    }
};

HOOK_DEFINE_TRAMPOLINE(PlayerMovement) {
    static void Callback(PlayerActorHakoniwa* player) {
        Orig(player);
        if (rs::isActiveDemo(player))
            return;
        auto* puppet = player->getPlayerPuppet();
        puppet->hide();
        if (!al::isActiveCamera(subjectiveCameraTicket))
            al::startCamera(player, subjectiveCameraTicket, -1);
        sead::Vector3f facing, up;
        al::calcCameraLookDir(&facing, player, 0);
        rs::calcGroundNormalOrUpDir(&up, player, player->getPlayerCollision());
        if (rs::isPlayerInWater(player)) {
            up = -al::getGravity(player);
        }
        al::verticalizeVec(&facing, up, facing);
        sead::Quatf finalRot;
        al::makeQuatFrontUp(&finalRot, facing, up);
        puppet->setPose(finalRot);
    }
};

void fpsInit() {
    nn::hid::InitializeMouse();

    patch::CodePatcher patcher(0x97bd4c);
    patcher.Write(0x52B85688); // -30 -> -90 for min angle
    patcher.SeekRel(16);
    patcher.Write(0x52A85688); // 75 -> 90 for max angle
    patcher.SeekRel(4);
    patcher.Write(0x52A86188); // 180 -> 110 for height
    patcher.Seek(0x97c0a4); // disable * 50f for CameraPoserSubjective offset
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());

    SetCameraInputMouse::InstallAtSymbol("_ZN2al14CameraDirector14setCameraInputEPKNS_12ICameraInputE");
    InterpoleStopper::InstallAtSymbol("_ZN2al15CameraInterpole5startEPKNS_12CameraTicketEfi");
    PlayerInit::InstallAtSymbol("_ZN19PlayerActorHakoniwa10initPlayerERKN2al13ActorInitInfoERK14PlayerInitInfo");
    PlayerMovement::InstallAtSymbol("_ZN19PlayerActorHakoniwa8movementEv");
}