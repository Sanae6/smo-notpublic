#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <lib.hpp>
#include <patch/code_patcher.hpp>

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;
bool checkHariet(al::HitSensor* sensor) { return al::isSensorPlayerAll(sensor); }
void killActor(al::LiveActor* actor) { actor->kill(); }

bool bowserBossFinished = false;

struct OnFinish : public exl::hook::impl::TrampolineHook<OnFinish> {
    static void Callback(al::LiveActor* actor) {
        Orig(actor);
        bowserBossFinished = true;
    }
};

struct OffFinish : public exl::hook::impl::TrampolineHook<OffFinish> {
    static void Callback(al::LiveActor* actor) {
        Orig(actor);
        bowserBossFinished = false;
    }
};

extern "C" void _ZN18BombTailBombHolder4killEv(void*);
struct ForceDisappearKill : public exl::hook::impl::ReplaceHook<ForceDisappearKill> {
    static void Callback(void* holder) {
        _ZN18BombTailBombHolder4killEv(holder);
    }
};

void koopaSideInit(patch::CodePatcher& patcher) {
    // broodal patches
    patcher.Seek(0xbca30); // topper stomp related
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Movz(reg::X0, 1));
    patcher.Seek(0xc2a9c); // topper
    patcher.WriteInst(inst::Nop());
    patcher.Seek(0xc2acc); // topper
    patcher.WriteInst(inst::Nop());
    patcher.Seek(0xb364); // hariet
    patcher.WriteInst(inst::Nop());
    patcher.Seek(0xb1b4); // hariet
    patcher.BranchLinkInst((void*)&checkHariet);
    ForceDisappearKill::InstallAtSymbol("_ZN18BombTailBombHolder14forceDisappearEv"); // hariet kill bombs
    patcher.Seek(0x59340); // spewert
    patcher.WriteInst(inst::Nop());
    patcher.Seek(0x519e0); // rango
    patcher.WriteInst(inst::Nop());
    patcher.Seek(0x51a10); // rango
    patcher.WriteInst(inst::Nop());
    // torkdrift patches
    patcher.Seek(0x21fb4);
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());
    patcher.Seek(0x22038);
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());
    patcher.WriteInst(inst::Nop());
    patcher.Seek(0x1c0ec);
    patcher.WriteInst(inst::Movz(reg::X0, 1));
    patcher.WriteInst(inst::Ret());

    // bowser patches
    OnFinish::InstallAtSymbol("_ZN14KoopaCapPlayer8onFinishEv");
    OffFinish::InstallAtSymbol("_ZN14KoopaCapPlayer9offFinishEv");
    patcher.Seek(0x84c88); // hide koopa cap
    patcher.BranchInst((void*)&killActor);
    patcher.Seek(0x85168); // isEquip
    patcher.WriteInst(inst::Movz(reg::X0, 1));
    patcher.WriteInst(inst::Ret());
    patcher.Seek(0x852c8); // isAttach
    patcher.WriteInst(inst::Movz(reg::X0, 1));
    patcher.WriteInst(inst::Ret());
    patcher.Seek(0xa4ae8); // exeCatchCapStart
    patcher.WriteInst(inst::Movz(reg::X0, 0));
    patcher.Seek(0xa4af4); // exeCatchCapStart
    patcher.WriteInst(inst::Movz(reg::X0, 0));

    patcher.Seek(0x26e3d8);
    patcher.WriteInst(inst::Movz(reg::X0, 0));
}
