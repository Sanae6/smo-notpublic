
#include <bonk/mods/NoOxygen.hpp>
#include <game/Player/PlayerActorHakoniwa.h>
#include <game/Player/PlayerFunction.h>

namespace bm {
    static bool enabled = false;
    struct NoOxygenChecker : Trampoline<NoOxygenChecker> {
        static void Callback(PlayerActorHakoniwa* player) {
            Orig(player);
            if (!enabled) return;

            if (player->mPlayerOxygen->isTriggerDamage() && !PlayerFunction::isPlayerDeadStatus(player)) {
                al::startHitReaction(player, "酸素不足ダメージ");
                player->mPlayerTrigger->set(PlayerTrigger::EActionTrigger::OxygenDamage);
                player->mPlayerDamageKeeper->damage(player->mPlayerConst->getDamageInvalidCount());
            }
        }
    };

    void NoOxygen::activate() {
        Mod::activate();
        enabled = true;

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
    void NoOxygen::deactivate() {
        Mod::deactivate();
        enabled = false;

        patch::CodePatcher patcher(0);

        patcher.Seek(0x41f634);         // isPlayerOxygen && checks in movement
        patcher.Write<u32>(0x37000080); // tbz w0, #0, #0x10
        patcher.Seek(0x41f640);
        patcher.Write<u32>(0x360012E0); // tbz w0, #0, #0x258

        patcher.Seek(0x425dc4); // handle our own damage for oxygen (unlabeled function)
        patcher.Write<u32>(0x9400E3D0);
    }
} // namespace bm
