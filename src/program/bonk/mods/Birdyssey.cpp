#include <al/Library/LiveActor/ActorActionFunction.h>
#include <al/Library/LiveActor/ActorFlagFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/LiveActor/LiveActorGroup.h>
#include <al/Library/Math/MathVectorUtil.h>
#include <al/Library/Memory/HeapUtil.h>
#include <basis/seadNew.h>
#include <bonk/mods/Birdyssey.hpp>
#include <logger/Logger.hpp>
#include <thread/seadThread.h>

namespace bm {
    static constexpr const char* birdTypes[] = {
        "BirdCap",  "BirdCity", "BirdClash", "BirdCloud", "BirdEagle", "BirdForest", "BirdGull",
        "BirdLake", "BirdLava", "BirdMoon",  "BirdSand",  "BirdSky",   "BirdSnow",   "BirdWaterfall",
    };
    static constexpr const s32 birdTypeCount = (s32)std::size(birdTypes);
    static int birdSelections[birdTypeCount] = {-1};
    static constexpr const s32 maxBirdTypes = (s32)std::size(birdSelections);
    static const int maxBirds = maxBirdTypes;
    struct AngryBird : public al::LiveActor {
        sead::Vector3f startPos;
        sead::Vector3f targetPos;
        s32 suffix;
        PlayerActorHakoniwa* mario;
        AngryBird(s32 suffix, PlayerActorHakoniwa* mario) : al::LiveActor("AngryBird"), suffix(suffix), mario(mario) {}
        void init(const al::ActorInitInfo& initInfo) override {
            al::initActorWithArchiveName(this, initInfo, birdTypes[suffix], nullptr);
            makeActorDead();
            al::invalidateClipping(this);
        }

        void appear() override {
            if (rs::isPlayerInWater(this))
                return;
            LiveActor::appear();
            auto playerPos = al::getTrans(mario);
            sead::Vector3f randomVec = sead::Vector3f::zero;
            al::getRandomVector(&randomVec, par::get("AngryBirdsDistance", 100.0f));
            al::setTrans(this, startPos = playerPos + randomVec);
            targetPos = playerPos - randomVec;
        }

        void control() override {
            LiveActor::control();
            if (al::isActionEnd(this)) {
                al::startAction(this, "Fly");
            }
            //            Logger::log("what the fuck is going on\n");
            auto dir = targetPos - al::getTrans(this);
            auto speed = par::get("AngryBirdsSpeed", 3.0f);
            if (dir.length() < speed * 2.0f) {
                kill();
            }
            dir.normalize();
            al::setTrans(this, al::getTrans(this) + dir * par::get("AngryBirdsSpeed", 3.0f));
            dir.y = 0;
            dir.normalize();
            sead::Quatf quat;
            al::makeQuatUpFront(&quat, sead::Vector3f::ey, dir);
            al::updatePoseQuat(this, quat);
        }

        void attackSensor(al::HitSensor* target, al::HitSensor* source) override {
            if (al::isSensorMapObj(target))
                al::sendMsgEnemyAttack(source, target);
        }
    };

    void Birdyssey::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        if (birdSelections[0] == -1) {
            birdSelections[0] = al::getRandom(birdTypeCount);
            birdSelections[1] = al::getRandom(birdTypeCount);
            birdSelections[2] = al::getRandom(birdTypeCount);
        }
        birds = allocArray<al::LiveActor*>(maxBirds);
        for (int i = 0; i < maxBirds; i++) {
            auto bird = alloc<AngryBird>(birdSelections[al::getRandom(maxBirdTypes)], getMario());
            al::initCreateActorNoPlacementInfo(bird, initInfo);
            birds[i] = bird;
        }
        pauseForFrames(par::get("AngryBirdsCooldown", 60));
    }
    void Birdyssey::control() {
        Mod::control();
        if (par::get("AngryBirdsDisabled", false))
            return;

        al::LiveActor* bird = nullptr;
        for (int i = 0; i < maxBirds; i++) {
            s32 r = al::getRandom(maxBirds);
            auto tempBird = birds[r];
            if (tempBird && al::isDead(tempBird)) {
                bird = tempBird;
                break;
            }
        }
        if (!bird)
            bird = findDeadBird();
        if (bird)
            bird->appear();

        pauseForFrames(par::get("AngryBirdsCooldown", 60));
    }
    al::LiveActor* Birdyssey::findDeadBird() {
        for (int i = 0; i < maxBirds; ++i) {
            if (al::isDead(birds[i]))
                return birds[i];
        }
        return nullptr;
    }
    void Birdyssey::sceneEnd(bool cleanResources) {
        Mod::sceneEnd(cleanResources);
        if (cleanResources)
            birdSelections[0] = -1;
    }

} // namespace bm
