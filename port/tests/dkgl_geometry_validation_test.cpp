// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer/dkgl/geometry_validation.h"
#include <array>
#include <cassert>
#include <limits>
#include <cstdio>
using namespace openrac::dkgl::detail;
int main() {
    assert(geometry_span(120,104,1,120,16));
    assert(!geometry_span(120,104,2,120,16));
    assert(!geometry_span(120,121,1,0,4));
    assert(!geometry_span(120,0,std::numeric_limits<std::uint64_t>::max(),120,16));
    assert(geometry_span(120,0,999,0,16));
    std::array<std::uint8_t,8> data{0,0,2,0,255,255,1,0}; IndexRange range;
    assert(geometry_indices(data,2,3,2,true,range));
    assert(range.any && range.first==1 && range.last==2);
    assert(geometry_indices(data,2,3,2,false,range) && range.last==65535);
    assert(!geometry_indices(data,3,3,2,false,range));
    assert(!geometry_indices(data,0,1,3,false,range));
    assert(geometry_indices(data,4,1,2,true,range) && !range.any);
    std::puts("PASS: index offsets, restart indices and overflow-safe geometry spans");
}
