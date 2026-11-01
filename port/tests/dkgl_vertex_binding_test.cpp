// SPDX-License-Identifier: GPL-3.0-or-later
// Host regression for the SDK 0.5 capture/replay workaround. No GPU needed.
#include "renderer/dkgl/vertex_bind_compat.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>

namespace {
struct Encoder {
    std::uint32_t* capture=nullptr;
    unsigned capacity=0, count=0, reserved=0, remaining=0, reallocations=0;
    bool short_capture=false;
    std::vector<std::uint32_t> recorded;
};
Encoder& state(DkCmdBuf command) { return *reinterpret_cast<Encoder*>(command); }
DkCmdBuf handle(Encoder& command) { return reinterpret_cast<DkCmdBuf>(&command); }
}
extern "C" {
void dkCmdBufBeginCaptureCmds(DkCmdBuf command,std::uint32_t* storage,std::uint32_t words) {
    auto& e=state(command); e.capture=storage; e.capacity=words; e.count=0;
}
void dkCmdBufBindVtxBuffers(DkCmdBuf command,std::uint32_t first,const DkBufExtents* buffers,std::uint32_t count) {
    auto& e=state(command);
    // Model the verified installed library: reserves five, writes six words.
    e.reserved=5*count;
    for(unsigned i=0;i<count;++i) {
        auto address=buffers[i].size ? buffers[i].addr : 0x1000;
        auto limit=buffers[i].size ? address+buffers[i].size-1 : 0xfff;
        const std::uint32_t packet[]{0x20020000u+0x701u+4*(first+i),std::uint32_t(address>>32),std::uint32_t(address),
                                    0x20020000u+0x7c0u+2*(first+i),std::uint32_t(limit>>32),std::uint32_t(limit)};
        assert(e.count+6<e.capacity);
        std::copy(std::begin(packet),std::end(packet),e.capture+e.count); e.count+=6;
    }
}
std::uint32_t dkCmdBufEndCaptureCmds(DkCmdBuf command) {
    auto& e=state(command); e.capture=nullptr; return e.count-(e.short_capture ? 1u : 0u);
}
void dkCmdBufReplayCmds(DkCmdBuf command,const std::uint32_t* words,std::uint32_t count) {
    auto& e=state(command); e.reserved=count;
    if(e.remaining<=count) { ++e.reallocations; e.remaining=count+1; }
    e.recorded.insert(e.recorded.end(),words,words+count); e.remaining-=count;
}
}
int main() {
    using openrac::dkgl::detail::bind_vertex_buffers_safe;
    std::array<DkBufExtents,16> buffers{};
    for(unsigned i=0;i<buffers.size();++i) buffers[i]={0x1234567800ull+4096*i,256+16*i};
    buffers[3]={0,0}; // SDK's legal empty-range encoding
    for(unsigned count=1;count<=16;++count) for(unsigned first=0;first+count<=16;++first) {
        Encoder destination,scratch; destination.remaining=5*count;
        assert(bind_vertex_buffers_safe(handle(destination),handle(scratch),first,{buffers.data(),count}));
        assert(scratch.reserved==5*count && destination.reserved==6*count);
        assert(destination.reallocations==1 && destination.recorded.size()==6*count);
        for(unsigned i=0;i<count;++i) {
            const auto address=buffers[i].size ? buffers[i].addr : 0x1000;
            const auto limit=buffers[i].size ? address+buffers[i].size-1 : 0xfff;
            const auto& words=destination.recorded;
            assert(words[6*i]==0x20020701u+4*(first+i));
            assert(words[6*i+1]==address>>32 && words[6*i+2]==std::uint32_t(address));
            assert(words[6*i+3]==0x200207c0u+2*(first+i));
            assert(words[6*i+4]==limit>>32 && words[6*i+5]==std::uint32_t(limit));
        }
    }
    Encoder destination,scratch;
    assert(bind_vertex_buffers_safe(handle(destination),handle(scratch),16,{}));
    assert(destination.recorded.empty());
    assert(!bind_vertex_buffers_safe(handle(destination),handle(scratch),1,buffers));
    assert(!bind_vertex_buffers_safe(handle(destination),handle(scratch),17,{}));
    scratch.short_capture=true;
    assert(!bind_vertex_buffers_safe(handle(destination),handle(scratch),0,buffers));
    assert(destination.recorded.empty());
    std::puts("PASS: all vertex binding ranges, boundary reservation, empty buffers and capture failures");
}
