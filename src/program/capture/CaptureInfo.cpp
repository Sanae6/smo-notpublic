#include "CaptureState.hpp"
#include "al/Library/LiveActor/ActorFlagFunction.h"
#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorSensorFunction.h"
#include "al/Library/Rail/Rail.h"
#include "al/Library/Rail/RailKeeper.h"
#include "al/Library/Rail/RailPart.h"
#include "al/Library/Rail/RailRider.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/Player/States/PlayerStateHack.h"
#include "logger/Logger.hpp"
#include "rs/util/SensorUtil.h"
#include "utils/Helpers.h"
#include <al/Library/LiveActor/ActorActionFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/Math/MathUtil.h>
#include <al/Library/Poser/ActorCameraTarget.h>
#include <capture/CaptureInfo.hpp>
#include <utils/ForwardDecls.hpp>

namespace cs {
  std::span<const CaptureInfo> CaptureInfo::getCaptures() {
    static al::ActorCameraTarget* target = nullptr;
    static sead::Vector3f storedPosition = sead::Vector3f::zero;
    static const CaptureInfo captures[]{
        CaptureInfo{"Frog", nullptr},
        CaptureInfo{"KuriboWing",
                    []() {
                      patch::CodePatcher patcher(0x15b12c);
                      patcher.WriteInst(inst::Movz(reg::X0, 1));
                      Logger::log("Setup KuriboWing\n");
                    },
                    [](al::LiveActor* actor, const sead::Vector3f& position) {
                      actor->mRailKeeper = alloc<al::RailKeeper>();
                      auto* rail = alloc<al::Rail>();

                      rail->mRailPart = allocArray<al::RailPart>(1);
                      auto newPosition = position + sead::Vector3f::ey * 500;
                      rail->mRailPart[0].init(position, position, newPosition, newPosition);
                      rail->mRailPartCount = 1;
                      rail->mRailPointsCount = 2;
                      rail->mRailPoints = allocArray<al::PlacementInfo*>(2);
                      std::fill(rail->mRailPoints, rail->mRailPoints + 2, nullptr);
                      rail->isClosed = false;
                      actor->mRailKeeper->mRailRider = alloc<al::RailRider>(rail);
                      actor->getRailRider()->setSpeed(1);
                      Logger::log("Set up rail %p %p %p\n", actor->mRailKeeper, actor->getRailRider(), rail);
                    }},
        CaptureInfo{"TRex", nullptr},
        CaptureInfo{"Megane", nullptr},
        CaptureInfo{"KuriboPossessed", nullptr},
        CaptureInfo{"Kakku", nullptr},
        CaptureInfo{"JugemFishing", nullptr},
        CaptureInfo{"Pukupuku", nullptr, nullptr, nullptr, sead::Vector3f::ey * 110.f},
        CaptureInfo{"Senobi", nullptr},
        CaptureInfo{"FireBrosPossessed",
                    []() {
                      struct FixBros : Replace<FixBros> {
                        static void Callback(void* keyPoseUpdater, sead::Vector3f* output) { *output = storedPosition; }
                      };

                      struct FixBrosGround : Trampoline<FixBrosGround> {
                        static void Callback(al::LiveActor* actor) {
                          Orig(actor);
                          if (al::isOnGround(actor, 0)) {
                            storedPosition = al::getTrans(actor);
                          }
                        }
                      };

                      FixBros::InstallAtSymbol("_ZNK28BrosEnemyMovePositionUpdater12getNextTransEPN4sead7Vector3IfEE");
                      //                      FixBrosGround::InstallAtSymbol("_ZN22BrosStateEnemyMovement14exeFallJumpEndEv");
                      FixBrosGround::InstallAtOffset(0xdbd04);

                      Logger::log("Prepared fire bros!\n");
                    },
                    [](auto actor, auto p) {
                      storedPosition = p;
                      unsafeRef<bool>(actor, 0x168) = true;
                    }},
        CaptureInfo{"Tank", nullptr},
        CaptureInfo{"Gamane", nullptr},
        CaptureInfo{"Imomu",
                    []() {
                      struct IommuControl : Trampoline<IommuControl> {
                        static void Callback(al::LiveActor* actor) {
                          Orig(actor);

                          if (actor != CaptureState::instance()->getCapture())
                            return;

                          auto& matrix = unsafeRef<sead::Matrix34f>(actor, 0x18c);
                          matrix.makeST(sead::Vector3f::ones, al::getTrans(actor));
                        }
                      };
                      IommuControl::InstallAtSymbol("_ZN5Imomu7controlEv");
                    }},
        CaptureInfo{"RadiconNpc",
                    []() {
                      patch::CodePatcher patcher(0x3bc704);
                      patcher.BranchLinkInst(0x83e204);

                      struct HandleStartCar : Trampoline<HandleStartCar> {
                        static void Callback(al::LiveActor* npc) {
                          Orig(npc);
                          if (al::isActionEnd(npc))
                            al::setCameraTarget(npc, target);
                        }
                      };

                      struct HandleEndCar : Trampoline<HandleStartCar> {
                        static void Callback(al::LiveActor* npc) {
                          Orig(npc);
                          if (al::isActionEnd(npc))
                            al::resetCameraTarget(npc, target);
                        }
                      };
                      HandleStartCar::InstallAtSymbol("_ZN10RadiconNpc12exeStartHackEv");
                      HandleEndCar::InstallAtSymbol("_ZN10RadiconNpc10exeEndHackEv");
                    },
                    nullptr,
                    [](al::LiveActor* actor) {
                      target = alloc<al::ActorCameraTarget>(unsafeRef<al::LiveActor*>(actor, 0x130), 0, nullptr);
                    }},
        CaptureInfo{"Byugo", nullptr, nullptr, nullptr, sead::Vector3f::ey * 500.f},
        CaptureInfo{"Yukimaru",
                    []() {
                      patch::CodePatcher patch(0x3f1538);
                      patch.WriteInst(inst::Ret());
                    }},
        CaptureInfo{"Hosui", []() { /*patch::CodePatcher(0x11b1fc).WriteInst(inst::Movz(reg::X0, 1));*/ }},
        CaptureInfo{"HammerBrosPossessed", nullptr,
                    [](auto actor, auto p) {
                      storedPosition = p;
                      unsafeRef<bool>(actor, 0x168) = true;
                    }},
        CaptureInfo{"Tsukkun", nullptr},
        CaptureInfo{"Statue", nullptr},
        CaptureInfo{"KaronWing", nullptr},
        CaptureInfo{"Bull", nullptr},
        CaptureInfo{"Koopa",
                    []() {
                      patch::CodePatcher p(0x7de80);
                      p.WriteInst(inst::Nop());
                      p.WriteInst(inst::Nop());

                      struct CheckForCapture : Trampoline<CheckForCapture> {
                        static void Callback(al::LiveActor* actor) {
                          if (actor != CaptureState::instance()->getCapture())
                            Orig(actor);
                        }
                      };
                      CheckForCapture::InstallAtSymbol("_ZN2rs26setFlagOnStartKoopaCaptureEPKN2al9LiveActorE");
                    }},
        CaptureInfo{"AnagramAlphabetCharacter", nullptr},
        CaptureInfo{"Yoshi",
                    []() {
                      struct YoshiNeverSpawned : Trampoline<YoshiNeverSpawned> {
                        static void Callback(al::ActorStateBase* state) {
                          unsafeRef<bool>(state, 0x48) = false;
                          //                          al::makeMtxRT(&unsafeRef<sead::Matrix34f>(state->mActor, 0x188),
                          //                                        CaptureState::instance()->getMario());
                          al::copyPose(state->mActor, CaptureState::instance()->getMario());
                          Orig(state);
                        }
                      };

                      YoshiNeverSpawned::InstallAtSymbol("_ZN13YoshiStateEgg6appearEv");
                    },
                    nullptr,
                    [](al::LiveActor* actor) {
                      auto func = getFunc<void, al::LiveActor*>("_ZN8YoshiEgg16initPlacementEggEv");
                      auto egg = unsafeRef<al::LiveActor*>(unsafeRef<al::NerveStateBase*>(actor, 0x168), 0x38);
                      //                      func(egg);
                    }},
        CaptureInfo{"KillerLauncher", nullptr, [](al::LiveActor* actor, auto) {
                      patch::CodePatcher patch(0x147eb4);
                      patch.WriteInst(inst::Movz(reg::W0, par::get("BanzaiBill", false)));
                    }, [](auto) {
                      patch::CodePatcher patch(0x147eb4);
                      patch.BranchLinkInst((void*)(bool(*)(const char*, const char*))al::isEqualString);
                    }, sead::Vector3f::zero, [](sead::Vector3f& pos) {
                      if (par::get("BanzaiBill", false)) {
                        pos += sead::Vector3f::ey * par::get("BanzaiBillHeight", 300.f);
                      }
                    }},
        CaptureInfo{"Fukankun"},
        CaptureInfo{"Wanwan"},
//        CaptureInfo{"PossessedMapParts"},
        CaptureInfo{"HackFork"},
        CaptureInfo{"PackunFire"},
    };

    return {std::data(captures), std::size(captures)};
  }
} // namespace cs