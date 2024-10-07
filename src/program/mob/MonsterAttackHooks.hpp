#pragma once

#include "MonsterState.hpp"

#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <logger/Logger.hpp>
#include <utils/ForwardDecls.hpp>
#include <utils/Helpers.h>

namespace mob {
  template <typename F>
  void writeIsMsgAttackHook(uintptr_t branchOffset, F hook) {
    auto& branch = unsafeRef<const inst::impl::op101x::UnconditionalBranchImmediate>(
        exl::util::modules::GetTargetOffset(branchOffset), 0);

    struct {
      int v : 26;
    } extended = {.v = branch.GetImm26()};

    static F storedHook = hook;
    static auto originalCheck =
        reinterpret_cast<bool (*)(al::SensorMsg*)>(exl::util::modules::GetTargetOffset(branchOffset + extended.v * 4));

    static bool (*newBranchTarget)(al::SensorMsg*) = [](al::SensorMsg* message) {
      return storedHook(message) || originalCheck(message);
    };

    patch::CodePatcher patcher(branchOffset);
    patcher.BranchLinkInst((void*)newBranchTarget);
  }

  template <typename F>
  void writeSendMsgAttackHook(uintptr_t branchOffset, F hook) {
    auto& branch = unsafeRef<const inst::impl::op101x::UnconditionalBranchImmediate>(
        exl::util::modules::GetTargetOffset(branchOffset), 0);

    struct {
      int v : 26;
    } extended = {.v = branch.GetImm26()};

    static F storedHook = hook;
    static auto originalCheck = reinterpret_cast<bool (*)(al::HitSensor*, al::HitSensor*)>(
        exl::util::modules::GetTargetOffset(branchOffset + extended.v * 4));

    Logger::log("new offset: %zx\n", branchOffset + extended.v * 4);
    static bool (*newBranchTarget)(al::HitSensor*, al::HitSensor*) = [](al::HitSensor* left,
                                                                        al::HitSensor* right) -> bool {
      auto leftActor = al::getSensorHost(left);
      auto rightActor = al::getSensorHost(right);
      if (!MonsterState::instance()->canAffect(leftActor, rightActor))
        return false; // originalCheck(left, right);
      return storedHook(left, right) || originalCheck(left, right);
    };
    Logger::log("woo %p\n", newBranchTarget);

    patch::CodePatcher patcher(branchOffset);
    if (branch.GetOp() == inst::impl::op101x::UnconditionalBranchImmediate::BL)
      patcher.BranchLinkInst((void*)newBranchTarget);
    else
      patcher.BranchInst((void*)newBranchTarget);
  }

  bool sendMsgEnemyAttack(al::HitSensor* l, al::HitSensor* r);
} // namespace mob