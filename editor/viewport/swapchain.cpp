//  Copyright (c) 2024 Feng Yang
//
//  I am making my contributions/submissions to this project solely in my
//  personal capacity and am not conveying any rights to any intellectual
//  property of any third parties.

#include "swapchain.h"
#include <fmt/format.h>
#include <QDebug>
#include <backends/imgui_impl_metal.h>

namespace vox {
Swapchain::Swapchain(MTL::Device *device, uint64_t window_handle,
                     uint width, uint height, bool allow_hdr,
                     bool vsync, uint back_buffer_size) noexcept
    : _layer{metal_backend_create_layer(
          device, window_handle,
          width, height, allow_hdr,
          vsync, back_buffer_size)},
      _render_pass_desc{MTL::RenderPassDescriptor::alloc()->init()} {
    _layer->retain();
    auto attachment_desc = _render_pass_desc->colorAttachments()->object(0);
    attachment_desc->setLoadAction(MTL::LoadActionClear);
    attachment_desc->setStoreAction(MTL::StoreActionStore);
    attachment_desc->setClearColor(MTL::ClearColor(1.0, 1.0, 1.0, 1.0));
    _format = allow_hdr ? MTL::PixelFormatRGBA16Float : MTL::PixelFormatBGRA8Unorm;

    create_pso(device);

    // load imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    applyStyle();
    io.FontDefault = io.Fonts->AddFontFromFileTTF(fmt::format("{}/{}", PROJECT_PATH, "editor/fonts/Roboto-Regular.ttf").c_str(), 50);
    io.FontGlobalScale = 0.3;

    // Global scale
    ImGui_ImplMetal_Init(device);
}

void Swapchain::applyStyle() {
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 4.0f;
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.GrabRounding = 3.0f;

    style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.2f, 0.88f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.3f, 0.3f, 0.3f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

    style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_TabUnfocused];
}

void Swapchain::resize(int width, int height) {
    _layer->setDrawableSize(CGSizeMake(width, height));
}

Swapchain::~Swapchain() noexcept {
    if (_command_label) { _command_label->release(); }
    _render_pass_desc->release();
    _pipeline->release();
    _layer->release();
    ImGui_ImplMetal_Shutdown();
}

void Swapchain::create_pso(MTL::Device *device) {
    auto raw_source =
        "#include <metal_stdlib>\n"
        "using namespace metal;\n"
        "\n"
        "struct VertexOut\n"
        "{\n"
        "    float4 position [[ position ]];\n"
        "    float2 texcoord;\n"
        "};\n"
        "\n"
        "vertex VertexOut vtxBlit(uint vid [[vertex_id]])\n"
        "{\n"
        "    // These vertices map a triangle to cover a fullscreen quad.\n"
        "    const float4 vertices[] = {\n"
        "        float4(-1, -1, 1, 1), // bottom left\n"
        "        float4(3, -1, 1, 1),  // bottom right\n"
        "        float4(-1, 3, 1, 1),  // upper left\n"
        "    };\n"
        "    \n"
        "    const float2 texcoords[] = {\n"
        "        float2(0.0, 0.0), // bottom left\n"
        "        float2(2.0, 0.0), // bottom right\n"
        "        float2(0.0, 2.0), // upper left\n"
        "    };\n"
        "    \n"
        "    VertexOut out;\n"
        "    out.position = vertices[vid];\n"
        "    out.texcoord = texcoords[vid];\n"
        "    return out;\n"
        "}\n"
        "\n"
        "fragment half4 fragBlitLinear(VertexOut in [[stage_in]], texture2d<float> tex[[texture(0)]])\n"
        "{\n"
        "    constexpr sampler s = sampler(address::clamp_to_edge);\n"
        "    \n"
        "    float4 pixel = tex.sample(s, in.texcoord);\n"
        "    return half4(pixel);\n"
        "}";

    auto source = NS::String::string(raw_source, NS::UTF8StringEncoding);
    NS::Error *error{nullptr};
    auto option = MTL::CompileOptions::alloc()->init();
    auto defaultLibrary = device->newLibrary(source, option, &error);
    if (error != nullptr) {
        qDebug("Error: could not load Metal shader library: %s",
               error->description()->cString(NS::StringEncoding::UTF8StringEncoding));
    }

    auto functionName = NS::String::string("vtxBlit", NS::UTF8StringEncoding);
    auto vertexFunction = defaultLibrary->newFunction(functionName);
    functionName = NS::String::string("fragBlitLinear", NS::UTF8StringEncoding);
    auto fragmentFunction = defaultLibrary->newFunction(functionName);

    // Set up the pipeline state object.
    auto pipelineStateDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pipelineStateDescriptor->setRasterSampleCount(1);
    pipelineStateDescriptor->setVertexFunction(vertexFunction);
    pipelineStateDescriptor->setFragmentFunction(fragmentFunction);
    pipelineStateDescriptor->setDepthAttachmentPixelFormat(MTL::PixelFormatInvalid);

    // Configure the color attachment for blending.
    MTL::RenderPipelineColorAttachmentDescriptor *colorDescriptor = pipelineStateDescriptor->colorAttachments()->object(0);
    colorDescriptor->setPixelFormat(_format);
    colorDescriptor->setBlendingEnabled(true);
    colorDescriptor->setRgbBlendOperation(MTL::BlendOperationAdd);
    colorDescriptor->setAlphaBlendOperation(MTL::BlendOperationAdd);
    colorDescriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
    colorDescriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    colorDescriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    colorDescriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);

    error = nullptr;
    _pipeline = device->newRenderPipelineState(pipelineStateDescriptor, &error);
    if (!_pipeline) {
        qDebug("Failed to created pipeline state, error %s", error->description()->cString(NS::StringEncoding::UTF8StringEncoding));
    }
}

MTL::Drawable *Swapchain::nextDrawable() {
    if (auto drawable = _layer->nextDrawable()) {
        auto attachment_desc = _render_pass_desc->colorAttachments()->object(0);
        attachment_desc->setTexture(drawable->texture());

        ImGui_ImplMetal_NewFrame(_render_pass_desc);
        ImGui::NewFrame();
        return drawable;
    } else {
        qDebug("Failed to acquire next drawable from swapchain.");
        return nullptr;
    }
}

void Swapchain::present(MTL::Drawable *drawable, MTL::CommandBuffer *command_buffer, MTL::Texture *image) noexcept {
    auto command_encoder = command_buffer->renderCommandEncoder(_render_pass_desc);
    // Blit the texture to the view.
    command_encoder->pushDebugGroup(NS::String::string("FinalBlit", NS::UTF8StringEncoding));
    command_encoder->setFragmentTexture(image, 0);
    command_encoder->setRenderPipelineState(_pipeline);
    command_encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(3));
    command_encoder->popDebugGroup();

    ImGui::Render();
    ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), command_buffer, command_encoder);

    command_encoder->endEncoding();
    command_buffer->presentDrawable(drawable);
    if (_command_label) { command_buffer->setLabel(_command_label); }
}

void Swapchain::set_name(std::string_view name) noexcept {
    if (_command_label) {
        _command_label->release();
        _command_label = nullptr;
    }
    if (!name.empty()) {
        auto label = fmt::format("{}::present", name);
        _command_label = NS::String::alloc()->init(
            label.data(), label.size(),
            NS::UTF8StringEncoding, false);
    }
}

}// namespace vox