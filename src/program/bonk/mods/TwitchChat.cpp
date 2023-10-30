#include <al/Library/Memory/HeapUtil.h>
#include <bonk/mods/TwitchChat.hpp>
#include <gfx/seadGraphics.h>
#include <helpers/fsHelper.h>
#include <logger/Logger.hpp>

namespace bm {
    static u8 font[0xa8] = {};
    static bool registered = false;
    static nn::font::ResFont* getFont(){
        return reinterpret_cast<nn::font::ResFont*>(font);
    }
    struct FontAcquire : public Trampoline<FontAcquire> {
        static nn::font::ResFont* Callback(al::LayoutResource* res, nn::gfx::Device* device, const char* name) {
            Logger::log("Acquired font: %s\n", name);
            if (!registered) {
                Logger::log("Registering texture views\n");
                registered = true;
                getFont()->RegisterTextureViewToDescriptorPool(eui::RegisterSlotForTexture, nullptr);
                Logger::log("Registered texture views\n");
            }
//            if (al::isEqualString(name, "Inter") || al::isEqualString(name, "Inter.bffnt")) {
                return getFont();
//            }
//            return Orig(res, device, name);
        }
    };
    TwitchChat::TwitchChat() {
        FontAcquire::InstallAtSymbol("_ZN2al14LayoutResource11AcquireFontEPN2nn3gfx7TDeviceINS2_12ApiVariationINS2_7ApiTypeILi4EEENS2_10ApiVersionILi8EEEEEEEPKc");

        auto fontData = loadFont();
        auto header = static_cast<u8*>(fontData);
        for (int i = 0; i < sizeof(nn::font::BinaryFileHeader); ++i) {
            Logger::log("%02x ", header[i]);
        }
        Logger::log("\n");
        auto inter = new (getFont()) nn::font::ResFont();
        bool success = inter->SetResource(
            unsafeRef<nn::gfx::Device*>(sead::Graphics::instance(), 0x48),
            fontData,
            nullptr,
            0,
            0
        );
        Logger::log("Done setting up font, %s\n", BTOC(success));

    }
    void* TwitchChat::loadFont() {
        Logger::log("Twitch chat font %s %x\n", al::getCurrentHeap()->getName().cstr(), al::getCurrentHeap()->getFreeSize());
        const char* name = "content:/BonkData/Inter.bffnt";
        nn::fs::FileHandle handle;
        EXL_ASSERT(R_SUCCEEDED(nn::fs::OpenFile(&handle, name, nn::fs::OpenMode_Read)));
        long fileSize = 0;
        EXL_ASSERT(R_SUCCEEDED(nn::fs::GetFileSize(&fileSize, handle)));
        auto* fontData = al::getCurrentHeap()->alloc(fileSize, 0x1000);
        EXL_ASSERT(R_SUCCEEDED(nn::fs::ReadFile(handle, 0, fontData, fileSize)));
        nn::fs::CloseFile(handle);
        return fontData;
    }
    void TwitchChat::sceneStart(const al::ActorInitInfo& initInfo) { Mod::sceneStart(initInfo);
        layout = new al::SimpleLayoutText(getLayoutInitInfo(initInfo), "TestText", "TxtTest00", nullptr);
        layout->start(sead::Vector2f ::zero, "Awesome!!!!", INT32_MAX);
    }
    void TwitchChat::update() { Mod::update();
        layout->setText(par::get("TwitchChatTest", "Awesome Test Text"));
    }
} // namespace bm
