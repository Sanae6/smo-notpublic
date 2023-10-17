#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/LiveActor/LiveActor.h>
#include <al/Library/MapObj/FixMapParts.h>
#include <bonk/ForwardDecls.hpp>
#include <bonk/mods/PoseRandomizer.hpp>
#include <logger/Logger.hpp>
#include <random/seadGlobalRandom.h>
#include <typeinfo>
#include <utils/Helpers.h>

namespace bm {
    struct PoseMapEntry {
        const std::type_info& typeInfo;
        size_t floats;
    };
    template <typename T>
    constexpr PoseMapEntry poseMap(size_t ignoredSize = 0) {
        return PoseMapEntry{typeid(T), (sizeof(T) - offsetof(T, mTrans) - ignoredSize) / 4};
    }
    static std::array<PoseMapEntry, 9> poseMaps = {
        poseMap<al::ActorPoseKeeperTFSV>(),
        poseMap<al::ActorPoseKeeperTFGSV>(),
        poseMap<al::ActorPoseKeeperTFUSV>(sizeof(u32)),
        poseMap<al::ActorPoseKeeperTQSV>(),
        poseMap<al::ActorPoseKeeperTQGSV>(),
        poseMap<al::ActorPoseKeeperTQGMSV>(sizeof(sead::Matrix34f)),
        poseMap<al::ActorPoseKeeperTRSV>(),
        poseMap<al::ActorPoseKeeperTRMSV>(sizeof(sead::Matrix34f)),
        poseMap<al::ActorPoseKeeperTRGMSV>(sizeof(sead::Matrix34f)),
    };

    static std::array<const std::type_info*, 5> fixMapParts = {
        &typeid(al::FixMapParts),
        &typeid(al::OneMeshFixMapParts),
        &typeid(FixMapPartsCapHanger),
        &typeid(FixMapPartsForceSafetyPoint),
        &typeid(FixMapPartsFukankunZoomCapMessage),
    };

    void PoseRandomizer::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        kit = initInfo.mAllActorsGroup;
    }
    void PoseRandomizer::control() {
        if (notInScene())
            return;
//        Logger::log("Pose randomizer control %d, %d\n", kit->mActorCount, kit->mMaxActorCount);
        for (int i = 0; i < par::get("PoseRCount", 5); ++i) {
            auto actor = kit->mActors[al::getRandom(0, kit->mActorCount)];
            hitActor(actor);
        }
        pauseForSeconds(par::get("PoseRWait", 15));
    }
    void PoseRandomizer::hitActor(al::LiveActor* actor) const {
        auto poseKeeper = actor->getPoseKeeper();
        if (poseKeeper == nullptr) {
            return;
        }
        PoseMapEntry* entry = nullptr;
        for (u32 j = 0; j < poseMaps.size(); j++) {
            //                Logger::log("Available pose %s\n", poseMaps[j].typeInfo.name());
            if (isSameType(poseKeeper, poseMaps[j].typeInfo)) {
                entry = &poseMaps[j];
                break;
            }
        }
        EXL_ASSERT(entry != nullptr, "Not in the list (%zu), %s has %s", poseMaps.size(), typeid(*actor).name(),
                   typeid(*poseKeeper).name());
        //            Logger::log("Ass: %02d - %s\n", entry->size, entry->typeInfo.name());
        auto pose = ((f32*)&actor->getPoseKeeper()->mTrans); // skip vtable
        for (u32 j = 0; j < entry->floats; j++) {
            pose[j] += al::getRandom(par::get("PoseRLow", -5.0f), par::get("PoseRHigh", 5.0f));
        }

        // FixMapParts need a little bit of nudging to work properly
        for (int j = 0; j < fixMapParts.size(); ++j) {
            if (isSameType(actor, *fixMapParts[j])) {
                actor->calcAnim();
                actor->movement();
                break;
            }
        }
    }
} // namespace bm
