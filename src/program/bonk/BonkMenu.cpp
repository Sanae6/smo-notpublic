#include <al/Library/Nerve/NerveSetupUtil.h>
#include <al/Library/Nerve/NerveUtil.h>
#include <imgui.h>
#include <logger/Params.h>
#include <program/bonk/BonkMenu.hpp>

namespace bm {
    BonkMenu* BonkMenu::instance = nullptr;
    namespace {
        MAKE_NERVE_BOTH(BonkMenu, Appear)
        MAKE_NERVE_BOTH(BonkMenu, Wait)
        MAKE_NERVE_BOTH(BonkMenu, Hide)
    } // namespace
    BonkMenu::BonkMenu() : al::NerveExecutor("BM") {
        initNerve(&BonkMenuNrvHide::sInstance, 0);
        al::NerveExecutor::getNerveKeeper()->mStep = 60;
    }
    void BonkMenu::appear(const char* textIn) {
        text = textIn;
        if (al::isNerve(this, &BonkMenuNrvHide::sInstance)) {
            al::setNerve(this, &BonkMenuNrvAppear::sInstance);
        }
    }
    void BonkMenu::control() {
        if (par::clicked("ShowWindow")) {
            al::setNerve(this, &BonkMenuNrvWait::sInstance);
        }
        updateNerve();
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        const char* effectiveText = text ?: "Placeholder text";
        char* awoo = (char*)alloca(strlen("New Mod!\n") + strlen(effectiveText) + 1);
        sprintf(awoo, "New Mod!\n%s", effectiveText);
        auto textSize = ImGui::CalcTextSize(awoo);
        ImGui::SetNextWindowContentSize(textSize);
        auto displaySize = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f - textSize.x * 0.5f, displaySize.y * 0.75f));
        ImGui::Begin("BonkMenu", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
        ImGui::Text("%s", awoo);
        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
    void BonkMenu::exeAppear() { alpha = al::calcNerveRate(this, 30);
        if (al::isGreaterEqualStep(this, 30)) {
            al::setNerve(this, &BonkMenuNrvWait::sInstance);
        } }
    void BonkMenu::exeWait() {
        alpha = 1;
        if (al::isGreaterEqualStep(this, 300)) {
            al::setNerve(this, &BonkMenuNrvHide::sInstance);
        }
    }
    void BonkMenu::exeHide() { alpha = 1 - al::calcNerveRate(this, 60); }
} // namespace bm
