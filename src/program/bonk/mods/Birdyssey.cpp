#include <al/Library/LiveActor/ActorActionFunction.h>
#include <al/Library/LiveActor/ActorFlagFunction.h>
#include <al/Library/LiveActor/ActorPoseKeeper.h>
#include <al/Library/LiveActor/ActorSensorFunction.h>
#include <al/Library/LiveActor/LiveActorGroup.h>
#include <al/Library/Math/MathVectorUtil.h>
#include <al/Library/Memory/HeapUtil.h>
#include <bonk/mods/Birdyssey.hpp>
#include <logger/Logger.hpp>

namespace bm {
    static constexpr const char* birdTypeSuffixes[] = {
        "BirdCap",  "BirdCity", "BirdClash", "BirdCloud", "BirdEagle", "BirdForest", "BirdGull",
        "BirdLake", "BirdLava", "BirdMoon",  "BirdSand",  "BirdSky",   "BirdSnow",   "BirdWaterfall",
    };
    static constexpr const s32 birdTypeCount = (s32)std::size(birdTypeSuffixes);
    struct AngryBird : public al::LiveActor {
        sead::Vector3f startPos;
        sead::Vector3f targetPos;
        s32 suffix;
        PlayerActorHakoniwa* mario;
        AngryBird(s32 suffix, PlayerActorHakoniwa* mario) : al::LiveActor("AngryBird"), suffix(suffix), mario(mario) {}
        void init(const al::ActorInitInfo& initInfo) override {
            al::initActorWithArchiveName(this, initInfo, birdTypeSuffixes[suffix], nullptr);
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
                al::startAction(this, "Flying");
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
        Logger::log("Heap name %s\n", al::getCurrentHeap()->getName().cstr());
        birds = new al::LiveActor*[birdTypeCount];
        for (int i = 0; i < birdTypeCount; i++) {
            auto bird = new AngryBird(i, getMario());
            al::initCreateActorNoPlacementInfo(bird, initInfo);
            birds[i] = bird;
        }
        Logger::log("Initialized birds\n");
        pauseForFrames(par::get("AngryBirdsCooldown", 60));
    }
    void Birdyssey::control() {
        Mod::control();
        Logger::log("Scene resources", al::getSceneResourceHeap()->getFreeSize(), );

        al::LiveActor* bird = nullptr;
        for (int i = 0; i < birdTypeCount; i++) {
            s32 r = al::getRandom(birdTypeCount);
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
        for (int i = 0; i < birdTypeCount; ++i) {
            if (al::isDead(birds[i]))
                return birds[i];
        }
        return nullptr;
    }

} // namespace bm
