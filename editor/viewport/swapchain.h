//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#pragma once

#include "third_party/metal-cpp/QuartzCore/QuartzCore.hpp"
#include "third_party/metal-cpp/Metal/Metal.hpp"
#include <string_view>

extern "C" CA::MetalLayer *metal_backend_create_layer(
    MTL::Device *device, uint64_t window_handle,
    uint32_t width, uint32_t height,
    bool hdr, bool vsync,
    uint32_t back_buffer_count) noexcept;

namespace vox {
class Swapchain {
public:
    Swapchain(MTL::Device *device, uint64_t window_handle,
              uint width, uint height, bool allow_hdr = false,
              bool vsync = true, uint back_buffer_size = 1) noexcept;

    ~Swapchain() noexcept;

    [[nodiscard]] auto layer() const noexcept { return _layer; }

    MTL::Drawable *nextDrawable();

    void present(MTL::Drawable *drawable, MTL::CommandBuffer *commandBuffer, MTL::Texture *image) noexcept;

    void set_name(std::string_view name) noexcept;

    void resize(int width, int height);


private:
    void create_pso(MTL::Device *device);

    static void applyStyle();

private:
    CA::MetalLayer *_layer{};
    MTL::RenderPipelineState *_pipeline{};
    MTL::RenderPassDescriptor *_render_pass_desc{};
    NS::String *_command_label{};
    MTL::PixelFormat _format;
};

}// namespace vox