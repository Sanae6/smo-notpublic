#include "MonsterAttackHooks.hpp"
#include "MonsterState.hpp"
#include "al/Library/LiveActor/ActorFlagFunction.h"
#include "al/Library/LiveActor/ActorMovementFunction.h"
#include "al/Library/LiveActor/ActorSensorFunction.h"
#include "logger/Logger.hpp"
#include "utils/Helpers.h"

#include <al/Library/LiveActor/ActorActionFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/LiveActor/LiveActorGroup.h>
#include <al/Library/Nerve/NerveStateBase.h>
#include <al/Library/Placement/PlacementFunction.h>
#include <al/Library/Placement/PlacementInfo.h>
#include <al/Library/Poser/ActorCameraTarget.h>
#include <al/Library/Rail/Rail.h>
#include <al/Library/Rail/RailKeeper.h>
#include <al/Library/Rail/RailPart.h>
#include <al/Library/Rail/RailRider.h>
#include <logger/Params.h>
#include <mob/MonsterInfo.hpp>
#include <rs/util/SensorUtil.h>
#include <utils/ForwardDecls.hpp>

namespace mob {
  std::span<const MonsterInfo> MonsterInfo::getMonsters() {
    static al::LiveActor* (*getEnemy)(const al::LiveActor*) = [](const al::LiveActor* actor) {
      return MonsterState::instance()->getEnemy(actor);
    };
    static sead::Vector3f storedPosition = sead::Vector3f::zero;
    static const MonsterInfo Monsters[]{
        // MonsterInfo{"KuriboWing",
        //             [] {
        //               patch::CodePatcher patcher(0x15b12c);
        //               patcher.WriteInst(inst::Movz(reg::X0, 1));
        //               Logger::log("Setup KuriboWing\n");
        //             },
        //             [](al::LiveActor* actor, auto, const sead::Vector3f& position) {
        //               actor->mRailKeeper = alloc<al::RailKeeper>();
        //               auto* rail = alloc<al::Rail>();
        //
        //               rail->mRailPart = allocArray<al::RailPart>(1);
        //               const auto newPosition = position + sead::Vector3f::ey * 500;
        //               rail->mRailPart[0].init(position, position, newPosition, newPosition);
        //               rail->mRailPartCount = 1;
        //               rail->mRailPointsCount = 2;
        //               rail->mRailPoints = allocArray<al::PlacementInfo*>(2);
        //               std::fill_n(rail->mRailPoints, 2, nullptr);
        //               rail->isClosed = false;
        //               actor->mRailKeeper->mRailRider = alloc<al::RailRider>(rail);
        //               actor->getRailRider()->setSpeed(1);
        //               Logger::log("Set up rail %p %p %p\n", actor->mRailKeeper, actor->getRailRider(), rail);
        //             }},
        MonsterInfo{
            "TRexPatrol",
            [] {
              struct Builder {
                sead::Vector3f currentRoute;
                sead::Vector3f newRoute;
                al::LiveActor* me;
                void recalculateRoute() {
                  currentRoute = al::getTrans(me);
                  newRoute = MonsterState::instance()->calcEnemyPosition(me);
                }
              };
              static struct PatrolRouteBuilder {
                void (*init)(Builder*);
                void (*calcRoute)(Builder*, sead::Vector3f**, sead::Vector3f**);
                void (*calcNextGoal)(Builder*, sead::Vector3f**, const sead::Vector3f*);
              } routeBuilderFunctions = {
                  .init = [](Builder* builder) { builder->me = MonsterState::instance()->currentlyInitializing; },
                  .calcRoute =
                      [](Builder* builder, sead::Vector3f** firstRoute, sead::Vector3f** secondRoute) {
                        builder->recalculateRoute();
                        *firstRoute = &builder->currentRoute;
                        *secondRoute = &builder->newRoute;
                      },
                  .calcNextGoal =
                      [](Builder* builder, sead::Vector3f** position, const sead::Vector3f* previous) {
                        builder->recalculateRoute();
                        if (previous == &builder->currentRoute)
                          *position = &builder->newRoute;
                        else
                          *position = &builder->currentRoute;
                      }};
              patch::CodePatcher patcher(0x1978e0);
              patcher.WriteInst(inst::Movz(reg::X0, true));
              patcher.WriteInst(inst::Ret());
              patcher.Seek("_ZN22TRexPatrolRouteBuilder4initERKN2al13ActorInitInfoE", 0);
              patcher.BranchInst((void*)routeBuilderFunctions.init);
              patcher.Seek("_ZNK22TRexPatrolRouteBuilder14calcFirstRouteEPPK17TRexRouteInfoBaseS3_", 0);
              patcher.BranchInst((void*)routeBuilderFunctions.calcRoute);
              patcher.Seek(
                  "_ZNK22TRexPatrolRouteBuilder16calcNearestRouteEPPK17TRexRouteInfoBaseS3_RKN4sead7Vector3IfEE", 0);
              patcher.BranchInst((void*)routeBuilderFunctions.calcRoute);
              patcher.Seek("_ZNK22TRexPatrolRouteBuilder17calcNextRouteGoalEPPK17TRexRouteInfoBaseS2_", 0);
              patcher.BranchInst((void*)routeBuilderFunctions.calcNextGoal);
              patcher.Seek(
                  "_ZNK22TRexPatrolRouteBuilder17calcRecoveryRouteEPPK17TRexRouteInfoBaseS3_RKN4sead7Vector3IfEE", 0);
              patcher.BranchInst((void*)routeBuilderFunctions.calcRoute);
            },
        },
        MonsterInfo{"KuriboPossessed",
                    [] {
                      patch::CodePatcher patcher(0x150ee4);
                      // patcher.WriteInst(inst::Movz(reg::X0, 1));
                      patcher.Seek(0x150ec8);
                      patcher.WriteInst(inst::Movz(reg::X0, 1));
                      writeSendMsgAttackHook(0x151018, mob::sendMsgEnemyAttack);
                    }},
        // MonsterInfo{"Kakku", nullptr},
        MonsterInfo{"Senobi", [] { writeSendMsgAttackHook(0x181194, mob::sendMsgEnemyAttack); }},
        MonsterInfo{"FireBrosPossessed",
                    [] {
                      // ph::nop(patch::CodePatcher(0x100e38));
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      // write patches for send enemy attacks (hammer included)
                      mob::writeSendMsgAttackHook(0x100eb8, mob::sendMsgEnemyAttack);
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
                    [](auto actor, auto, auto p) {
                      storedPosition = p;
                      unsafeRef<bool>(actor, 0x168) = true;
                    }},
        MonsterInfo{"Tank", nullptr},
        MonsterInfo{"Gamane",
                    [] {
                      patch::CodePatcher p(0x104190);
                      p.BranchLinkInst(reinterpret_cast<void*>(getEnemy));
                      p.Seek(0x1047b4);
                      p.BranchLinkInst(reinterpret_cast<void*>(getEnemy));
                      p.Seek(0x1043d4);
                      p.BranchLinkInst(reinterpret_cast<void*>(getEnemy));
                      p.Seek(0x104518);
                      p.BranchLinkInst(reinterpret_cast<void*>(getEnemy));
                      p.Seek(0x1042c0);
                      p.BranchLinkInst(reinterpret_cast<void*>(getEnemy));
                    }},
        MonsterInfo{"Imomu",
                    [] {
                      struct IommuControl : Trampoline<IommuControl> {
                        static void Callback(al::LiveActor* actor) {
                          Orig(actor);

                          if (!MonsterState::instance()->isAMonster(actor))
                            return;

                          auto& matrix = unsafeRef<sead::Matrix34f>(actor, 0x18c);
                          matrix.makeST(sead::Vector3f::ones, al::getTrans(actor));
                        }
                      };
                      IommuControl::InstallAtSymbol("_ZN5Imomu7controlEv");
                    }},
        // MonsterInfo{"Byugo", nullptr, nullptr, nullptr, sead::Vector3f::ey * 500.f},
        MonsterInfo{"Hosui",
                    [] {

                    }},
        MonsterInfo{"HammerBrosPossessed", nullptr,
                    [](auto actor, auto, auto p) {
                      storedPosition = p;
                      unsafeRef<bool>(actor, 0x168) = true;
                      // ph::nop(patch::CodePatcher(0x112b83));
                      mob::writeSendMsgAttackHook(0x112be4, mob::sendMsgEnemyAttack);
                    }},
        MonsterInfo{"Tsukkun", nullptr}, MonsterInfo{"KaronWing", nullptr}, MonsterInfo{"Bull", nullptr},
        MonsterInfo{"KillerLauncher",
                    [] {
                      struct KillerTarget : Trampoline<KillerTarget> {
                        static void Callback(al::LiveActor* actor, int unk) {
                          Orig(actor, unk);
                          MonsterState::instance()->setTarget(actor);
                        }
                      };

                      KillerTarget::InstallAtSymbol("_ZN6Killer6launchEi");
                    },
                    [](al::LiveActor*, auto, auto) {
                      patch::CodePatcher patch(0x147eb4);
                      patch.WriteInst(inst::Movz(reg::W0, par::get("BanzaiBill", false)));
                      patch.Seek(0x148678); // prevent more than one shot
                      patch.WriteInst(inst::Movz(reg::W8, 1));
                    },
                    [](auto, auto, auto) {
                      patch::CodePatcher patch(0x147eb4);
                      patch.BranchLinkInst((void*)static_cast<bool (*)(const char*, const char*)>(al::isEqualString));
                    },
                    sead::Vector3f::zero,
                    [](sead::Vector3f& pos) {
                      if (par::get("BanzaiBill", false)) {
                        pos += sead::Vector3f::ey * par::get("BanzaiBillHeight", 300.f);
                      }
                    }},
        MonsterInfo{"Fukankun"}, MonsterInfo{"Wanwan"},
        MonsterInfo{"PackunFire", [] { patch::CodePatcher(0x170264).WriteInst(inst::Movz(reg::W8, 0x200)); }},
        MonsterInfo{"PopnGenerator",
                    [] {
                      patch::CodePatcher patcher(0x174a24);
                      patcher.WriteInst(inst::Ret());
                    },
                    nullptr,
                    [](al::LiveActor* actor, auto, auto) {
                      // auto target = unsafeRef<al::LiveActorGroup*>(actor, 0x108)->mActors[0];
                      // MonsterState::instance()->setTarget(target);
                    },
                    {},
                    nullptr,
                    [](auto* actor) -> al::LiveActor* {
                      auto group = unsafeRef<al::LiveActorGroup*>(actor, 0x108);
                      sead::Vector3f start = sead::Vector3f::ey * 1000000.0f; // very far
                      auto returnActor = actor;
                      auto enemy = MonsterState::staticGetEnemy(actor);
                      for (int i = 0; i < group->mActorCount; i++) {
                        if (al::isAlive(group->mActors[i]) &&
                            al::calcDistance(enemy, start) > al::calcDistance(enemy, group->mActors[i])) {
                          returnActor = group->mActors[i];
                          start = al::getTrans(group->mActors[i]);
                          // Logger::log("consider %d\n", i);
                        }
                      }
                      return returnActor;
                    }},
        MonsterInfo{"Mummy",
                    [] {
                      patch::CodePatcher patcher(0x1654b0);
                      patcher.WriteInst(inst::Nop());
                    }},
        MonsterInfo{"LongGenerator",
                    [] {
                      patch::CodePatcher patcher(0x1615ec);
                      patcher.WriteInst(inst::Movz(reg::W0, 1));
                      patcher.WriteInst(inst::Ret());
                      patcher.Seek(0x161b8c);
                      patcher.WriteInst(inst::Ret());
                    },
                    nullptr,
                    [](auto* actor, auto, auto& pos) {
                      auto group = unsafeRef<al::LiveActorGroup*>(actor, 0x108);
                      for (int i = 0; i < group->mActorCount; i++) {
                        al::setTrans(group->mActors[i], pos);
                      }
                    },
                    {},
                    nullptr,
                    [](auto* actor) -> al::LiveActor* {
                      auto group = unsafeRef<al::LiveActorGroup*>(actor, 0x108);

                      sead::Vector3f start = sead::Vector3f::ey * 1000000.0f; // very far
                      auto returnActor = actor;
                      auto enemy = MonsterState::staticGetEnemy(actor);
                      for (int i = 0; i < group->mActorCount; i++) {
                        if (al::isAlive(group->mActors[i]) &&
                            al::calcDistance(enemy, start) > al::calcDistance(enemy, group->mActors[i])) {
                          returnActor = group->mActors[i];
                          start = al::getTrans(group->mActors[i]);
                        }
                      }
                      return returnActor;
                    }},
        MonsterInfo{"KuriboMini"}, MonsterInfo{"PackunPoison"}, MonsterInfo{"Donsuke"},
        MonsterInfo{"Togezo",
                    [] {
                      // writeSendMsgAttackHook(0x18eb20, [](auto* left, auto* right) {
                      //   return rs::sendMsgPlayerSwordAttack(left, right) || al::sendMsgExplosion(left, right,
                      //   nullptr);
                      // });
                    }},
        MonsterInfo{"CatchBomb"},
        MonsterInfo{"Gunetter",
                    [] {
                      struct Body : Trampoline<Body> {
                        static void Callback(int* ptr, const al::ActorInitInfo& initInfo, const char* key) {
                          Logger::log("owo %s %d %p %p\n", key, *ptr, ptr, ((uintptr_t)ptr) - 0x108);
                          Orig(ptr, initInfo, key);
                          Logger::log("uwu %s %d %p %p\n", key, *ptr, ptr, ((uintptr_t)ptr) - 0x108);
                        }
                      };
                      // Body::InstallAtSymbol("_ZN2al6getArgEPiRKNS_13ActorInitInfoEPKc");
                    },
                    [](al::LiveActor* actor, const al::ActorInitInfo& initInfo, auto&) {
                      int bodyNum = 0;
                      al::getArg(&bodyNum, initInfo, "BodyNum");
                      Logger::log("main %p\n", actor);
                      Logger::log("body b4: %p\n", unsafeOffset<int>(actor, 0x108));
                    },
                    [](al::LiveActor* actor, const al::ActorInitInfo& initInfo, auto&) {
                      int bodyNum = 0;
                      al::getArg(&bodyNum, initInfo, "BodyNum");
                      Logger::log("main %p\n", actor);
                      Logger::log("body af: %p\n", unsafeOffset<int*>(actor, 0x108));
                    }},
        MonsterInfo{"Grower"},
        MonsterInfo{
            "SmallWanderBoss",
            [] {
              patch::CodePatcher patcher(0xba294);
              patcher.WriteInst(inst::Nop());
              patcher.Seek(0x166e04);
              patcher.WriteInst(inst::Nop());
            },
            [](al::LiveActor* actor, const al::ActorInitInfo& initInfo, const sead::Vector3f& position) {
              if (!actor->mRailKeeper)
                Logger::log("oh we already have none\n");
              else
                Logger::log("oh cool we have one\n");
              actor->mRailKeeper = alloc<al::RailKeeper>();
              auto* rail = alloc<al::Rail>();

              rail->mRailPart = allocArray<al::RailPart>(1);
              const auto newPosition = position + sead::Vector3f::ey * 500;
              rail->mRailPart[0].init(position, position, newPosition, newPosition);
              rail->mRailPartCount = 1;
              rail->mRailPointsCount = 2;
              rail->mRailPoints = allocArray<al::PlacementInfo*>(2);
              std::fill_n(rail->mRailPoints, 2, const_cast<al::PlacementInfo*>(&initInfo.getPlacementInfo()));
              auto& pinfo = initInfo.getPlacementInfo().getPlacementIter();
              for (int i = 0; i < pinfo.getSize(); i++) {
                const char* key = nullptr;
                pinfo.getKeyName(&key, i);
                Logger::log("Key: %s\n", key);
              }
              rail->isClosed = false;
              actor->mRailKeeper->mRailRider = alloc<al::RailRider>(rail);
              actor->getRailRider()->setSpeed(1);
              Logger::log("Set up rail %p %p %p\n", actor->mRailKeeper, actor->getRailRider(), rail);
            },
            nullptr,
            sead::Vector3f::ey * 400.0f,
        }};

    return {std::data(Monsters), std::size(Monsters)};
  }
} // namespace mob