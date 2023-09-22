#include "al/Library/Nerve/NerveStateBase.h"
#include "al/Library/Nerve/NerveUtil.h"
#include "al/Project/HitSensor/HitSensor.h"
#include "koopa/Koopa.hpp"
#include "logger/Logger.hpp"
#include "rs/util/SensorUtil.h"
#include "util/sys/mem_layout.hpp"
#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <lib.hpp>

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;

struct BreedaWanwan {
    bool tryBlowCap(al::HitSensor*);
};

struct WanwanBackHook : exl::hook::impl::TrampolineHook<WanwanBackHook> {
    static bool Callback(al::LiveActor* actor, al::SensorMsg* msg, al::HitSensor* source, al::HitSensor* target) {
        if (rs::isMsgTRexAttack(msg)) {
            al::setNerve(actor, (al::Nerve*)(exl::util::GetMainModuleInfo().m_Data.m_Start + 0x1c621e8 - 0x1c4e000));
            auto* wan = (BreedaWanwan*)actor;
            for(int i = 0; i < 10; i++) {
                wan->tryBlowCap(source);
            }
            return true;
        }

        return Orig(actor, msg, source, target);
    }
};

static bool koopaFinished = false;

void handleKoopaOnFinish() {
    Logger::log("Koopa finished!\n");
    koopaFinished = true;
}

void handleKoopaOffFinish() {
    Logger::log("Koopa off finished...\n");
    koopaFinished = false;
}

bool punchL(al::HitSensor* sensor) { return al::isSensorName(sensor, "PunchL"); }
bool punchR(al::HitSensor* sensor) { return al::isSensorName(sensor, "PunchR"); }

bool lastPunchRight = false;
static const int finishCooldown = 360;
static const int punchCooldown = 10;

bool koopaPunch(al::HitSensor* target, al::HitSensor* source, int& recTimer) {
    // Logger::log("Punch source: %s ", target->name);
    if (!punchL(target) && !punchR(target)) return false;
    // Logger::log("Trying L=%s R=%s\n", BTOC(punchL(target)), BTOC(punchR(target)));
    if (lastPunchRight == punchR(target)) return false;
    Logger::log("%s punch checked\n", punchR(target) ? "Right" : "Left");
    if (punchR(target)) {
        if (koopaFinished) {
            if (rs::sendMsgKoopaCapPunchFinishR(source, target) ||
                rs::sendMsgKoopaCapPunchInvincibleR(source, target) ||
                rs::sendMsgKoopaCapPunchR(source, target)) {
                koopaFinished = false;
                lastPunchRight = true;
                recTimer = finishCooldown;
                return true;
            }
            Logger::log("Actually finished R\n");
            return false;
        } else if (rs::sendMsgKoopaCapPunchKnockBackR(source, target) ||
                   rs::sendMsgKoopaCapPunchInvincibleR(source, target) ||
                   rs::sendMsgKoopaCapPunchR(source, target)) {
            lastPunchRight = true;
            recTimer = punchCooldown;
            Logger::log("Koopa cap punch R\n");
            return true;
        }
    } else {
        if (koopaFinished) {
            if (rs::sendMsgKoopaCapPunchFinishL(source, target) ||
                rs::sendMsgKoopaCapPunchInvincibleL(source, target) ||
                rs::sendMsgKoopaCapPunchL(source, target)) {
                koopaFinished = false;
                lastPunchRight = false;
                recTimer = finishCooldown;
                return true;
            }
            Logger::log("Actually finished L\n");
            return false;
        } else if (rs::sendMsgKoopaCapPunchKnockBackL(source, target) ||
                   rs::sendMsgKoopaCapPunchInvincibleL(source, target) ||
                   rs::sendMsgKoopaCapPunchL(source, target)) {
            lastPunchRight = false;
            recTimer = punchCooldown;
            Logger::log("Koopa cap punch L\n");
            return true;
        }
    }

    return false;
}

void koopaHatHide(al::LiveActor* actor) {
    actor->kill();
}

void damageOnTired(al::LiveActor* actor) {
    al::setNerve(actor, (const al::Nerve*)(exl::util::GetMainModuleInfo().m_Rodata.m_Start + 0x106b340));
}

void jangoEndDemo(al::LiveActor* actor) {
    actor->kill();
}

bool electricWireTransfer(al::HitSensor* sensor) {
    return al::isSensorEnemyBody(sensor) || typeid(*sensor->host).hash_code() == typeid(Koopa).hash_code();
}

void enemyHooks(patch::CodePatcher& patcher) {
    WanwanBackHook::InstallAtSymbol("_ZN12BreedaWanwan10receiveMsgEPKN2al9SensorMsgEPNS0_9HitSensorES5_");
    patcher.Seek(0x87c84); // Koopa::onFinish
    patcher.BranchInst((void*)handleKoopaOnFinish);
    patcher.Seek(0x87c8c); // Koopa::offFinish
    patcher.BranchInst((void*)handleKoopaOffFinish);
    patcher.Seek(0x84c88); // Koopa::offFinish
    patcher.BranchInst((void*)koopaHatHide);
    patcher.Seek(0x85168); // KoopaCap::isEquip
    patcher.WriteInst(inst::Movz(reg::X0, 1));
    patcher.WriteInst(inst::Ret());
    patcher.Seek(0x852c8); // KoopaCap::isAttach
    patcher.WriteInst(inst::Movz(reg::X0, 1));
    patcher.WriteInst(inst::Ret());
    patcher.Seek(0xa4ae8); // KoopaStateThrowCap::exeCatchCapStart
    patcher.WriteInst(inst::Movz(reg::X0, 0));
    patcher.Seek(0xa4af4); // KoopaStateThrowCap::exeCatchCapStart
    patcher.WriteInst(inst::Movz(reg::X0, 0));
    patcher.Seek(0x12e73c);
    patcher.BranchInst((void*)jangoEndDemo);
    patcher.Seek(0x269084); // change transfer message
    patcher.BranchLinkInst((void*)electricWireTransfer);
    patcher.Seek(0x3d758);
    patcher.BranchInst((void*)damageOnTired);
}