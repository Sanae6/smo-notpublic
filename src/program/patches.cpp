#include "patches.hpp"

namespace patch = exl::patch;
namespace inst = exl::armv8::inst;
namespace reg = exl::armv8::reg;

void costumeRoomPatches() {
    patch::CodePatcher p(0x262850);
    p.WriteInst(inst::Movz(reg::W0, 0));
    p.Seek(0x2609B4);
    p.WriteInst(inst::Movz(reg::W0, 0));

    p.Seek(0x25FF74);
    p.WriteInst(inst::Movz(reg::W0, 1));
    p.Seek(0x25FF74);
    p.WriteInst(inst::Movz(reg::W0, 0));

    // CHECKPOINT WARP
    p.Seek(0x1f2998);
    p.WriteInst(inst::Movz(reg::X0, 0));
    p.WriteInst(inst::Ret());
}

void stubSocketInit() {
    patch::CodePatcher p(0x95C498);
    p.WriteInst(inst::Nop());
}

void enableDebugNvn() {
    patch::CodePatcher p(0x7312CC);
    p.WriteInst(inst::Nop());
}


void runCodePatches() {
    costumeRoomPatches();
    stubSocketInit();
    enableDebugNvn();
}