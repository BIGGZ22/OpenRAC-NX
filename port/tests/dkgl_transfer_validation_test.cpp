// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer/dkgl/transfer_validation.h"
#include <cassert>
#include <climits>
#include <cstdio>
int main() {
    using namespace openrac::dkgl::detail;
    assert(texture_rectangle(1024,512,11,0,0,0,1024,512));
    assert(texture_rectangle(3,2,2,1,0,0,1,1));
    assert(!texture_rectangle(3,2,2,1,0,0,2,1));
    assert(!texture_rectangle(1024,512,11,0,-1,0,4,4));
    assert(!texture_rectangle(1024,512,11,0,1020,0,8,4));
    assert(!texture_rectangle(1024,512,11,31,0,0,1,1));
    assert(!texture_rectangle(1024,512,11,0,INT_MAX,0,INT_MAX,1));
    std::uint32_t pitch=0,size=0;
    assert(staging_layout(3,2,4,48u<<20,pitch,size) && pitch==32 && size==64);
    assert(staging_layout(1024,2,16,48u<<20,pitch,size) && pitch==16384 && size==32768);
    assert(staging_layout(1,1,16,48u<<20,pitch,size) && pitch==32 && size==32);
    assert(!staging_layout(INT_MAX,INT_MAX,16,48u<<20,pitch,size));
    assert(!staging_layout(1024,4096,16,48u<<20,pitch,size));
    assert(!staging_layout(-1,1,4,48u<<20,pitch,size));
    assert(!staging_layout(1,1,0,48u<<20,pitch,size));
    std::puts("PASS: RGBA8/float staging, mip rectangles, padded rows and overflow limits");
}
