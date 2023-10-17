
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
    void NoOxygen::activate() {
        Mod::activate();
        patch::CodePatcher patcher(0x41f630); // isReduceOxygen = true in Player::movement
        patcher.WriteInst(inst::Movz(X0, 1));
        patcher.Seek(0x425dc4); // handle our own damage for oxygen (unlabeled function)
        patcher.Write(inst::Movz(X0, 0));
        NoOxygenChecker::InstallAtSymbol("_ZN19PlayerActorHakoniwa8movementEv");
    }
} // namespace bm
