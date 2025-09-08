#pragma once

#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <atomic>
#include <string>

#include "camera.hpp"
#include "controls.hpp"
#include "settings.hpp"

namespace runtime {

using mesh_t = struct mesh {
    bgfx::VertexBufferHandle vertexBufferHandle = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle indexBufferHandle = BGFX_INVALID_HANDLE;
    bgfx::TextureHandle textureHandle = BGFX_INVALID_HANDLE;
    uint32_t indexCount{};
    uint32_t vertexCount{};
};

using event_t = SDL_Event;

using applicationState_t = struct applicationState {
    applicationState() = default;
    applicationState( const applicationState& ) = delete;
    applicationState( applicationState&& ) = delete;
    ~applicationState() = default;
    auto operator=( const applicationState& ) -> applicationState& = delete;
    auto operator=( applicationState&& ) -> applicationState& = delete;

    auto load() -> bool;
    auto unload() -> bool;

    SDL_Window* window = nullptr;
    size_t logicalWidth = 1280;
    size_t logicalHeight = 720;
    float width = logicalWidth;
    float height = logicalHeight;
    std::atomic< size_t > totalFramesRendered = 0;

    camera::camera_t camera;
    settings::settings_t settings;
    controls::input_t currentInput;

    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::string modelPath;

    bgfx::ProgramHandle shaderProgramHandle = BGFX_INVALID_HANDLE;
    mesh_t mesh;
    bgfx::UniformHandle textureColorSampler = BGFX_INVALID_HANDLE;

    bool status = false;
};

auto init( applicationState_t& _applicationState ) -> bool;
void quit( applicationState_t& _applicationState );

auto event( applicationState_t& _applicationState, const event_t& _event )
    -> bool;
auto iterate( applicationState_t& _applicationState ) -> bool;

} // namespace runtime
