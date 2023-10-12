#include "CoolScene.hpp"
#include <al/Library/Audio/AudioDirector.h>
#include <al/Library/Audio/System/AudioSystemFunction.h>
#include <al/Library/Memory/HeapUtil.h>
#include <al/Library/Nerve/NerveSetupUtil.h>
#include <al/Library/Sequence/SequenceFunction.h>
#include <al/Library/Shader/ForwardRendering/ShaderHolder.h>
#include <heap/seadHeapMgr.h>
#include <logger/Logger.hpp>
#include <scene/AmonglaSceneFactory.hpp>
#include <scene/AmonglaSequence.hpp>
namespace seen {
    namespace {
        NERVE_DEF(AmonglaSequence, LoadScene);
        NERVE_DEF(AmonglaSequence, Play);
    }
    using namespace al;
    AmonglaSequence::AmonglaSequence(const char* name) : al::Sequence(name) {}

    void AmonglaSequence::init(const al::SequenceInitInfo& initInfo) {
        initDrawSystemInfo(initInfo);

        mScreenCaptureExecutor = new ScreenCaptureExecutor(1);

        const auto physicalArea = getDrawInfo()->handheldRenderBuffer->getPhysicalArea();
        mScreenCaptureExecutor->createScreenCapture((s32)physicalArea.getSizeX(), (s32)physicalArea.getSizeY(), 0);
        AudioDirectorInitInfo audioInitInfo;
        audioInitInfo.seDirectorInitInfo.listenerCount = 1;
        AudioSystemInfo* audioSystemInfo = alAudioSystemFunction::getAudioSystemInfo(initInfo.gameSystemInfo);
        al::initAudioDirector(this, audioSystemInfo, audioInitInfo);
        initAudioKeeper("Sequence");

        GameDataHolder* holder = new GameDataHolder(initInfo.gameSystemInfo->messageSystem);
        al::initSceneCreator(this, initInfo, holder, mAudioDirector, mScreenCaptureExecutor,
                             new AmonglaSceneFactory("アモンガッスのシーン生成"));

        al::ShaderHolder::instance()->initAndLoadAllFromDir("ShaderData", al::findNamedHeap("StationedResourceHeap"), nullptr);

        mLayoutKit = new al::LayoutKit(initInfo.gameSystemInfo->fontHolder);
        mLayoutKit->setLayoutSystem(initInfo.gameSystemInfo->layoutSystem);
        mLayoutKit->setDrawContext(getDrawInfo()->drawContext);
        mLayoutKit->createExecuteDirector(0x80);

        initNerve(&AmonglaSequenceNrvLoadScene::sInstance, 0);
    }

    void AmonglaSequence::update() { Sequence::update(); }
    void AmonglaSequence::drawMain() const {
        Sequence::drawMain();
        agl::RenderBuffer* renderBuffer = getDrawInfo()->getRenderBuffer();
        agl::DrawContext* drawContext = getDrawInfo()->drawContext;
        mScreenCaptureExecutor->tryCaptureAndDraw(drawContext, renderBuffer, 0);
        sead::Viewport viewport(*renderBuffer);
        viewport.apply(drawContext, *renderBuffer);
        renderBuffer->bind(drawContext);
        al::setRenderBuffer(this->mLayoutKit, renderBuffer);
        al::executeDraw(mLayoutKit, "２Ｄバック（メイン画面）");
        al::executeDraw(mLayoutKit, "２Ｄベース（メイン画面）");
        al::executeDraw(mLayoutKit, "２Ｄオーバー（メイン画面）");
    }

    void AmonglaSequence::exeLoadScene() {
        if (al::isFirstStep(this)) {
            al::createSceneHeap("CoolScene", false);
            sead::ScopedCurrentHeapSetter heapSetter(al::getSceneHeap());
            Logger::log("Heap name: %s %x\n", al::getCurrentHeap()->getName().cstr(),
                        al::getCurrentHeap()->getFreeSize());
            auto* scene = new CoolScene("CoolScene");
            al::setSceneAndUseInitThread(this, scene, 0, "WorldMapStage", 1,
                                         "Sequence=AmonglaSequence ", nullptr);
            Logger::log("Ieaw\n");
        }

        if (al::tryEndSceneInitThread(this)) {
            al::setNerve(this, &AmonglaSequenceNrvPlay::sInstance);
        }
    }
    void AmonglaSequence::exePlay() {}
} // namespace seen
