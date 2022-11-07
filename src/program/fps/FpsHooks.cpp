#include "al/camera/CameraDirector.h"
#include "al/camera/ICameraInput.h"
#include "al/nerve/ActorStateBase.h"
#include "al/sequence/Sequence.h"
#include "al/util.hpp"
#include "al/util/VectorUtil.h"
#include "fps/MouseCameraInput.hpp"
#include "game/Player/PlayerActorHakoniwa.h"
#include "lib.hpp"
#include "logger/Logger.hpp"
#include "nn/hid.h"
#include "rs/util.hpp"

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

struct ButtonHook {
    using Func = bool(*)();
    ButtonHook(Func func) {
        mFunc = func;
    }
    Func mFunc;
    uint state = 0;
    bool trigger = false;
    bool release = false;
    inline bool isDown() const {
        return state > 0;
    }
    void update() {
        if (mFunc()) {
            release = false;
            trigger = state == 0;
            state++;
//            Logger::log("a hook has been activeded %s %s\n", BTOC(trigger), BTOC(isDown()));
        } else {
            trigger = false;
            release = state > 0;
            state = 0;
        }
    }
};

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

static sead::Vector2f keyboard;
nn::hid::KeyboardState keyboardState = {};
nn::hid::MouseState mouseState = {};
ButtonHook jumpHook = ButtonHook([]() {
    return keyboardState.isKeyDown(nn::hid::KeyboardKey::Space);
});
ButtonHook capHook = ButtonHook([]() {
    return mouseState.buttons.isBitSet(nn::hid::MouseButton::Left);
});
ButtonHook subActionHook = ButtonHook([]() {
    return keyboardState.isKeyDown(nn::hid::KeyboardKey::LeftShift);
});
ButtonHook upUiHook = ButtonHook([]() {
    return keyboardState.isKeyDown(nn::hid::KeyboardKey::W);
});
ButtonHook leftUiHook = ButtonHook([]() {
    return keyboardState.isKeyDown(nn::hid::KeyboardKey::A);
});
ButtonHook downUiHook = ButtonHook([]() {
    return keyboardState.isKeyDown(nn::hid::KeyboardKey::S);
});
ButtonHook rightUiHook = ButtonHook([]() {
    return keyboardState.isKeyDown(nn::hid::KeyboardKey::D);
});
ButtonHook aBtnHook = ButtonHook([]() {
    return mouseState.buttons.isBitSet(nn::hid::MouseButton::Left) || keyboardState.isKeyDown(nn::hid::KeyboardKey::Space);
});
ButtonHook bBtnHook = ButtonHook([]() {
    return mouseState.buttons.isBitSet(nn::hid::MouseButton::Right);
});
ButtonHook pauseHook = ButtonHook([]() {
    return keyboardState.isKeyDown(nn::hid::KeyboardKey::Escape);
});
ButtonHook mapHook = ButtonHook([]() {
    return keyboardState.isKeyDown(nn::hid::KeyboardKey::T);
});

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
        rs::calcGroundNormalOrUpDir(&up, player, player->getPlayerCollision());
        if (rs::isPlayerInWater(player)) {
            up = -al::getGravity(player);
        }

        al::calcCameraLookDir(&facing, player, 0);
        al::verticalizeVec(&facing, up, facing);

        sead::Quatf finalRot;
        al::makeQuatFrontUp(&finalRot, facing, up);
        puppet->setPose(finalRot);
    }
};

HOOK_DEFINE_TRAMPOLINE(PlayerInputGoodinator) {
    static sead::Vector2f& Callback() {
        keyboard = sead::Vector2f::zero;
        if (keyboardState.isKeyDown(nn::hid::KeyboardKey::W)) keyboard.y += 1.0f;
        if (keyboardState.isKeyDown(nn::hid::KeyboardKey::A)) keyboard.x -= 1.0f;
        if (keyboardState.isKeyDown(nn::hid::KeyboardKey::S)) keyboard.y -= 1.0f;
        if (keyboardState.isKeyDown(nn::hid::KeyboardKey::D)) keyboard.x += 1.0f;
        return keyboard;
    }
};

HOOK_DEFINE_TRAMPOLINE(IsTriggerJump) { static bool Callback() { return jumpHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsHoldJump) { static bool Callback() { return jumpHook.isDown(); } };
HOOK_DEFINE_TRAMPOLINE(IsReleaseJump) { static bool Callback() { return jumpHook.release; } };

HOOK_DEFINE_TRAMPOLINE(IsTriggerAction) { static bool Callback() { return capHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsHoldAction) { static bool Callback() { return capHook.isDown(); } };
HOOK_DEFINE_TRAMPOLINE(IsReleaseAction) { static bool Callback() { return capHook.release; } };

HOOK_DEFINE_TRAMPOLINE(IsTriggerSubAction) { static bool Callback() { return subActionHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsHoldSubAction) { static bool Callback() { return subActionHook.isDown(); } };
HOOK_DEFINE_TRAMPOLINE(IsReleaseSubAction) { static bool Callback() { return subActionHook.release; } };

HOOK_DEFINE_TRAMPOLINE(IsTriggerUiUp) { static bool Callback() { return upUiHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsHoldUiUp) { static bool Callback() { return upUiHook.isDown(); } };

HOOK_DEFINE_TRAMPOLINE(IsTriggerUiLeft) { static bool Callback() { return leftUiHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsHoldUiLeft) { static bool Callback() { return leftUiHook.isDown(); } };

HOOK_DEFINE_TRAMPOLINE(IsTriggerUiDown) { static bool Callback() { return downUiHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsHoldUiDown) { static bool Callback() { return downUiHook.isDown(); } };

HOOK_DEFINE_TRAMPOLINE(IsTriggerUiRight) { static bool Callback() { return rightUiHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsHoldUiRight) { static bool Callback() { return rightUiHook.isDown(); } };

HOOK_DEFINE_TRAMPOLINE(IsPadTriggerA) { static bool Callback() { return aBtnHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsPadHoldA) { static bool Callback() { return aBtnHook.isDown(); } };
HOOK_DEFINE_TRAMPOLINE(IsPadReleaseA) { static bool Callback() { return aBtnHook.release; } };

HOOK_DEFINE_TRAMPOLINE(IsPadTriggerB) { static bool Callback() { return bBtnHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsPadHoldB) { static bool Callback() { return bBtnHook.isDown(); } };
HOOK_DEFINE_TRAMPOLINE(IsPadReleaseB) { static bool Callback() { return bBtnHook.release; } };

HOOK_DEFINE_TRAMPOLINE(IsPadTriggerPlus) { static bool Callback() { return pauseHook.trigger; } };
HOOK_DEFINE_TRAMPOLINE(IsPadTriggerMinus) { static bool Callback() { return mapHook.trigger; } };

HOOK_DEFINE_TRAMPOLINE(SequenceUpdate) { static void Callback(al::Sequence* sequence) {
    Orig(sequence);

    nn::hid::GetKeyboardState(&keyboardState);
    nn::hid::GetMouseState(&mouseState);

    jumpHook.update();
    capHook.update();
    subActionHook.update();
    upUiHook.update();
    leftUiHook.update();
    downUiHook.update();
    rightUiHook.update();
    aBtnHook.update();
    bBtnHook.update();
    pauseHook.update();
    mapHook.update();

    if ((keyboardState.modifiers.isBitSet(nn::hid::KeyboardModifier::LeftAlt) || keyboardState.modifiers.isBitSet(nn::hid::KeyboardModifier::RightAlt)) && keyboardState.isKeyDown(nn::hid::KeyboardKey::F4)) {
        svcExitProcess();
    }
} };
HOOK_DEFINE_TRAMPOLINE(Gun) { static bool Callback() { return mouseState.buttons.isBitSet(nn::hid::MouseButton::Right); } };
namespace rs {
    bool isMsgTankBullet(al::SensorMsg const*);
}
HOOK_DEFINE_TRAMPOLINE(KleptoGunPatch) { static bool Callback(al::ActorStateBase* jango, al::SensorMsg const* msg, al::HitSensor* source, al::HitSensor* target) {
    if (rs::isMsgTankBullet(msg)) {
        al::invalidateClipping(jango->getHost());
        jango->kill();

        return true;
    }
    return Orig(jango, msg, source, target);
}};

void fpsInit() {
    nn::hid::InitializeMouse();
    nn::hid::InitializeKeyboard();

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
    patcher.Seek(0x44e1a4);
    patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    patcher.WriteInst(inst::Ret());
    patcher.Seek("_ZN20PlayerActionFunction13isOppositeDirERKN4sead7Vector3IfEES4_", 0x0);
    patcher.WriteInst(inst::MovRegister(reg::X0, reg::None64));
    patcher.WriteInst(inst::Ret());
    patcher.Seek("_ZN23PlayerActionTurnControl6updateERKN4sead7Vector3IfEES4_", 0x5f4);
    patcher.BranchInst(0x41a23c);
    patcher.Seek("_ZN23PlayerActionTurnControl6updateERKN4sead7Vector3IfEES4_", 0x688);
    patcher.WriteInst(inst::Nop());
    patcher.Seek("_ZN23PlayerActionTurnControl6updateERKN4sead7Vector3IfEES4_", 0x694);
    patcher.WriteInst(inst::Nop());
    patcher.Seek("_ZN23PlayerActionTurnControl6updateERKN4sead7Vector3IfEES4_", 0x87C);
    patcher.Write(0xBD4DFD00); // ldr s0, [x8, #0xdfc] (loads 180f)
    patcher.Seek("_ZN29PlayerActionGroundMoveControl17updatePoseUpFrontERKN4sead7Vector3IfEES4_f", 0x0);
    patcher.Write(inst::Ret());

    SetCameraInputMouse::InstallAtSymbol("_ZN2al14CameraDirector14setCameraInputEPKNS_12ICameraInputE");
    InterpoleStopper::InstallAtSymbol("_ZN2al15CameraInterpole5startEPKNS_12CameraTicketEfi");
    PlayerInit::InstallAtSymbol("_ZN19PlayerActorHakoniwa10initPlayerERKN2al13ActorInitInfoERK14PlayerInitInfo");
    PlayerMovement::InstallAtSymbol("_ZN19PlayerActorHakoniwa8movementEv");
    PlayerInputGoodinator::InstallAtSymbol("_ZN19PlayerInputFunction17getMoveInputStickEPKN2al9LiveActorEii");

    IsTriggerJump::InstallAtSymbol("_ZN19PlayerInputFunction13isTriggerJumpEPKN2al9LiveActorEi");
    IsHoldJump::InstallAtSymbol("_ZN19PlayerInputFunction10isHoldJumpEPKN2al9LiveActorEi");
    IsReleaseJump::InstallAtSymbol("_ZN19PlayerInputFunction13isReleaseJumpEPKN2al9LiveActorEi");

    IsTriggerAction::InstallAtSymbol("_ZN19PlayerInputFunction15isTriggerActionEPKN2al9LiveActorEi");
    IsHoldAction::InstallAtSymbol("_ZN19PlayerInputFunction12isHoldActionEPKN2al9LiveActorEi");
    IsReleaseAction::InstallAtSymbol("_ZN19PlayerInputFunction15isReleaseActionEPKN2al9LiveActorEi");

    IsTriggerSubAction::InstallAtSymbol("_ZN19PlayerInputFunction18isTriggerSubActionEPKN2al9LiveActorEi");
    IsHoldSubAction::InstallAtSymbol("_ZN19PlayerInputFunction15isHoldSubActionEPKN2al9LiveActorEi");
    IsReleaseSubAction::InstallAtSymbol("_ZN19PlayerInputFunction18isReleaseSubActionEPKN2al9LiveActorEi");

    IsTriggerUiUp::InstallAtSymbol("_ZN2rs13isTriggerUiUpEPKN2al18IUseSceneObjHolderE");
    IsHoldUiUp::InstallAtSymbol("_ZN2rs10isHoldUiUpEPKN2al18IUseSceneObjHolderE");

    IsTriggerUiLeft::InstallAtSymbol("_ZN2rs15isTriggerUiLeftEPKN2al18IUseSceneObjHolderE");
    IsHoldUiLeft::InstallAtSymbol("_ZN2rs12isHoldUiLeftEPKN2al18IUseSceneObjHolderE");

    IsTriggerUiDown::InstallAtSymbol("_ZN2rs15isTriggerUiDownEPKN2al18IUseSceneObjHolderE");
    IsHoldUiDown::InstallAtSymbol("_ZN2rs12isHoldUiDownEPKN2al18IUseSceneObjHolderE");

    IsTriggerUiRight::InstallAtSymbol("_ZN2rs16isTriggerUiRightEPKN2al18IUseSceneObjHolderE");
    IsHoldUiRight::InstallAtSymbol("_ZN2rs13isHoldUiRightEPKN2al18IUseSceneObjHolderE");

    IsPadTriggerA::InstallAtSymbol("_ZN2al13isPadTriggerAEi");
    IsPadHoldA::InstallAtSymbol("_ZN2al10isPadHoldAEi");
    IsPadReleaseA::InstallAtSymbol("_ZN2al13isPadReleaseAEi");

    IsPadTriggerB::InstallAtSymbol("_ZN2al13isPadTriggerBEi");
    IsPadHoldB::InstallAtSymbol("_ZN2al10isPadHoldBEi");
    IsPadReleaseB::InstallAtSymbol("_ZN2al13isPadReleaseBEi");

    IsPadTriggerPlus::InstallAtSymbol("_ZN2al16isPadTriggerPlusEi");

    IsPadTriggerMinus::InstallAtSymbol("_ZN2al17isPadTriggerMinusEi");

    SequenceUpdate::InstallAtSymbol("_ZN16HakoniwaSequence6updateEv");
    Gun::InstallAtSymbol("_ZN2al11isPadHoldZREi");
    KleptoGunPatch::InstallAtSymbol("_ZN18JangoStateCapCatch10receiveMsgEPKN2al9SensorMsgEPNS0_9HitSensorES5_");



    /**
     * movement right walks up right diagonally
     * movement back walks
     * camera not attached to mario's head joint /j
     * ui input hooks required
     * captures not hidden
     * captures don't face player
     * do shake inputs depending on vertical look direction (up-throw when looking up, down-throw when looking down)
     * do shake inputs on cap button
     * disable all gyro camera related stuff
     */

    Logger::log("Done setting up jump hooks!\n");
}
