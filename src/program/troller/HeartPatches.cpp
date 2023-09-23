#include <al/Library/LiveActor/LiveActor.h>
#include <game/GameData/GameDataFunction.h>
#include <game/GameData/GameDataHolderAccessor.h>
#include <lib.hpp>

struct LoveYouNot : exl::hook::impl::ReplaceHook<LoveYouNot> {
    static void Callback(al::LiveActor* actor) {
        actor->kill();

        GameDataFunction::killPlayer(GameDataHolderAccessor(actor));
    }
};

void heartPatches() {
    LoveYouNot::InstallAtSymbol("_ZN10LifeUpItem3getEv");
    LoveYouNot::InstallAtSymbol("_ZN12LifeUpItem2D3getEv");
    LoveYouNot::InstallAtOffset(0x1c7d34);
    LoveYouNot::InstallAtOffset(0x1c8f98);
}
