#include <scene/ScenePatches.hpp>
#include <scene/AmonglaSequence.hpp>
#include <lib.hpp>

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;

struct SequenceCreation : exl::hook::impl::ReplaceHook<SequenceCreation> {
    static al::Sequence* Callback() {
        return new seen::AmonglaSequence(seen::AmonglaSequence::name());
    }
};

void scenePatchesInit() {
    SequenceCreation::InstallAtSymbol("_ZN15SequenceFactory14createSequenceEPKc");
    patch::CodePatcher patcher(0x536580);
    patcher.Write(inst::Movz(reg::X0, 0));
}