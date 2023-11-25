#include <bonk/mods/PlayerConstMod.hpp>

namespace bm {
    static constexpr const u8 playerConstFloatLut[] = {
        0xff, 0x59, 0xff, 0xff, 0x3f, 0x1c, 0xf8, 0x57, 0x26, 0xa0, 0x6e, 0xc0, 0xe7, 0x07, 0xfc, 0xff,
        0x8c, 0xff, 0x7f, 0xfe, 0xfc, 0xef, 0xe1, 0xfe, 0x7f, 0xfc, 0xff, 0xff, 0xf1, 0xdf, 0xff, 0xff,
        0xff, 0xcf, 0x77, 0xe7, 0x3f, 0x8c, 0xef, 0xff, 0xff, 0xc3, 0xff, 0xe3, 0x84, 0xfd, 0xff, 0xbf,
        0x57, 0xfe, 0xf7, 0x7f, 0xff, 0xf8, 0xfe, 0xf4, 0xff, 0x7f, 0x0e, 0x3e, 0x7e, 0xfe, 0x6f, 0xbf,
        0xc0, 0xff, 0xdf, 0xf7, 0xe3, 0xff, 0x7f, 0xfc, 0xff, 0xdf, 0x1f, 0x00, 0x70};
    static constexpr const int playerConstCount = 615;

    bool PlayerConstMod::isFloat(int index) { return (playerConstFloatLut[index / 8] & (1 << (index % 8))) != 0; }
    void PlayerConstMod::sceneStart(const al::ActorInitInfo& initInfo) {
        Mod::sceneStart(initInfo);
        basePlayerConst = *getMario()->mPlayerConst;
        if (active)
            randomize();
    }
    void PlayerConstMod::activate() {
        Mod::activate();
        if (notInScene()) return;
        randomize();
    }
    void PlayerConstMod::randomize() {
        f32* baseCur = &basePlayerConst.mGravity;
        f32* cur = &getMario()->mPlayerConst->mGravity;
        for (int i = 0; i < par::get("ConstMaxChanges", 15); i++) {
            int index = al::getRandom(playerConstCount);
            if (isFloat(index))
                cur[index] *= baseCur[index] * al::getRandom(0.8f, 1.2f);
        }
        pauseForSeconds(par::get("ConstRandoSecs", 5));
    }
    void PlayerConstMod::control() {
//        randomize();
    }

} // namespace bm
