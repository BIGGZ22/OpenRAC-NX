// SPDX-License-Identifier: GPL-3.0-or-later
#include "check.h"
#include "platform/switch/overlay_protocol.h"
#include <cmath>

int main() {
    using namespace openrac::platform::overlay_stats;
    Timing timing;
    CHECK(!timing.present(100, 6000));
    // 60 actual presents a second, regardless of the game's configured cap.
    for (int i = 1; i <= 60; ++i) CHECK(timing.present(100 + i * 100, 6000));
    CHECK(timing.frames == 61);
    CHECK(timing.fps == 60);
    CHECK(std::abs(timing.average - 60) < 0.001f);
    // Sustained stalls are visible, rather than reporting 60 because it is the selected cap.
    for (int i = 1; i <= 20; ++i) CHECK(timing.present(6100 + i * 300, 6000));
    CHECK(timing.fps == 20);
    CHECK(std::abs(timing.average - 20) < 0.001f);
    CHECK(timing.frames == 81);
    CHECK(!timing.present(timing.previous, 6000));
    CHECK(!timing.present(timing.previous + 100, 0));
    // Exercise the actual publisher against the reader's heartbeat/resolution
    // handshake, keeping overlay-owned control bytes intact.
    FpsBlock block{};
    block.fps_locked=37;
    block.force_suspend=true;
    block.expected_set_buffers=-1;
    block.performance_counters[0]=123;
    block.active=false;
    block.render[0].calls=0xffff;
    publish(block,timing,2,854,480,1280,720);
    CHECK(block.active);
    CHECK(block.api==2);
    CHECK(block.render[0].calls==1);
    CHECK(block.render[0].width==854 && block.render[0].height==480);
    CHECK(block.viewport[0].width==1280 && block.viewport[0].height==720);
    CHECK(block.frame_number==timing.frames && block.fps==timing.fps);
    CHECK(block.fps_locked==37 && block.force_suspend);
    CHECK(block.expected_set_buffers==-1 && block.performance_counters[0]==123);
    for(unsigned i=1;i<8;++i) CHECK(block.render[i].calls==0 && block.viewport[i].calls==0);
    // A renderer/size change replaces old resolution entries, never leaves
    // stale launcher dimensions or a resolution lookup sentinel behind.
    block.active=false;
    block.render[0].calls=0xffff;
    publish(block,timing,3,1920,1080,1920,1080);
    CHECK(block.active && block.api==3 && block.render[0].calls==1);
    CHECK(block.render[0].width==1920 && block.render[0].height==1080);
    publish(block,timing,2,-1,70000,640,480);
    CHECK(block.render[0].width==0 && block.render[0].height==65535);
    return openrac::test::result();
}
