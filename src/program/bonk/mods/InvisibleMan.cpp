#include <al/Library/LiveActor/ActorModelFunction.h>
#include <bonk/mods/InvisibleMan.hpp>

namespace bm {
    struct AlwaysHidePlayer : Replace<AlwaysHidePlayer> {
        static void Callback(PlayerModelChangerHakoniwa* modelChanger) {
            modelChanger->isDirty = true;
            modelChanger->showingShadowMask = false;
            modelChanger->showingSilhouette = true;
            if (modelChanger->currentModel) {
                al::invalidateOcclusionQuery(modelChanger->currentModel);
                modelChanger->currentModel->kill();
            }
        }
    };
    struct HideMarioOnStart : Trampoline<HideMarioOnStart> {
        static void Callback(PlayerModelChangerHakoniwa* modelChanger, al::LiveActor* actor,
                             PlayerModelHolder* playerModelHolder, PlayerPainPartsKeeper* playerPainPartsKeeper,
                             PlayerCostumeInfo* playerCostumeInfo, IUseDimension* dimension) {
            Orig(modelChanger, actor, playerModelHolder, playerPainPartsKeeper, playerCostumeInfo, dimension);
            modelChanger->isDirty = true;
            modelChanger->showingShadowMask = false;
            modelChanger->showingModel = false;
            if (modelChanger->currentModel) {
                al::invalidateOcclusionQuery(modelChanger->currentModel);
                modelChanger->currentModel->kill();
            }
        }
    };

    void InvisibleMan::activate() {
        Mod::activate();

        updateAlways = true;
        patch::CodePatcher patcher(0x45dcd0); // disable model on load
        patcher.Seek(0x45e494);
        patcher.WriteInst(inst::Ret());
        AlwaysHidePlayer::InstallAtSymbol("_ZN26PlayerModelChangerHakoniwa9showModelEv");
        HideMarioOnStart::InstallAtSymbol("_ZN26PlayerModelChangerHakoniwaC1EPKN2al9LiveActorEP17PlayerModelHolderP21Pl"
                                          "ayerPainPartsKeeperP17PlayerCostumeInfoPK13IUseDimension");

        if (inScene()) {
            getMario()->mModelChanger->isDirty = true;
            getMario()->mModelChanger->showingModel = false;
            getMario()->mModelChanger->showingShadowMask = false;
            getMario()->mModelChanger->syncHost(true);
            al::invalidateOcclusionQuery(getMario()->mModelChanger->currentModel);
        }
    }
} // namespace bm
