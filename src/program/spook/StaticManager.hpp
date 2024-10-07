#pragma once
#include <game/Player/PlayerActorHakoniwa.h>
#include <imgui_backend/ImguiShaderCompiler.h>
#include <imgui_backend/MemoryBuffer.h>
#include <imgui_backend/imgui_impl_nvn.hpp>
#include <utils/ForwardDecls.hpp>

namespace sp {

  class StaticManager {
    nvn::Program shaderProgram;
    nvn::ShaderData shaderDatas[2]; // 0 - Vert 1 - Frag
    nvn::VertexAttribState attribStates[3]; // pos, uv, color
    nvn::VertexStreamState streamState;
    nvn::CommandBuffer *cmdBuf = nullptr;
    nvn::Queue *queue = nullptr;

    MemoryBuffer *shaderBuffer = nullptr;
    MemoryBuffer *vtxBuffer = nullptr;
    MemoryBuffer *uniformBuffer = nullptr;

    CompiledData shaderBinary;

    struct {
      ImguiNvnBackend::Matrix44f projMtx = {};
      float time = 1.0f;
      float alphaValue = 0.0f;
    } ubo;

    bool enableDraw = false;

    void setRenderStates();
public:
    static void init();
    static void draw();
    static void update(PlayerActorHakoniwa* player);

    StaticManager();
    ~StaticManager();
  };

} // namespace sp
