// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer/dkgl/sampler_refresh.h"
#include <array>
#include <cassert>
#include <cstdio>
using namespace openrac::dkgl::detail;
int main() {
    std::vector<bool> read(16,false);
    const std::array<std::uint32_t,3> draw{(1u<<20)|4,(2u<<20)|5,(3u<<20)|6};
    assert(refresh_draw_samplers(read,draw));
    assert(read[1] && read[2] && read[3]);
    for(int i=0;i<1000;++i) assert(!refresh_draw_samplers(read,draw));
    const std::array<std::uint32_t,3> changed_images{(1u<<20)|8,(2u<<20)|9,(3u<<20)|10};
    assert(!refresh_draw_samplers(read,changed_images));
    const std::array<std::uint32_t,2> next{(3u<<20)|1,(4u<<20)|2};
    assert(refresh_draw_samplers(read,next));
    assert(read[3] && read[4] && !read[1]);
    assert(refresh_draw_samplers(read,draw));
    assert(!refresh_draw_samplers(read,draw));
    assert(!refresh_draw_samplers(read,{}));
    std::fill(read.begin(),read.end(),false); // new command list
    assert(refresh_draw_samplers(read,draw));
    std::puts("PASS: multi-sampler draws refresh once, repeated draws stay warm, list resets refresh");

    // The default rule forgets other samplers at each refresh: draws alternating between two
    // sampler sets refresh on every switch (what the counters show as ~0.4 rebinds a draw).
    const std::array<std::uint32_t,1> a{(5u<<20)|1}, b{(6u<<20)|2};
    std::fill(read.begin(),read.end(),false);
    int on_refreshes=0;
    for(int i=0;i<10;++i) on_refreshes+=refresh_draw_samplers(read,i%2 ? b : a);
    assert(on_refreshes==10);

    // first: once per sampler per list, others are kept; a new list refreshes again.
    std::fill(read.begin(),read.end(),false);
    int first_refreshes=0;
    for(int i=0;i<10;++i) first_refreshes+=refresh_first_samplers(read,i%2 ? b : a);
    assert(first_refreshes==2);
    assert(!refresh_first_samplers(read,a) && !refresh_first_samplers(read,b));
    assert(refresh_first_samplers(read,draw));            // samplers 1..3 first read here
    assert(!refresh_first_samplers(read,{}));
    std::fill(read.begin(),read.end(),false);             // new command list
    assert(refresh_first_samplers(read,a));

    // new: only a sampler written after the set was bound (index >= the count bound then).
    assert(!refresh_new_samplers(7,a) && !refresh_new_samplers(7,b) && !refresh_new_samplers(7,draw));
    assert(refresh_new_samplers(6,b));                    // sampler 6 written after a 6-sampler bind
    assert(!refresh_new_samplers(0,{}));

    assert(parse_sampler_rebind(nullptr)==SamplerRebind::On);
    assert(parse_sampler_rebind("1")==SamplerRebind::On);
    assert(parse_sampler_rebind("0")==SamplerRebind::Off);
    assert(parse_sampler_rebind("first")==SamplerRebind::First);
    assert(parse_sampler_rebind("new")==SamplerRebind::New);
    assert(parse_sampler_rebind("typo")==SamplerRebind::On);
    std::puts("PASS: default rule ping-pongs on alternating samplers; first/new experiments refresh once; parsing keeps the default");
}
