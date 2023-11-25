
#include <bonk/mods/NoOxygen.hpp>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/Player/PlayerFunction.h>

namespace bm {
    struct NoOxygenChecker : Trampoline<NoOxygenChecker> {
        static void Callback(PlayerActorHakoniwa* player) {
            Orig(player);

            if (player->mPlayerOxygen->isTriggerDamage() && !PlayerFunction::isPlayerDeadStatus(player)) {
                al::startHitReaction(player, "酸素不足ダメージ");
                player->mPlayerTrigger->set(PlayerTrigger::EActionTrigger::OxygenDamage);
                player->mPlayerDamageKeeper->damage(player->mPlayerConst->getDamageInvalidCount());
            }
        }
    };

    static bool invertOxygen(bool in) { return in; }
    void NoOxygen::activate() {
        Mod::activate();
        //        patch::CodePatcher patcher(0x485674); // return in PlayerStateSwim::isReduceOxygen
        //        patcher.BranchInst((void*)&invertOxygen);
        //        patcher.Seek(0x46a408); // return in PlayerStateDamageSwim::isReduceOxygen
        //        patcher.BranchInst((void*)&invertOxygen);

        patch::CodePatcher patcher(0);

        if (!par::get("OxygenDisableOnStart", false)) {
            patcher.Seek(0x41f634);         // isPlayerOxygen && checks in movement
            patcher.Write<u32>(0x37001340); // tbz w0, #0, #0x10
            patcher.Seek(0x41f640);
            patcher.Write<u32>(0x370012E0); // tbz w0, #0, #0x258
        }

        patcher.Seek(0x425dc4); // handle our own damage for oxygen (unlabeled function)
        patcher.Write(inst::Movz(X0, 0));
        NoOxygenChecker::InstallAtSymbol("_ZN19PlayerActorHakoniwa8movementEv");
    }
} // namespace bm
