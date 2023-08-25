#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <lib.hpp>
#include <patch/code_patcher.hpp>
#include <rs/util/SensorUtil.h>

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;

struct WanwanBackHook : exl::hook::impl::TrampolineHook<WanwanBackHook> {
    static bool Callback(al::LiveActor* actor, al::SensorMsg* msg, al::HitSensor* source, al::HitSensor* target) {
        if (rs::isMsgTRexAttack(msg)) {
            al::setNerve(actor, (al::Nerve*)(exl::util::GetMainModuleInfo().m_Data.m_Start + 0x1c621e8 - 0x1c4e000));
            return true;
        }

        return Orig(actor, msg, source, target);
    }
};

void enemyHooks(patch::CodePatcher& patcher) {
    WanwanBackHook::InstallAtSymbol("_ZN12BreedaWanwan10receiveMsgEPKN2al9SensorMsgEPNS0_9HitSensorES5_");
}