// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <algorithm>
namespace openrac::dkgl::detail {
inline bool geometry_span(std::uint64_t size,std::uint64_t offset,std::uint64_t count,
                          std::uint64_t stride,std::uint64_t element) {
    if(!count) return offset<=size;
    if(offset>size || element>size-offset) return false;
    return !stride || count-1<=(size-offset-element)/stride;
}
struct IndexRange { std::uint32_t first=0,last=0; bool any=false; };
inline bool geometry_indices(std::span<const std::uint8_t> data,std::uint64_t offset,
                             std::uint64_t count,unsigned bytes,bool restart,IndexRange& out) {
    out={};
    if((bytes!=1 && bytes!=2 && bytes!=4) || !geometry_span(data.size(),offset,count,bytes,bytes)) return false;
    const auto sentinel=bytes==1 ? 0xffu : bytes==2 ? 0xffffu : 0xffffffffu;
    for(std::uint64_t i=0;i<count;++i) {
        std::uint32_t index=0;
        std::memcpy(&index,data.data()+offset+i*bytes,bytes);
        if(restart && index==sentinel) continue;
        if(!out.any) { out.first=out.last=index; out.any=true; }
        else { out.first=std::min(out.first,index); out.last=std::max(out.last,index); }
    }
    return true;
}
}
