#include "StaticManager.hpp"

#include "static_bin.h"
#include <account.h>
#include <agl/TextureSampler.h>
#include <agl/utl.h>
#include <al/Library/Controller/JoyPadUtil.h>
#include <al/Library/LiveActor/ActorModelFunction.h>
#include <al/Library/LiveActor/ActorMovementFunction.h>
#include <al/Library/Math/MathLengthUtil.hpp>
#include <game/System/Application.h>
#include <imgui.h>
#include <imgui_backend/imgui_impl_nvn.hpp>
#include <imgui_bin.h>
#include <imgui_nvn.h>
#include <init.h>
#include <logger/Logger.hpp>
#include <logger/Params.h>
#include <nvn/nvn_Cpp.h>
#include <utils/Helpers.h>

#define UBOSIZE 0x1000

// smoo headers decl

struct PuppetInfo {
  // General Puppet Info
  char puppetName[0x10] = {}; // max user account name size is 10 chars, so this could go down to 0xB
  bool isConnected = false;
  nn::account::Uid playerID;
  // Puppet Translation Info
  sead::Vector3f playerPos = sead::Vector3f(0.f,0.f,0.f);
  sead::Quatf playerRot = sead::Quatf(0.f,0.f,0.f,0.f);
  // Puppet Stage Info
  u8 scenarioNo = -1;
  char stageName[0x40] = {};
  bool isInSameStage = false;
  // Puppet Costume Info
  char costumeBody[0x20] = {};
  char costumeHead[0x20] = {};
  // Puppet Capture Info
  char curHack[0x40] = {};
  bool isCaptured = false;
  bool isStartCapture = false;
  // Puppet Model Info
  s16 curAnim;
  s16 curSubAnim;
  char curAnimStr[0x30] = {};
  char curSubAnimStr[0x30] = {};
  float blendWeights[6] = {};
  float animRate = 0.f;
  bool is2D = false;
  // Puppet Hack Cap Info
  sead::Vector3f capPos = sead::Vector3f(0.f,0.f,0.f);
  sead::Quatf capRot = sead::Quatf(0.f,0.f,0.f,0.f);
  char capAnim[0x30] = {};
  bool isCapThrow = false;
  bool isHoldThrow = false;
  // Hide and Seek Gamemode Info
  bool isIt = false;
  u8 seconds = 0;
  u16 minutes = 0;
};

class Client {
  public:

  static int (*getConnectCount)();

  static PuppetInfo* (*getPuppetInfo)(int idx);
  static PuppetActor* (*getPuppet)(int idx);

  static PuppetInfo* (*getDebugPuppetInfo)();
  static PuppetActor* (*getDebugPuppet)();

  static uintptr_t GetSMOOTargetOffset(uintptr_t offset) { return exl::util::GetModuleInfo(3).m_Total.m_Start + offset; }
};

int (*Client::getConnectCount)() = nullptr;

PuppetInfo* (*Client::getDebugPuppetInfo)() = nullptr;
PuppetActor* (*Client::getDebugPuppet)() = nullptr;

PuppetInfo* (*Client::getPuppetInfo)(int idx) = nullptr;
PuppetActor* (*Client::getPuppet)(int idx) = nullptr;

namespace sp {
  namespace {
    StaticManager* instance;

    constexpr int triVertCount = 3;
    constexpr int quadVertCount = triVertCount * 2;
    constexpr int quadCount = 1; // modify to reflect how many quads need to be drawn per frame

    constexpr int pointCount = quadVertCount * quadCount;

    void createQuad(ImDrawVert *verts, int startIndex, int x, int y, int width, int height) {
      float minXVal = x;
      float maxXVal = x + width;
      float minYVal = y; // 400
      float maxYVal = y + height; // 400

      // top left
      ImDrawVert p1 = {
          .pos = ImVec2(minXVal, minYVal),
          .uv = ImVec2(0.0f, 0.0f)
      };
      // top right
      ImDrawVert p2 = {
          .pos = ImVec2(minXVal, maxYVal),
          .uv = ImVec2(0.0f, 1.0f)
      };
      // bottom left
      ImDrawVert p3 = {
          .pos = ImVec2(maxXVal, minYVal),
          .uv = ImVec2(1.0f, 0.0f)
      };
      // bottom right
      ImDrawVert p4 = {
          .pos = ImVec2(maxXVal, maxYVal),
          .uv = ImVec2(1.0f, 1.0f)
      };

      verts[startIndex] = p4;
      verts[startIndex + 1] = p2;
      verts[startIndex + 2] = p1;

      verts[startIndex + 3] = p1;
      verts[startIndex + 4] = p3;
      verts[startIndex + 5] = p4;
    }

    PuppetActor* findSmooPuppet(const char* name) {
      for (int i = 0; i < Client::getConnectCount(); ++i) {
        auto curPuppet = Client::getPuppetInfo(i);
        if(curPuppet && al::isEqualString(curPuppet->puppetName, name))
          return Client::getPuppet(i);
      }
      return nullptr;
    }
  } // namespace
  void StaticManager::init() {
    // verify we've loaded smoo first
    const char* smooFirString = (const char*)Client::GetSMOOTargetOffset(0x136C0);
    EXL_ASSERT(al::isEqualString(smooFirString, "Fir"), "Failed to load SMOO func offsets!");

    // setup func ptrs for smoo stuff
    Client::getConnectCount = reinterpret_cast<int (*)(void)>(Client::GetSMOOTargetOffset(0xB620));

    Client::getDebugPuppetInfo = reinterpret_cast<PuppetInfo* (*)(void)>(Client::GetSMOOTargetOffset(0xAF20));
    Client::getDebugPuppet = reinterpret_cast<PuppetActor* (*)(void)>(Client::GetSMOOTargetOffset(0xAF40));

    Client::getPuppetInfo = reinterpret_cast<PuppetInfo* (*)(int)>(Client::GetSMOOTargetOffset(0xAE00));
    Client::getPuppet = reinterpret_cast<PuppetActor* (*)(int)>(Client::GetSMOOTargetOffset(0xADC0));

    auto bd = ImguiNvnBackend::getBackendData();
    instance->cmdBuf = bd->cmdBuf;
    instance->queue = bd->queue;

    ImguiNvnBackend::orthoRH_ZO(instance->ubo.projMtx, 0.0f, 1600.0f, 900.0f, 0.0f, -1.0f, 1.0f);

    instance->shaderBinary.size = static_bin_size;
    instance->shaderBinary.ptr = (u8*)malloc(static_bin_size);
    memcpy(instance->shaderBinary.ptr, static_bin, static_bin_size);

    instance->shaderBuffer = IM_NEW(MemoryBuffer)(instance->shaderBinary.size, instance->shaderBinary.ptr,
                                            nvn::MemoryPoolFlags::CPU_UNCACHED |
                                                nvn::MemoryPoolFlags::GPU_CACHED |
                                                nvn::MemoryPoolFlags::SHADER_CODE);

    EXL_ASSERT(instance->shaderBuffer->IsBufferReady(), "Shader Buffer was not ready! unable to continue. Shader Size: %d", static_bin_size);

    auto offsetData = BinaryHeader((u32 *) instance->shaderBinary.ptr);
    nvn::BufferAddress addr = instance->shaderBuffer->GetBufferAddress();
    nvn::ShaderData &vertShaderData = instance->shaderDatas[0];

    vertShaderData.data = addr + offsetData.mVertexDataOffset;
    vertShaderData.control = instance->shaderBinary.ptr + offsetData.mVertexControlOffset;

    nvn::ShaderData &fragShaderData = instance->shaderDatas[1];
    fragShaderData.data = addr + offsetData.mFragmentDataOffset;
    fragShaderData.control = instance->shaderBinary.ptr + offsetData.mFragmentControlOffset;

    EXL_ASSERT(instance->shaderProgram.Initialize(bd->device), "Unable to Init Program!");
    EXL_ASSERT(instance->shaderProgram.SetShaders(2, instance->shaderDatas), "Unable to Set Shaders!");

    instance->attribStates[0].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(ImDrawVert, pos)); // pos
    instance->attribStates[1].SetDefaults().SetFormat(nvn::Format::RG32F, offsetof(ImDrawVert, uv)); // uv
    instance->attribStates[2].SetDefaults().SetFormat(nvn::Format::RGBA8, offsetof(ImDrawVert, col)); // color

    instance->streamState.SetDefaults().SetStride(sizeof(ImDrawVert));

    instance->uniformBuffer = IM_NEW(MemoryBuffer)(UBOSIZE);

    size_t totalVtxSize = pointCount * sizeof(ImDrawVert);

    instance->vtxBuffer = IM_NEW(MemoryBuffer)(totalVtxSize);

    ImDrawVert *verts = (ImDrawVert *) instance->vtxBuffer->GetMemPtr();
    createQuad(verts, 0, 0, 0, 1600.0f, 900.0f);
  }

  StaticManager::StaticManager() { instance = this; }

  StaticManager::~StaticManager() { instance = nullptr; }

  void StaticManager::draw() {
    if(!instance->enableDraw) {
      instance->ubo.alphaValue = 0.0f;
      return;
    }

    instance->cmdBuf->BeginRecording();
    instance->cmdBuf->BindProgram(&instance->shaderProgram, nvn::ShaderStageBits::VERTEX | nvn::ShaderStageBits::FRAGMENT);
    instance->cmdBuf->BindUniformBuffer(nvn::ShaderStage::VERTEX, 0, *instance->uniformBuffer, UBOSIZE);
    instance->cmdBuf->UpdateUniformBuffer(*instance->uniformBuffer, UBOSIZE, 0, sizeof(instance->ubo), &instance->ubo);
    instance->cmdBuf->BindVertexBuffer(0, (*instance->vtxBuffer), instance->vtxBuffer->GetPoolSize());

    instance->setRenderStates();

    instance->cmdBuf->DrawArrays(nvn::DrawPrimitive::TRIANGLES, 0, pointCount);

    auto handle = instance->cmdBuf->EndRecording();
    instance->queue->SubmitCommands(1, &handle);

    instance->enableDraw = false;
  }

  void StaticManager::update(PlayerActorHakoniwa* player) {
    if(!player)
      return;

    // only attempt a draw while update is being run
    instance->enableDraw = true;

    // weird and jank af
    instance->ubo.time += 0.0001f;
    if (instance->ubo.time >= 1.3f)
      instance->ubo.time -= 0.3f;

    PuppetActor* spookyPuppet = findSmooPuppet("Fir");

    if(!spookyPuppet) {
      spookyPuppet = Client::getDebugPuppet();
      if(!spookyPuppet)
        return;
    }

    float actorDist = al::calcDistance(player, spookyPuppet);

    float minEffectDist = par::get("StaticEffectStartDist", 2000.0f);
    if(actorDist < minEffectDist)
      instance->ubo.alphaValue = 1.0f - al::normalize(actorDist, 0.0f, minEffectDist);
    else
      instance->ubo.alphaValue = 0.0f;
  }

  void StaticManager::setRenderStates() {
    nvn::PolygonState polyState;
    polyState.SetDefaults();
    polyState.SetPolygonMode(nvn::PolygonMode::FILL);
    polyState.SetCullFace(nvn::Face::NONE);
    polyState.SetFrontFace(nvn::FrontFace::CCW);
    cmdBuf->BindPolygonState(&polyState);

    nvn::ColorState colorState;
    colorState.SetDefaults();
    colorState.SetLogicOp(nvn::LogicOp::COPY);
    colorState.SetAlphaTest(nvn::AlphaFunc::ALWAYS);
    for (int i = 0; i < 8; ++i) {
      colorState.SetBlendEnable(i, true);
    }
    cmdBuf->BindColorState(&colorState);

    nvn::BlendState blendState;
    blendState.SetDefaults();
    blendState.SetBlendFunc(nvn::BlendFunc::SRC_ALPHA, nvn::BlendFunc::ONE_MINUS_SRC_ALPHA, nvn::BlendFunc::ONE,
                            nvn::BlendFunc::ZERO);
    blendState.SetBlendEquation(nvn::BlendEquation::ADD, nvn::BlendEquation::ADD);
    cmdBuf->BindBlendState(&blendState);

    cmdBuf->BindVertexAttribState(3, attribStates);
    cmdBuf->BindVertexStreamState(1, &streamState);
  }

} // namespace sp