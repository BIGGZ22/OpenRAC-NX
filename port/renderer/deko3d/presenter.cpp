// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer/deko3d/presenter.h"
#include "renderer/texture.h"
#include "renderer/deko3d/scene_renderer.h"
#include <unordered_map>
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstddef>
#include <limits>
#include <switch.h>
#include <deko3d.h>

namespace openrac::deko {
namespace {
constexpr unsigned kBuffers = 2, kCommandBytes = 16 * 1024;
constexpr std::uint64_t align_up(std::uint64_t size, std::uint64_t alignment) {
    return (size + alignment - 1) / alignment * alignment;
}
void diagnostic(void*, const char* context, DkResult result, const char* message) {
    // The callback can run on a driver thread. Do not touch renderer ownership.
    if (FILE* file = std::fopen("sdmc:/switch/openrac/deko3d-driver.log", "a")) {
        std::fprintf(file,"%s: result %u: %s\n",context ? context : "deko3d",
            static_cast<unsigned>(result),message ? message : "");
        std::fclose(file);
    }
}
bool load_program(DkDevice device, DkMemBlock& code, DkShader& vs, DkShader& fs,
                  std::span<const std::uint8_t> vertex, std::span<const std::uint8_t> fragment, std::string& error) {
    constexpr unsigned kCodeBytes = 64*1024;
    const auto fragment_offset = align_up(vertex.size(),DK_SHADER_CODE_ALIGNMENT);
    if (vertex.size() < 16 || fragment.size() < 16 || vertex.size() > kCodeBytes || fragment.size() > kCodeBytes
        || fragment_offset + fragment.size() > kCodeBytes-DK_SHADER_CODE_UNUSABLE_SIZE
        || std::memcmp(vertex.data(),"DKSH",4) != 0 || std::memcmp(fragment.data(),"DKSH",4) != 0) {
        error = "Deko3D shaders are missing, oversized or not DKSH files."; return false;
    }
    DkMemBlockMaker maker;
    dkMemBlockMakerDefaults(&maker,device,kCodeBytes);
    maker.flags = DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached | DkMemBlockFlags_Code;
    code = dkMemBlockCreate(&maker);
    if (!code) { error = "Deko3D shader memory allocation failed."; return false; }
    auto* bytes = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(code));
    std::memcpy(bytes,vertex.data(),vertex.size()); std::memcpy(bytes+fragment_offset,fragment.data(),fragment.size());
    DkShaderMaker shader;
    dkShaderMakerDefaults(&shader,code,0); dkShaderInitialize(&vs,&shader);
    dkShaderMakerDefaults(&shader,code,unsigned(fragment_offset)); dkShaderInitialize(&fs,&shader);
    if (!dkShaderIsValid(&vs) || !dkShaderIsValid(&fs)
        || dkShaderGetStage(&vs) != DkStage_Vertex || dkShaderGetStage(&fs) != DkStage_Fragment) {
        error = "Deko3D shaders have invalid programs or stages."; return false;
    }
    return true;
}
}
struct Presenter::Impl {
    struct Slot {
        DkMemBlock upload = nullptr, commands = nullptr, transform = nullptr;
        DkCmdBuf command_buffer = nullptr;
        DkCmdList list = 0;
        std::uint64_t key = 0;
        DkFence finished{};
        bool pending = false;
    };
    struct Texture {
        DkImage image{};
        DkMemBlock memory = nullptr, descriptors = nullptr;
        ~Texture() {
            if (descriptors) dkMemBlockDestroy(descriptors);
            if (memory) dkMemBlockDestroy(memory);
        }
    };
    std::unique_ptr<Texture> texture;
    DkDevice device = nullptr;
    DkQueue queue = nullptr;
    DkSwapchain swapchain = nullptr;
    DkMemBlock images_memory = nullptr, readback = nullptr, code = nullptr;
    DkMemBlock depth_memory = nullptr, mesh_code = nullptr, geometry = nullptr;
    DkShader vertex{}, fragment{};
    DkShader mesh_vertex{}, mesh_fragment{};
    unsigned mesh_index_offset = 0, mesh_vertex_bytes = 0, mesh_indices = 0;
    std::uint32_t geometry_generation = 0;
    bool mesh_ready = false;
    bool triangle_ready = false;
    std::array<DkImage,kBuffers> images{};
    std::array<DkImage,kBuffers> depth_images{};
    std::array<Slot,kBuffers> slots{};
    unsigned width = 0, height = 0, upload_width = 0;
    std::uint64_t frame_count = 0;
    ~Impl() {
        // Every partial-open failure takes this same cleanup path.
        if (queue) { dkQueueWaitIdle(queue); dkQueueDestroy(queue); }
        for (auto& slot : slots) {
            if (slot.command_buffer) dkCmdBufDestroy(slot.command_buffer);
            if (slot.commands) dkMemBlockDestroy(slot.commands);
            if (slot.upload) dkMemBlockDestroy(slot.upload);
            if (slot.transform) dkMemBlockDestroy(slot.transform);
        }
        if (swapchain) dkSwapchainDestroy(swapchain);
        if (readback) dkMemBlockDestroy(readback);
        if (code) dkMemBlockDestroy(code);
        if (mesh_code) dkMemBlockDestroy(mesh_code);
        if (geometry) dkMemBlockDestroy(geometry);
        if (depth_memory) dkMemBlockDestroy(depth_memory);
        if (images_memory) dkMemBlockDestroy(images_memory);
        texture.reset();
        if (device) dkDeviceDestroy(device);
    }
    DkMemBlock memory(unsigned bytes, unsigned flags) {
        DkMemBlockMaker maker;
        dkMemBlockMakerDefaults(&maker,device,bytes); maker.flags = flags;
        return dkMemBlockCreate(&maker);
    }
    bool wait(Slot& slot, std::string& error) {
        if (!slot.pending) return true;
        const DkResult result = dkFenceWait(&slot.finished,1'000'000'000);
        if (result != DkResult_Success) {
            error = "Deko3D frame fence failed or timed out (" + std::to_string(unsigned(result)) + ").";
            return false;
        }
        slot.pending = false; return true;
    }
    int acquire(std::string& error) {
        if (!queue || dkQueueIsInErrorState(queue)) { error = "Deko3D graphics queue is unavailable."; return -1; }
        const int slot = dkQueueAcquireImage(queue,swapchain);
        if (slot < 0 || slot >= int(kBuffers)) { error = "Deko3D could not acquire a display image."; return -1; }
        if (!wait(slots[slot],error)) return -1;
        return slot;
    }
    // A list references retained slot addresses, not their current CPU contents.
    // Reuse it only after the slot fence; geometry replacement changes its key.
    bool prepare(unsigned index, std::uint64_t key, std::string& error) {
        auto& slot = slots[index];
        if (slot.list && slot.key == key) return true;
        slot.list = 0;
        if (slot.command_buffer) dkCmdBufDestroy(slot.command_buffer);
        DkCmdBufMaker maker; dkCmdBufMakerDefaults(&maker,device);
        slot.command_buffer = dkCmdBufCreate(&maker);
        if (!slot.command_buffer) { error = "Deko3D command-buffer allocation failed."; return false; }
        dkCmdBufAddMemory(slot.command_buffer,slot.commands,0,kCommandBytes);
        slot.key = key;
        return true;
    }
    bool submit(unsigned index, std::string& error, bool verify) {
        auto& slot = slots[index];
        if (!slot.list) slot.list = dkCmdBufFinishList(slot.command_buffer);
        const DkCmdList list = slot.list;
        if (!list) { error = "Deko3D command recording failed."; return false; }
        dkQueueSubmitCommands(queue,list);
        dkQueueSignalFence(queue,&slot.finished,true);
        // Publish queued work before an optional CPU readback fence wait.
        dkQueueFlush(queue);
        slot.pending = true;
        if (verify && !wait(slot,error)) return false;
        dkQueuePresentImage(queue,swapchain,int(index));
        if (dkQueueIsInErrorState(queue)) { error = "Deko3D queue reported an error during presentation."; return false; }
        ++frame_count; return true;
    }
};
Presenter::Presenter() = default;
Presenter::~Presenter() = default;
bool Presenter::open(int width, int height, std::string& error) {
    if (p) { error = "Deko3D presenter is already open."; return false; }
    if (width < 640 || height < 480 || width > 1920 || height > 1080) {
        error = "Deko3D display dimensions must be between 640x480 and 1920x1080."; return false;
    }
    auto next = std::make_unique<Impl>();
    next->width = unsigned(width); next->height = unsigned(height);
    next->upload_width = unsigned(align_up(std::uint64_t(width)*4,DK_IMAGE_LINEAR_STRIDE_ALIGNMENT)/4);
    if (R_FAILED(nwindowSetDimensions(nwindowGetDefault(),next->width,next->height))) {
        error = "Could not size the native Deko3D display."; return false;
    }
    DkDeviceMaker device;
    dkDeviceMakerDefaults(&device); device.cbDebug = diagnostic;
    next->device = dkDeviceCreate(&device);
    if (!next->device) { error = "Deko3D device creation failed."; return false; }
    DkQueueMaker queue;
    dkQueueMakerDefaults(&queue,next->device); queue.flags = DkQueueFlags_Graphics;
    next->queue = dkQueueCreate(&queue);
    if (!next->queue) { error = "Deko3D queue creation failed."; return false; }
    DkImageLayoutMaker maker;
    dkImageLayoutMakerDefaults(&maker,next->device);
    maker.flags = DkImageFlags_UsageRender | DkImageFlags_UsagePresent | DkImageFlags_Usage2DEngine;
    maker.format = DkImageFormat_RGBA8_Unorm;
    maker.dimensions[0] = next->width; maker.dimensions[1] = next->height;
    DkImageLayout layout;
    dkImageLayoutInitialize(&layout,&maker);
    const auto alignment = dkImageLayoutGetAlignment(&layout);
    const auto size = dkImageLayoutGetSize(&layout);
    if (!alignment || !size) { error = "Deko3D display image layout is invalid."; return false; }
    const auto stride = align_up(size,alignment);
    if (stride * kBuffers > std::numeric_limits<unsigned>::max()) { error = "Deko3D image allocation is too large."; return false; }
    const auto image_bytes = align_up(stride*kBuffers,DK_MEMBLOCK_ALIGNMENT);
    next->images_memory = next->memory(unsigned(image_bytes),DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image);
    if (!next->images_memory) { error = "Deko3D display memory allocation failed."; return false; }
    std::array<const DkImage*,kBuffers> images;
    for (unsigned i = 0; i < kBuffers; ++i) {
        dkImageInitialize(&next->images[i],&layout,next->images_memory,unsigned(i*stride));
        images[i] = &next->images[i];
    }
    maker.flags = DkImageFlags_UsageRender; maker.format = DkImageFormat_Z24S8;
    DkImageLayout depth_layout; dkImageLayoutInitialize(&depth_layout,&maker);
    const auto depth_alignment = dkImageLayoutGetAlignment(&depth_layout);
    if (!depth_alignment) { error = "Deko3D depth layout is invalid."; return false; }
    const auto depth_stride = align_up(dkImageLayoutGetSize(&depth_layout),depth_alignment);
    next->depth_memory = next->memory(unsigned(align_up(depth_stride*kBuffers,DK_MEMBLOCK_ALIGNMENT)),DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image);
    if (!next->depth_memory) { error = "Deko3D depth memory allocation failed."; return false; }
    for (unsigned i = 0; i < kBuffers; ++i)
        dkImageInitialize(&next->depth_images[i],&depth_layout,next->depth_memory,unsigned(i*depth_stride));
    DkSwapchainMaker swap;
    dkSwapchainMakerDefaults(&swap,next->device,nwindowGetDefault(),images.data(),kBuffers);
    next->swapchain = dkSwapchainCreate(&swap);
    if (!next->swapchain) { error = "Deko3D swapchain creation failed."; return false; }
    dkSwapchainSetSwapInterval(next->swapchain,1);
    const auto upload_bytes = unsigned(align_up(std::uint64_t(next->upload_width)*height*4,DK_MEMBLOCK_ALIGNMENT));
    for (auto& slot : next->slots) {
        slot.upload = next->memory(upload_bytes,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
        slot.commands = next->memory(kCommandBytes,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
        slot.transform = next->memory(DK_MEMBLOCK_ALIGNMENT,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
        DkCmdBufMaker commands;
        dkCmdBufMakerDefaults(&commands,next->device);
        slot.command_buffer = dkCmdBufCreate(&commands);
        if (!slot.upload || !slot.commands || !slot.transform || !slot.command_buffer) { error = "Deko3D frame-slot allocation failed."; return false; }
    }
    next->readback = next->memory(DK_MEMBLOCK_ALIGNMENT,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    if (!next->readback) { error = "Deko3D readback allocation failed."; return false; }
    p = std::move(next); return true;
}
bool Presenter::clear(std::uint32_t color, std::string& error) {
    if (!p) { error = "Deko3D presenter is not open."; return false; }
    const int index = p->acquire(error); if (index < 0) return false;
    if (!p->prepare(unsigned(index),(std::uint64_t(color)<<8)|1,error)) return false;
    if (!p->slots[index].list) {
    auto cmd = p->slots[index].command_buffer;
    DkImageView view; dkImageViewDefaults(&view,&p->images[index]);
    const DkScissor scissor{0,0,p->width,p->height};
    dkCmdBufBindRenderTarget(cmd,&view,nullptr);
    dkCmdBufSetScissors(cmd,0,&scissor,1);
    dkCmdBufClearColorFloat(cmd,0,DkColorMask_RGBA,float(color & 255)/255,
        float((color>>8)&255)/255,float((color>>16)&255)/255,float(color>>24)/255);
    }
    return p->submit(unsigned(index),error,false);
}
bool Presenter::present(std::span<const std::uint32_t> pixels, std::string& error, bool verify) {
    if (!p) { error = "Deko3D presenter is not open."; return false; }
    if (pixels.size() != std::size_t(p->width)*p->height) { error = "Deko3D upload dimensions do not match the display."; return false; }
    const int index = p->acquire(error); if (index < 0) return false;
    if (!p->prepare(unsigned(index),2|(std::uint64_t(verify)<<4),error)) return false;
    auto& slot = p->slots[index];
    auto* upload = static_cast<std::uint32_t*>(dkMemBlockGetCpuAddr(slot.upload));
    if (p->upload_width == p->width) std::memcpy(upload,pixels.data(),pixels.size_bytes());
    else for (unsigned y = 0; y < p->height; ++y)
        std::memcpy(upload+std::size_t(y)*p->upload_width,pixels.data()+std::size_t(y)*p->width,p->width*4);
    if (!slot.list) {
    DkImageView view; dkImageViewDefaults(&view,&p->images[index]);
    // DkCopyBuf names are misleading: rowLength is a byte pitch and
    // imageHeight is a byte layer stride (not pixels/rows). See dk_image.cpp.
    const unsigned row_bytes = p->upload_width * sizeof(std::uint32_t);
    const DkCopyBuf source{dkMemBlockGetGpuAddr(slot.upload),row_bytes,row_bytes*p->height};
    const DkImageRect rectangle{0,0,0,p->width,p->height,1};
    // CPU writes into a reused staging slot must replace GPU-cached contents.
    dkCmdBufBarrier(slot.command_buffer,DkBarrier_Full,DkInvalidateFlags_L2Cache);
    dkCmdBufCopyBufferToImage(slot.command_buffer,&source,&view,&rectangle,0);
    if (verify) {
        dkCmdBufBarrier(slot.command_buffer,DkBarrier_Full,DkInvalidateFlags_Image);
        const DkImageRect corner{0,0,0,2,2,1};
        const DkCopyBuf target{dkMemBlockGetGpuAddr(p->readback),DK_IMAGE_LINEAR_STRIDE_ALIGNMENT,2*DK_IMAGE_LINEAR_STRIDE_ALIGNMENT};
        dkCmdBufCopyImageToBuffer(slot.command_buffer,&view,&corner,&target,0);
    }
    }
    if (!p->submit(unsigned(index),error,verify)) return false;
    if (verify) {
        const auto* returned = static_cast<const std::uint32_t*>(dkMemBlockGetCpuAddr(p->readback));
        if (returned[0] != pixels[0] || returned[1] != pixels[1]
            || returned[DK_IMAGE_LINEAR_STRIDE_ALIGNMENT/4] != pixels[p->width]
            || returned[DK_IMAGE_LINEAR_STRIDE_ALIGNMENT/4+1] != pixels[p->width+1]) {
            error = "Deko3D GPU image round-trip mismatch (format, stride or copy ordering)."; return false;
        }
    }
    return true;
}
bool Presenter::load_triangle(std::span<const std::uint8_t> vertex, std::span<const std::uint8_t> fragment, std::string& error) {
    if (!p || p->code) { error = "Deko3D shader setup needs an open presenter and can run only once."; return false; }
    p->triangle_ready = load_program(p->device,p->code,p->vertex,p->fragment,vertex,fragment,error);
    return p->triangle_ready;
}
bool Presenter::triangle(std::string& error, bool verify) {
    if (!p || !p->triangle_ready) { error = "Deko3D triangle shaders are not loaded."; return false; }
    const int index = p->acquire(error); if (index < 0) return false;
    if (!p->prepare(unsigned(index),3|(std::uint64_t(verify)<<4),error)) return false;
    if (!p->slots[index].list) {
    const auto cmd = p->slots[index].command_buffer;
    DkImageView target; dkImageViewDefaults(&target,&p->images[index]);
    const DkViewport viewport{0,0,float(p->width),float(p->height),0,1};
    const DkScissor scissor{0,0,p->width,p->height};
    DkRasterizerState rasterizer; dkRasterizerStateDefaults(&rasterizer); rasterizer.cullMode = DkFace_None;
    DkColorState color; dkColorStateDefaults(&color);
    DkColorWriteState writes; dkColorWriteStateDefaults(&writes);
    DkDepthStencilState depth; dkDepthStencilStateDefaults(&depth);
    depth.depthTestEnable = false; depth.depthWriteEnable = false;
    const DkShader* shaders[] = {&p->vertex,&p->fragment};
    dkCmdBufBindRenderTarget(cmd,&target,nullptr);
    dkCmdBufSetViewports(cmd,0,&viewport,1); dkCmdBufSetScissors(cmd,0,&scissor,1);
    dkCmdBufClearColorFloat(cmd,0,DkColorMask_RGBA,0.05f,0.10f,0.30f,1.0f);
    dkCmdBufBindRasterizerState(cmd,&rasterizer); dkCmdBufBindColorState(cmd,&color);
    dkCmdBufBindColorWriteState(cmd,&writes); dkCmdBufBindDepthStencilState(cmd,&depth);
    dkCmdBufBindShaders(cmd,DkStageFlag_GraphicsMask,shaders,2);
    dkCmdBufDraw(cmd,DkPrimitive_Triangles,3,1,0,0);
    if (verify) {
        dkCmdBufBarrier(cmd,DkBarrier_Full,DkInvalidateFlags_Image);
        const DkImageRect center{p->width/2,p->height/2,0,1,1,1};
        const DkCopyBuf destination{dkMemBlockGetGpuAddr(p->readback),DK_IMAGE_LINEAR_STRIDE_ALIGNMENT,DK_IMAGE_LINEAR_STRIDE_ALIGNMENT};
        dkCmdBufCopyImageToBuffer(cmd,&target,&center,&destination,0);
    }
    }
    if (!p->submit(unsigned(index),error,verify)) return false;
    if (verify && *static_cast<const std::uint32_t*>(dkMemBlockGetCpuAddr(p->readback)) != 0xff0000ffu) {
        error = "Deko3D triangle center pixel did not render red."; return false;
    }
    return true;
}
bool Presenter::load_mesh_shaders(std::span<const std::uint8_t> vertex, std::span<const std::uint8_t> fragment, std::string& error) {
    if (!p || p->mesh_code) { error = "Deko3D mesh shaders require an open presenter and can load only once."; return false; }
    p->mesh_ready = load_program(p->device,p->mesh_code,p->mesh_vertex,p->mesh_fragment,vertex,fragment,error);
    return p->mesh_ready;
}
bool Presenter::upload_mesh(std::span<const MeshVertex> vertices, std::span<const std::uint32_t> indices, std::string& error) {
    if (!p || vertices.empty() || indices.empty() || indices.size()%3 || vertices.size()>1'000'000 || indices.size()>3'000'000) {
        error = "Deko3D mesh needs bounded vertices and complete indexed triangles."; return false;
    }
    for (const auto index : indices) if (index >= vertices.size()) { error = "Deko3D mesh index is out of bounds."; return false; }
    for (auto& slot : p->slots) if (!p->wait(slot,error)) return false;
    const auto index_offset = unsigned(align_up(vertices.size_bytes(),4));
    const auto bytes = unsigned(align_up(index_offset+indices.size_bytes(),DK_MEMBLOCK_ALIGNMENT));
    const auto geometry = p->memory(bytes,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    if (!geometry) { error = "Deko3D mesh memory allocation failed; old mesh retained."; return false; }
    auto* cpu = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(geometry));
    std::memcpy(cpu,vertices.data(),vertices.size_bytes()); std::memcpy(cpu+index_offset,indices.data(),indices.size_bytes());
    if (p->geometry) dkMemBlockDestroy(p->geometry);
    ++p->geometry_generation;
    // Explicit invalidation also covers generation counter wraparound.
    for (auto& slot : p->slots) slot.list = 0;
    p->geometry = geometry; p->mesh_index_offset = index_offset;
    p->mesh_vertex_bytes = unsigned(vertices.size_bytes()); p->mesh_indices = unsigned(indices.size());
    return true;
}
bool Presenter::upload_texture(const renderer::Rgba8Image& image, std::string& error,
                               bool linear_filter, bool repeat, int repeat_t) {
    if (!p || image.width <= 0 || image.height <= 0
        || image.width > renderer::kMaxTextureSide || image.height > renderer::kMaxTextureSide
        || image.pixels.size() != std::size_t(image.width)*image.height*4) {
        error = "Deko3D texture needs a complete RGBA8 image up to 1024x1024."; return false;
    }
    if (dkQueueIsInErrorState(p->queue)) { error = "Deko3D texture upload: graphics queue failed."; return false; }
    for (auto& slot : p->slots) if (!p->wait(slot,error)) return false;
    auto next = std::make_unique<Impl::Texture>();
    DkImageLayoutMaker maker; dkImageLayoutMakerDefaults(&maker,p->device);
    maker.format = DkImageFormat_RGBA8_Unorm;
    maker.dimensions[0] = unsigned(image.width); maker.dimensions[1] = unsigned(image.height);
    DkImageLayout layout; dkImageLayoutInitialize(&layout,&maker);
    const auto size = dkImageLayoutGetSize(&layout);
    if (!size) { error = "Deko3D texture layout failed."; return false; }
    next->memory = p->memory(unsigned(align_up(size,DK_MEMBLOCK_ALIGNMENT)),DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image);
    next->descriptors = p->memory(DK_MEMBLOCK_ALIGNMENT,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    if (!next->memory || !next->descriptors) { error = "Deko3D texture allocation failed; old texture retained."; return false; }
    dkImageInitialize(&next->image,&layout,next->memory,0);
    DkImageView view; dkImageViewDefaults(&view,&next->image);
    auto* descriptors = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(next->descriptors));
    dkImageDescriptorInitialize(reinterpret_cast<DkImageDescriptor*>(descriptors),&view,false,false);
    DkSampler sampler; dkSamplerDefaults(&sampler);
    sampler.minFilter = sampler.magFilter = linear_filter ? DkFilter_Linear : DkFilter_Nearest;
    sampler.wrapMode[0] = repeat ? DkWrapMode_Repeat : DkWrapMode_ClampToEdge;
    sampler.wrapMode[1] = (repeat_t<0 ? repeat : repeat_t!=0) ? DkWrapMode_Repeat : DkWrapMode_ClampToEdge;
    dkSamplerDescriptorInitialize(reinterpret_cast<DkSamplerDescriptor*>(descriptors+DK_MEMBLOCK_ALIGNMENT/2),&sampler);
    // Synchronous scene-load transfer; never release staging before GPU completion.
    struct Transfer {
        DkMemBlock staging = nullptr, commands = nullptr;
        DkCmdBuf cmd = nullptr;
        ~Transfer() {
            if (cmd) dkCmdBufDestroy(cmd);
            if (commands) dkMemBlockDestroy(commands);
            if (staging) dkMemBlockDestroy(staging);
        }
    } transfer;
    const unsigned pitch = unsigned(align_up(std::uint64_t(image.width)*4,DK_IMAGE_LINEAR_STRIDE_ALIGNMENT));
    transfer.staging = p->memory(unsigned(align_up(std::uint64_t(pitch)*image.height,DK_MEMBLOCK_ALIGNMENT)),DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    transfer.commands = p->memory(kCommandBytes,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    DkCmdBufMaker commands; dkCmdBufMakerDefaults(&commands,p->device);
    transfer.cmd = dkCmdBufCreate(&commands);
    if (!transfer.staging || !transfer.commands || !transfer.cmd) { error = "Deko3D texture staging allocation failed."; return false; }
    auto* bytes = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(transfer.staging));
    for (int y = 0; y < image.height; ++y)
        std::memcpy(bytes+std::size_t(y)*pitch,image.pixels.data()+std::size_t(y)*image.width*4,std::size_t(image.width)*4);
    dkCmdBufAddMemory(transfer.cmd,transfer.commands,0,kCommandBytes);
    dkCmdBufBarrier(transfer.cmd,DkBarrier_Full,DkInvalidateFlags_L2Cache);
    const DkCopyBuf source{dkMemBlockGetGpuAddr(transfer.staging),pitch,pitch*unsigned(image.height)};
    const DkImageRect rectangle{0,0,0,unsigned(image.width),unsigned(image.height),1};
    dkCmdBufCopyBufferToImage(transfer.cmd,&source,&view,&rectangle,0);
    dkCmdBufBarrier(transfer.cmd,DkBarrier_Full,DkInvalidateFlags_Image);
    const auto list = dkCmdBufFinishList(transfer.cmd);
    if (!list) { error = "Deko3D texture transfer recording failed."; return false; }
    dkQueueSubmitCommands(p->queue,list);
    dkQueueWaitIdle(p->queue);
    if (dkQueueIsInErrorState(p->queue)) { error = "Deko3D texture transfer failed."; return false; }
    for (auto& slot : p->slots) slot.list = 0;
    p->texture = std::move(next);
    return true;
}
bool Presenter::mesh(const std::array<float,16>& transform, std::string& error, bool verify_red_center, bool textured) {
    if (!p || !p->mesh_ready || !p->geometry) { error = "Deko3D mesh shaders and geometry must be loaded first."; return false; }
    if (textured && !p->texture) { error = "Deko3D textured draw requires an uploaded game texture."; return false; }
    for (const auto value : transform) if (!std::isfinite(value)) { error = "Deko3D mesh transform is not finite."; return false; }
    const int index = p->acquire(error); if (index < 0) return false;
    const auto key = (std::uint64_t(p->geometry_generation)<<8)|4|(std::uint64_t(verify_red_center)<<4)|(std::uint64_t(textured)<<5);
    if (!p->prepare(unsigned(index),key,error)) return false;
    auto& slot = p->slots[index]; const auto cmd = slot.command_buffer;
    struct alignas(16) Constants { std::array<float,16> transform; std::array<std::uint32_t,4> material; };
    const Constants constants{transform,{textured ? 1u : 0u,0,0,0}};
    static_assert(sizeof(Constants) == 80);
    std::memcpy(dkMemBlockGetCpuAddr(slot.transform),&constants,sizeof(constants));
    if (!slot.list) {
    DkImageView target, depth_target;
    dkImageViewDefaults(&target,&p->images[index]); dkImageViewDefaults(&depth_target,&p->depth_images[index]);
    const DkViewport viewport{0,0,float(p->width),float(p->height),0,1};
    const DkScissor scissor{0,0,p->width,p->height};
    DkRasterizerState rasterizer; dkRasterizerStateDefaults(&rasterizer); rasterizer.cullMode = DkFace_None;
    DkColorState color; dkColorStateDefaults(&color);
    DkColorWriteState writes; dkColorWriteStateDefaults(&writes);
    DkDepthStencilState depth; dkDepthStencilStateDefaults(&depth);
    DkVtxAttribState attributes[3]{};
    attributes[0].bufferId = 0; attributes[0].offset = offsetof(MeshVertex,position);
    attributes[0].size = DkVtxAttribSize_3x32; attributes[0].type = DkVtxAttribType_Float;
    attributes[1].bufferId = 0; attributes[1].offset = offsetof(MeshVertex,color);
    attributes[1].size = DkVtxAttribSize_4x32; attributes[1].type = DkVtxAttribType_Float;
    attributes[2].bufferId = 0; attributes[2].offset = offsetof(MeshVertex,uv);
    attributes[2].size = DkVtxAttribSize_2x32; attributes[2].type = DkVtxAttribType_Float;
    const DkVtxBufferState vertex_state{sizeof(MeshVertex),0};
    const DkBufExtents uniform{dkMemBlockGetGpuAddr(slot.transform),DK_UNIFORM_BUF_ALIGNMENT};
    const DkShader* shaders[] = {&p->mesh_vertex,&p->mesh_fragment};
    dkCmdBufBarrier(cmd,DkBarrier_Full,DkInvalidateFlags_L2Cache | DkInvalidateFlags_Shader);
    dkCmdBufBindRenderTarget(cmd,&target,&depth_target);
    dkCmdBufSetViewports(cmd,0,&viewport,1); dkCmdBufSetScissors(cmd,0,&scissor,1);
    dkCmdBufClearColorFloat(cmd,0,DkColorMask_RGBA,0.05f,0.10f,0.30f,1.0f);
    dkCmdBufClearDepthStencil(cmd,true,1.0f,0xff,0);
    dkCmdBufBindRasterizerState(cmd,&rasterizer); dkCmdBufBindColorState(cmd,&color);
    dkCmdBufBindColorWriteState(cmd,&writes); dkCmdBufBindDepthStencilState(cmd,&depth);
    dkCmdBufBindShaders(cmd,DkStageFlag_GraphicsMask,shaders,2);
    dkCmdBufBindUniformBuffers(cmd,DkStage_Vertex,0,&uniform,1);
    dkCmdBufBindUniformBuffers(cmd,DkStage_Fragment,0,&uniform,1);
    if (textured) {
        const auto address = dkMemBlockGetGpuAddr(p->texture->descriptors);
        dkCmdBufBarrier(cmd,DkBarrier_Full,DkInvalidateFlags_Descriptors | DkInvalidateFlags_Image);
        dkCmdBufBindImageDescriptorSet(cmd,address,1);
        dkCmdBufBindSamplerDescriptorSet(cmd,address+DK_MEMBLOCK_ALIGNMENT/2,1);
        dkCmdBufBindTexture(cmd,DkStage_Fragment,0,dkMakeTextureHandle(0,0));
    }
    dkCmdBufBindVtxAttribState(cmd,attributes,3); dkCmdBufBindVtxBufferState(cmd,&vertex_state,1);
    dkCmdBufBindVtxBuffer(cmd,0,dkMemBlockGetGpuAddr(p->geometry),p->mesh_vertex_bytes);
    dkCmdBufBindIdxBuffer(cmd,DkIdxFormat_Uint32,dkMemBlockGetGpuAddr(p->geometry)+p->mesh_index_offset);
    dkCmdBufDrawIndexed(cmd,DkPrimitive_Triangles,p->mesh_indices,1,0,0,0);
    if (verify_red_center) {
        dkCmdBufBarrier(cmd,DkBarrier_Full,DkInvalidateFlags_Image);
        const DkImageRect center{p->width/2,p->height/2,0,1,1,1};
        const DkCopyBuf destination{dkMemBlockGetGpuAddr(p->readback),DK_IMAGE_LINEAR_STRIDE_ALIGNMENT,DK_IMAGE_LINEAR_STRIDE_ALIGNMENT};
        dkCmdBufCopyImageToBuffer(cmd,&target,&center,&destination,0);
    }
    }
    if (!p->submit(unsigned(index),error,verify_red_center)) return false;
    if (verify_red_center && *static_cast<const std::uint32_t*>(dkMemBlockGetCpuAddr(p->readback)) != 0xff0000ffu) {
        error = "Deko3D indexed-mesh/depth check did not keep the near red triangle visible."; return false;
    }
    return true;
}
std::uint64_t Presenter::frames() const { return p ? p->frame_count : 0; }

struct SceneRenderer::Impl {
    struct Geometry {
        DkMemBlock memory = nullptr;
        unsigned vertices_bytes = 0, index_offset = 0, index_count = 0;
        ~Geometry() { if (memory) dkMemBlockDestroy(memory); }
    };
    struct Frame {
        DkMemBlock commands = nullptr, uniforms = nullptr, dynamic = nullptr;
        DkCmdBuf cmd = nullptr;
        ~Frame() {
            if (cmd) dkCmdBufDestroy(cmd);
            if (commands) dkMemBlockDestroy(commands);
            if (uniforms) dkMemBlockDestroy(uniforms);
            if (dynamic) dkMemBlockDestroy(dynamic);
        }
    };
    Presenter& owner;
    std::array<Frame,kBuffers> frames;
    std::unordered_map<MeshId,std::unique_ptr<Geometry>> geometry;
    std::unordered_map<TextureId,std::unique_ptr<Presenter::Impl::Texture>> textures;
    MeshId next_mesh = 1;
    TextureId next_texture = 1;
    DkMemBlock code = nullptr;
    DkShader vertex{}, fragment{};
    bool ready = false;
    SceneCounters counters;
    static constexpr unsigned command_bytes = 2*1024*1024, uniform_bytes = 16*1024*1024, dynamic_bytes = 8*1024*1024;
    explicit Impl(Presenter& presenter) : owner(presenter) {}
    ~Impl() {
        if (owner.p && owner.p->queue) dkQueueWaitIdle(owner.p->queue);
        geometry.clear(); textures.clear();
        if (code) dkMemBlockDestroy(code);
    }
    bool idle(std::string& error) {
        if (!owner.p || dkQueueIsInErrorState(owner.p->queue)) { error = "Native scene renderer has no healthy display device."; return false; }
        for (auto& slot : owner.p->slots) if (!owner.p->wait(slot,error)) return false;
        return true;
    }
};
SceneRenderer::SceneRenderer(Presenter& presenter) : p(std::make_unique<Impl>(presenter)) {}
SceneRenderer::~SceneRenderer() = default;
bool SceneRenderer::load_shaders(std::span<const std::uint8_t> vertex, std::span<const std::uint8_t> fragment, std::string& error) {
    if (!p->owner.p || p->code) { error = "Native scene shaders require an open display and load only once."; return false; }
    p->ready = load_program(p->owner.p->device,p->code,p->vertex,p->fragment,vertex,fragment,error);
    return p->ready;
}
MeshId SceneRenderer::create_mesh(std::span<const scene::Vertex> vertices, std::span<const std::uint32_t> indices, std::string& error) {
    if (vertices.empty() || indices.empty() || indices.size()%3 || vertices.size()>4'000'000 || indices.size()>12'000'000) {
        error = "Native scene geometry is empty, oversized or not triangular."; return 0;
    }
    for (const auto index : indices) if (index >= vertices.size()) { error = "Native scene index exceeds its vertex buffer."; return 0; }
    if (!p->next_mesh) { error="Native mesh handle capacity exhausted."; return 0; }
    if (!p->idle(error)) return 0;
    auto next = std::make_unique<Impl::Geometry>();
    next->vertices_bytes = unsigned(vertices.size_bytes());
    next->index_offset = unsigned(align_up(vertices.size_bytes(),4));
    next->index_count = unsigned(indices.size());
    next->memory = p->owner.p->memory(unsigned(align_up(next->index_offset+indices.size_bytes(),DK_MEMBLOCK_ALIGNMENT)),DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    if (!next->memory) { error = "Native scene retained geometry allocation failed."; return 0; }
    auto* data = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(next->memory));
    std::memcpy(data,vertices.data(),vertices.size_bytes());
    std::memcpy(data+next->index_offset,indices.data(),indices.size_bytes());
    const auto id = p->next_mesh++;
    p->geometry.emplace(id,std::move(next));
    return id;
}
TextureId SceneRenderer::create_texture(const renderer::Rgba8Image& image, bool linear, bool repeat_s, bool repeat_t, std::string& error) {
    if (!p->next_texture) { error="Native texture handle capacity exhausted."; return 0; }
    if (!p->idle(error)) return 0;
    auto& owner = p->owner;
    auto previous = std::move(owner.p->texture);
    const bool uploaded = owner.upload_texture(image,error,linear,repeat_s,repeat_t ? 1 : 0);
    auto next = std::move(owner.p->texture);
    owner.p->texture = std::move(previous);
    if (!uploaded) return 0;
    const auto id = p->next_texture++;
    p->textures.emplace(id,std::move(next));
    return id;
}
void SceneRenderer::release_resources() {
    if (p->owner.p && p->owner.p->queue) dkQueueWaitIdle(p->owner.p->queue);
    p->geometry.clear(); p->textures.clear();
    // IDs remain monotonic, so a stale scene handle cannot alias a new level.
}
SceneCounters SceneRenderer::counters() const { return p->counters; }

namespace {
struct SpanKey {
    const void* data;
    std::size_t size;
    bool operator==(const SpanKey&) const = default;
};
struct SpanHash {
    std::size_t operator()(const SpanKey& key) const {
        return std::hash<const void*>{}(key.data) ^ (std::hash<std::size_t>{}(key.size)<<1);
    }
};
struct alignas(16) SceneConstants {
    renderer::Mat4 projection, model, view;
    std::array<float,4> tint, material, moby_light, ambient, fog_params, fog_colour;
    std::array<std::uint32_t,4> flags;
};
static_assert(sizeof(SceneConstants) == 304);
bool finite_matrix(const renderer::Mat4& matrix) {
    return std::all_of(matrix.begin(),matrix.end(),[](float v) { return std::isfinite(v); });
}
}
bool SceneRenderer::render(const SceneView& view, std::span<const SceneDraw> draws, std::string& error, bool verify_red_center) {
    if (!p->ready || !p->owner.p) { error = "Native scene renderer shaders/display are not ready."; return false; }
    auto& core = *p->owner.p;
    if (!finite_matrix(view.projection) || !finite_matrix(view.view) || draws.size()>4096) {
        error = "Native scene camera is invalid or its draw count exceeds 4096."; return false;
    }
    // Validate before acquisition: never silently omit unsupported/invalid objects.
    std::size_t required = 256*sizeof(renderer::Mat4) + 1024, dynamic_required = 0;
    std::unordered_map<SpanKey,bool,SpanHash> counted_palettes, counted_lights;
    for (const auto& draw : draws) {
        const auto found = p->geometry.find(draw.mesh);
        if ((draw.mesh && found == p->geometry.end()) || (draw.texture && !p->textures.contains(draw.texture))) {
            error = "Native scene draw references a missing mesh or texture."; return false;
        }
        if (!draw.mesh) {
            if (draw.dynamic_vertices.empty() || draw.dynamic_indices.empty()
                || draw.dynamic_vertices.size_bytes()>Impl::dynamic_bytes || draw.dynamic_indices.size_bytes()>Impl::dynamic_bytes) {
                error="Native dynamic draw has no geometry or exceeds the frame ring."; return false;
            }
            for (const auto index : draw.dynamic_indices) if (index>=draw.dynamic_vertices.size()) {
                error="Native dynamic draw has an invalid vertex index."; return false;
            }
            dynamic_required+=align_up(draw.dynamic_vertices.size_bytes(),16)+align_up(draw.dynamic_indices.size_bytes(),16);
            if (dynamic_required>Impl::dynamic_bytes) { error="Native dynamic geometry ring capacity exceeded."; return false; }
        } else if (!draw.dynamic_vertices.empty() || !draw.dynamic_indices.empty()) {
            error="Native draw mixes retained and dynamic geometry."; return false;
        }
        const auto available=draw.mesh ? found->second->index_count : draw.dynamic_indices.size();
        if (draw.index_count%3 || draw.first_index>available || draw.index_count>available-draw.first_index
            || draw.joints.size()>256 || draw.light_colours.size()>64 || !finite_matrix(draw.model)) {
            error = "Native scene draw has invalid indices, transform, joint palette or light colours."; return false;
        }
        for (const auto& joint : draw.joints) if (!finite_matrix(joint)) { error = "Native scene joint matrix is not finite."; return false; }
        required += align_up(sizeof(SceneConstants),DK_UNIFORM_BUF_ALIGNMENT);
        if (!draw.joints.empty() && counted_palettes.emplace(SpanKey{draw.joints.data(),draw.joints.size()},true).second) required += 256*sizeof(renderer::Mat4);
        if (!draw.light_colours.empty() && counted_lights.emplace(SpanKey{draw.light_colours.data(),draw.light_colours.size()},true).second) required += 64*16;
        if (required>Impl::uniform_bytes) { error = "Native scene uniform capacity exceeded; frame was not submitted."; return false; }
        if (draw.scissor[2] && draw.scissor[3] && (draw.scissor[0]>core.width || draw.scissor[1]>core.height
            || draw.scissor[2]>core.width-draw.scissor[0] || draw.scissor[3]>core.height-draw.scissor[1])) {
            error = "Native scene scissor is outside the display."; return false;
        }
    }
    const int index = core.acquire(error); if (index<0) return false;
    auto& frame = p->frames[index];
    if (!frame.commands) frame.commands = core.memory(Impl::command_bytes,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    if (!frame.uniforms) frame.uniforms = core.memory(Impl::uniform_bytes,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    if (dynamic_required && !frame.dynamic) frame.dynamic=core.memory(Impl::dynamic_bytes,DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    if (frame.cmd) dkCmdBufDestroy(frame.cmd);
    DkCmdBufMaker maker; dkCmdBufMakerDefaults(&maker,core.device); frame.cmd = dkCmdBufCreate(&maker);
    if (!frame.commands || !frame.uniforms || !frame.cmd || (dynamic_required && !frame.dynamic)) { error = "Native scene frame allocation failed."; return false; }
    const auto cmd = frame.cmd;
    dkCmdBufAddMemory(cmd,frame.commands,0,Impl::command_bytes);
    DkImageView target, depth_target;
    dkImageViewDefaults(&target,&core.images[index]); dkImageViewDefaults(&depth_target,&core.depth_images[index]);
    dkCmdBufBindRenderTarget(cmd,&target,&depth_target);
    const DkViewport viewport{0,0,float(core.width),float(core.height),0,1};
    const DkScissor full{0,0,core.width,core.height};
    dkCmdBufSetViewports(cmd,0,&viewport,1); dkCmdBufSetScissors(cmd,0,&full,1);
    dkCmdBufClearColorFloat(cmd,0,DkColorMask_RGBA,view.background[0],view.background[1],view.background[2],view.background[3]);
    dkCmdBufClearDepthStencil(cmd,true,0.0f,0xff,0); // OpenRAC reversed depth
    DkRasterizerState raster; dkRasterizerStateDefaults(&raster); raster.cullMode = DkFace_None;
    DkColorWriteState writes; dkColorWriteStateDefaults(&writes);
    dkCmdBufBindRasterizerState(cmd,&raster); dkCmdBufBindColorWriteState(cmd,&writes);
    const DkShader* shaders[]{&p->vertex,&p->fragment}; dkCmdBufBindShaders(cmd,DkStageFlag_GraphicsMask,shaders,2);
    DkVtxAttribState attributes[7]{};
    static_assert(sizeof(scene::Vertex)<16384);
    const unsigned offsets[]{offsetof(scene::Vertex,position),offsetof(scene::Vertex,normal),offsetof(scene::Vertex,uv),offsetof(scene::Vertex,colour),offsetof(scene::Vertex,joints),offsetof(scene::Vertex,weights),offsetof(scene::Vertex,light_slot)};
    const DkVtxAttribSize sizes[]{DkVtxAttribSize_3x32,DkVtxAttribSize_3x32,DkVtxAttribSize_2x32,DkVtxAttribSize_4x32,DkVtxAttribSize_4x8,DkVtxAttribSize_4x32,DkVtxAttribSize_1x32};
    for (unsigned i=0;i<7;++i) { attributes[i].bufferId=0; attributes[i].offset=offsets[i]&0x3fffu; attributes[i].size=sizes[i]; attributes[i].type=i==4 ? DkVtxAttribType_Uint : DkVtxAttribType_Float; }
    const DkVtxBufferState vertex_state{sizeof(scene::Vertex),0};
    dkCmdBufBindVtxAttribState(cmd,attributes,7); dkCmdBufBindVtxBufferState(cmd,&vertex_state,1);
    auto* cpu = static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(frame.uniforms));
    const auto gpu = dkMemBlockGetGpuAddr(frame.uniforms);
    unsigned cursor = 0;
    const auto upload = [&](const void* bytes,unsigned size,unsigned allocation) {
        const unsigned offset = cursor; cursor += unsigned(align_up(allocation,DK_UNIFORM_BUF_ALIGNMENT));
        std::memset(cpu+offset,0,allocation); std::memcpy(cpu+offset,bytes,size);
        return DkBufExtents{gpu+offset,unsigned(align_up(allocation,DK_UNIFORM_BUF_ALIGNMENT))};
    };
    // Shared immutable per-frame data and safe dummy bindings for unskinned draws.
    static const std::array<std::uint8_t,256*sizeof(renderer::Mat4)> zero{};
    const auto dummy = upload(zero.data(),unsigned(zero.size()),unsigned(zero.size()));
    const auto lights = upload(view.light_sets.data(),sizeof(view.light_sets),sizeof(view.light_sets));
    dkCmdBufBindUniformBuffers(cmd,DkStage_Vertex,3,&lights,1);
    std::unordered_map<SpanKey,DkBufExtents,SpanHash> palettes, colours;
    p->counters = {};
    unsigned dynamic_cursor=0;
    for (const auto& draw : draws) {
        if (!draw.index_count) continue;
        DkGpuAddr vertex_address, index_address;
        unsigned vertex_bytes;
        if (draw.mesh) {
            const auto& geometry=*p->geometry.at(draw.mesh);
            vertex_address=dkMemBlockGetGpuAddr(geometry.memory); vertex_bytes=geometry.vertices_bytes;
            index_address=vertex_address+geometry.index_offset;
        } else {
            auto* mapped=static_cast<std::uint8_t*>(dkMemBlockGetCpuAddr(frame.dynamic));
            const auto base=dkMemBlockGetGpuAddr(frame.dynamic);
            vertex_address=base+dynamic_cursor; vertex_bytes=unsigned(draw.dynamic_vertices.size_bytes());
            std::memcpy(mapped+dynamic_cursor,draw.dynamic_vertices.data(),vertex_bytes);
            dynamic_cursor+=unsigned(align_up(vertex_bytes,16));
            index_address=base+dynamic_cursor;
            std::memcpy(mapped+dynamic_cursor,draw.dynamic_indices.data(),draw.dynamic_indices.size_bytes());
            dynamic_cursor+=unsigned(align_up(draw.dynamic_indices.size_bytes(),16));
        }
        const SceneConstants constants{draw.screen_space ? renderer::identity() : view.projection,draw.model,draw.screen_space ? renderer::identity() : view.view,
            draw.tint,draw.material,draw.moby_light,draw.ambient,view.fog_params,view.fog_colour,
            {draw.texture ? 1u : 0u,unsigned(draw.joints.size()),unsigned(draw.light_colours.size()),(draw.alpha_test ? draw.alpha_test : draw.cutout ? 1u : 0u)|(draw.metal ? 256u : 0u)|(draw.screen_space ? 512u : 0u)}};
        const auto uniform = upload(&constants,sizeof(constants),sizeof(constants));
        dkCmdBufBindUniformBuffers(cmd,DkStage_Vertex,0,&uniform,1); dkCmdBufBindUniformBuffers(cmd,DkStage_Fragment,0,&uniform,1);
        auto palette = dummy;
        if (!draw.joints.empty()) {
            auto [at,inserted] = palettes.try_emplace(SpanKey{draw.joints.data(),draw.joints.size()});
            if (inserted) at->second = upload(draw.joints.data(),unsigned(draw.joints.size_bytes()),256*sizeof(renderer::Mat4));
            palette = at->second;
        }
        dkCmdBufBindUniformBuffers(cmd,DkStage_Vertex,1,&palette,1);
        auto colour = dummy;
        if (!draw.light_colours.empty()) {
            auto [at,inserted] = colours.try_emplace(SpanKey{draw.light_colours.data(),draw.light_colours.size()});
            if (inserted) {
                std::array<std::array<float,4>,64> values{};
                for (unsigned i=0;i<draw.light_colours.size();++i) for (unsigned c=0;c<4;++c)
                    values[i][c]=float((draw.light_colours[i]>>(8*c))&255)/128.0f;
                at->second = upload(values.data(),sizeof(values),sizeof(values));
            }
            colour = at->second;
        }
        dkCmdBufBindUniformBuffers(cmd,DkStage_Vertex,2,&colour,1);
        if (draw.texture) {
            const auto descriptor = dkMemBlockGetGpuAddr(p->textures.at(draw.texture)->descriptors);
            dkCmdBufBindImageDescriptorSet(cmd,descriptor,1); dkCmdBufBindSamplerDescriptorSet(cmd,descriptor+DK_MEMBLOCK_ALIGNMENT/2,1);
            dkCmdBufBindTexture(cmd,DkStage_Fragment,0,dkMakeTextureHandle(0,0));
        }
        // CPU-updated constants and per-texture descriptor base changes are explicit.
        dkCmdBufBarrier(cmd,DkBarrier_Full,DkInvalidateFlags_Shader | DkInvalidateFlags_L2Cache | DkInvalidateFlags_Descriptors | DkInvalidateFlags_Image);
        DkDepthStencilState depth; dkDepthStencilStateDefaults(&depth);
        depth.depthTestEnable=draw.depth_test; depth.depthWriteEnable=draw.depth_write; depth.depthCompareOp=DkCompareOp_Gequal;
        dkCmdBufBindDepthStencilState(cmd,&depth);
        DkColorState color; dkColorStateDefaults(&color); color.blendEnableMask=draw.blend==Blend::Opaque ? 0 : 1;
        DkBlendState blend; dkBlendStateDefaults(&blend);
        // Match destination factors to avoid stock 0.5's separate-alpha bug.
        blend.dstColorBlendFactor=blend.dstAlphaBlendFactor=draw.blend==Blend::Additive ? DkBlendFactor_One : DkBlendFactor_InvSrcAlpha;
        dkCmdBufBindColorState(cmd,&color); dkCmdBufBindBlendState(cmd,0,&blend);
        const DkScissor scissor = draw.scissor[2] && draw.scissor[3] ? DkScissor{draw.scissor[0],draw.scissor[1],draw.scissor[2],draw.scissor[3]} : full;
        dkCmdBufSetScissors(cmd,0,&scissor,1);
        dkCmdBufBindVtxBuffer(cmd,0,vertex_address,vertex_bytes);
        dkCmdBufBindIdxBuffer(cmd,DkIdxFormat_Uint32,index_address);
        dkCmdBufDrawIndexed(cmd,DkPrimitive_Triangles,draw.index_count,1,draw.first_index,0,0);
        ++p->counters.draws; p->counters.triangles += draw.index_count/3;
    }
    p->counters.uniform_bytes=cursor; p->counters.dynamic_bytes=dynamic_cursor;
    if (verify_red_center) {
        dkCmdBufBarrier(cmd,DkBarrier_Full,DkInvalidateFlags_Image);
        const DkImageRect center{core.width/2,core.height/2,0,1,1,1};
        const DkCopyBuf destination{dkMemBlockGetGpuAddr(core.readback),DK_IMAGE_LINEAR_STRIDE_ALIGNMENT,DK_IMAGE_LINEAR_STRIDE_ALIGNMENT};
        dkCmdBufCopyImageToBuffer(cmd,&target,&center,&destination,0);
    }
    auto& slot=core.slots[index];
    slot.list=dkCmdBufFinishList(cmd);
    if (!slot.list) { error="Native scene command recording failed."; return false; }
    const bool submitted=core.submit(unsigned(index),error,verify_red_center);
    slot.list=0; // this list belongs to SceneRenderer's frame, not the probe cache
    if (!submitted) return false;
    if (verify_red_center && *static_cast<const std::uint32_t*>(dkMemBlockGetCpuAddr(core.readback))!=0xff0000ffu) {
        error="Native multi-draw scene center pixel was not red."; return false;
    }
    return true;
}

}

