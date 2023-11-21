#include <al/Library/Audio/AudioKeeper.h>
#include <bonk/mods/DiscordPing.hpp>
#include <logger/Logger.hpp>

namespace bm {
    al::AudioKeeper* DiscordPing::getAudioKeeper() const { return audioKeeper; }
    void DiscordPing::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        audioKeeper = alAudioKeeperFunction::createAudioKeeper(initInfo.mAudioDirector);
        audioKeeper->initSeKeeper(initInfo.mAudioDirector, "SePlayObjDiscord", nullptr, nullptr, nullptr, nullptr);
        if (!isPaused()) pauseMe();
    }
    void DiscordPing::activate() {
        Mod::activate();
        pauseMe();
    }
    void DiscordPing::control() {
        Mod::control();
        al::startSe(this, "Ping");
        Logger::log("Started se\n");
        pauseMe();
    }
    void DiscordPing::pauseMe() {
        pauseForSeconds(al::getRandom(par::get("PingRangeStart", 30),
                                      par::get("PingRangeStart", 30) + par::get("PingRangeLength", 30)));
    }
} // namespace bm
