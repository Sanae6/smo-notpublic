#include "StaticManager.hpp"

#include "static_bin.h"
#include <agl/TextureSampler.h>
#include <agl/utl.h>
#include <al/Library/Controller/JoyPadUtil.h>
#include <al/Library/LiveActor/ActorModelFunction.h>
#include <game/System/Application.h>
#include <imgui.h>
#include <imgui_backend/imgui_impl_nvn.hpp>
#include <imgui_bin.h>
#include <imgui_nvn.h>
#include <init.h>
#include <logger/Logger.hpp>
#include <nvn/nvn_Cpp.h>
#include <utils/Helpers.h>

namespace sp {
  namespace {
    nvn::Program program;
    StaticManager* instance;
  } // namespace
  void StaticManager::init() {
    // Logger::log("init\n");
    // // steps: load and create shader
    //
    // // auto shader = ImguiShaderCompiler::CompileShader("static");
    // // auto binary = static_cast<u8*>(nn::init::GetAllocator()->Allocate(static_cast<s32>(static_bin_size)));
    //
    // auto shader = const_cast<u8*>(imgui_bin);
    // auto shaderSize = imgui_bin_size;
    // auto shaderBuffer =
    //     alloc<MemoryBuffer>(shaderSize, nvn::MemoryPoolFlags::CPU_UNCACHED | nvn::MemoryPoolFlags::GPU_CACHED |
    //                                         nvn::MemoryPoolFlags::SHADER_CODE);
    //
    // memcpy(shaderBuffer->memBuffer, static_bin, shaderSize);
    // // auto shader = CompiledData{.ptr = static_cast<u8*>(shaderBuffer->memBuffer), .size = shaderSize};
    // auto offsetData = BinaryHeader(reinterpret_cast<u32*>(shader));
    //
    // nvn::BufferAddress addr = shaderBuffer->GetBufferAddress();
    //
    // nvn::ShaderData shaders[2] = {};
    // nvn::ShaderData& vertShaderData = shaders[0];
    // vertShaderData.data = addr + offsetData.mVertexDataOffset;
    // vertShaderData.control = shader + offsetData.mVertexControlOffset;
    //
    // nvn::ShaderData& fragShaderData = shaders[1];
    // fragShaderData.data = addr + offsetData.mFragmentDataOffset;
    // fragShaderData.control = shader + offsetData.mFragmentControlOffset;
    //
    // EXL_ASSERT(program.Initialize(ImguiNvnBackend::getBackendData()->device), "unable to init program!");
    // EXL_ASSERT(program.SetShaders(2, shaders), "unable to set shaders!");
  }
  StaticManager::StaticManager(const al::ActorInitInfo& initInfo) {
    instance = this;
    // overScreen = alloc<al::LayoutActor>("StaticScreen");
    // al::initLayoutActor(overScreen, al::getLayoutInitInfo(initInfo), "PlayGuideMovie", nullptr);
    // al::startAction(overScreen, "Wait", nullptr);
    // noiseTextureKeeper = unsafeRef<al::NoiseTextureKeeper*>(al::getGraphicsSystemInfo(initInfo), 0x278);
    // Logger::log("static texture keeper: %s\n", getTypename(noiseTextureKeeper));
    // textureInfo = al::createTextureInfo(al::getBlack2DTexture());
    // noiseTextureKeeper->declareUsingSimpleNoiseTexture();
    // Logger::log("appearin %d\n", overScreen->mIsAlive);
    // // overScreen->appear();
    // auto sampler = nvn::SamplerBuilder()
    //                    .SetDefaults()
    //                    .SetDevice(ImguiNvnBackend::getBackendData()->device)
    //                    .SetMinMagFilter(nvn::MinFilter::LINEAR, nvn::MagFilter::LINEAR)
    //                    .SetWrapMode(nvn::WrapMode::REPEAT, nvn::WrapMode::REPEAT, nvn::WrapMode::REPEAT);
    //
    // // moviePlayer = static_cast<MoviePlayer*>(al::createSceneObj(overScreen, 0x24));
    // this->sampler.Initialize(&sampler);
    // static auto self = this;
    // static int lol = 65535;
    // // moviePlayer->play("content:/MovieData/TvStatic.mp4");
    // // nvnImGui::addDrawFunc([] {
    // //   ImGui::SetNextWindowPos({});
    // //   ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    // //   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    // //   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    // //   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{});
    // //   ImGui::Begin("named window", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    // //
    // //   // auto texture = self->noiseTextureKeeper->simple->data->textureData;
    // //   auto texture = &self->moviePlayer->getTexture();
    // //   // auto* texture = &al::getBlack2DTexture();
    // //   // auto id = &unsafeRef<u32>(&texture->data->textureSampler->mTextureData._58, 0xc0);
    // //   // Logger::log("texture id: %u\n", texture->mTexture.mTextureID);
    // //   ImguiNvnBackend::getBackendData()->texPool.RegisterTexture(
    // //       lol, reinterpret_cast<const nvn::Texture*>(&texture->mTexture), nullptr);
    // //
    // //   // ImGui::Image(&texture->data->textureData->mTexture.mTextureID, ImGui::GetIO().DisplaySize);
    // //   ImGui::Image(&lol, ImGui::GetIO().DisplaySize);
    // //   ImGui::End();
    // //   ImGui::PopStyleVar(3);
    // // });
  }
  StaticManager::~StaticManager() { instance = nullptr; }
  void StaticManager::draw() {
    // auto& commandBuffer = *ImguiNvnBackend::getBackendData()->cmdBuf;
    // static nvn::CommandBuffer commandBuffer = nvn::CommandBuffer();
    // commandBuffer.BeginRecording();
    // agl::DrawContext drawContext = agl::DrawContext();
    // ImguiNvnBackend::setRenderStates();
    // ((nvn::CommandBuffer*)&drawContext.commandBuffer)
    // ->BindProgram(&program, nvn::ShaderStageBits::VERTEX | nvn::ShaderStageBits::FRAGMENT);
    // sead::ScopedCurrentHeapSetter setter(al::getSceneHeap() ?: al::getSequenceHeap());
    // al::FullScreenQuadModel quadModel;
    // quadModel.drawQuad(&drawContext);
    // auto handle = .EndRecording();
    // ImguiNvnBackend::getBackendData()->queue->SubmitCommands(0, &handle);
  }

  void StaticManager::update() const {
    // moviePlayer->update();
    // Logger::log("static texture keeper runtime: %s\n", getTypename(noiseTextureKeeper));
    // Logger::log("floats %.02f %.02f %.02f %.02f %.02f %.02f %.02f\n", texture->floats[0], texture->floats[1],
    //             texture->floats[2], texture->floats[3], texture->floats[4], texture->floats[5], texture->floats[6]);
    // Logger::log("Texture %p\n", texture->data);
    // Logger::log("Texture name max size %s\n", texture->data->name.cstr());
    // Logger::log("Texture %p %p\n", texture->data->textureSampler, texture->data->textureData);

    // auto& simple = *texture->data->textureSampler;

    // if (al::isPadHoldR(-1)) {
    // al::updateTextureInfo(textureInfo, simple);
    // al::setPaneTexture(overScreen, "PicMovie", textureInfo);
    // } else {
    // al::updateTextureInfo(textureInfo, al::getBlack2DTexture());
    // }
  }
} // namespace sp