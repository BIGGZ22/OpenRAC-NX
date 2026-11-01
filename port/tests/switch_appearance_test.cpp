// SPDX-License-Identifier: GPL-3.0-or-later
#include "check.h"
#include "platform/switch/launcher_appearance.h"
#include <filesystem>
#include <fstream>
using namespace openrac::platform::appearance;
int main(int argc, char** argv) {
    CHECK(argc == 2); if (argc != 2) return 1;
    Color c = rgb(1,2,3);
    CHECK(parse_color("#44E8C1",c)); CHECK(c == rgb(68,232,193));
    CHECK(parse_color("aBcDeF",c)); CHECK(hex(c) == "#ABCDEF");
    const Color before = c;
    CHECK(!parse_color("#12345",c)); CHECK(c == before);
    CHECK(!parse_color("#12345678",c)); CHECK(!parse_color("#FF00GG",c));
    CHECK(!parse_color(" 112233",c)); CHECK(!parse_color("#12\n345",c));
    CHECK(contrasting(rgb(255,255,255)) == rgb(9,17,30));
    CHECK(contrasting(rgb(0,0,0)) == rgb(255,255,255));
    Settings s;
    s.select(Preset::Light); CHECK(s.palette().panel == light().panel); CHECK(s.banner_text == light().text);
    s.color(5,rgb(140,90,220)); CHECK(s.preset == Preset::Custom);
    CHECK(s.custom.panel == light().panel); CHECK(s.palette().accent == rgb(140,90,220));
    s.select(Preset::Dark); CHECK(s.palette().panel == dark().panel);
    s.select(Preset::Custom); CHECK(s.palette().accent == rgb(140,90,220));
    s.background = Background::LocalImage; s.window_art = WindowArt::Hidden;
    s.tint = 35; s.title = "My adventure"; s.heading = ""; s.subtitle = "A custom banner";
    const std::filesystem::path file(argv[1]);
    CHECK(save(file,s)); auto loaded = load(file);
    CHECK(loaded.title == s.title && loaded.heading.empty() && loaded.subtitle == s.subtitle);
    CHECK(loaded.preset == Preset::Custom && loaded.palette().accent == s.palette().accent);
    CHECK(loaded.window_art == WindowArt::Hidden && loaded.background == Background::LocalImage && loaded.tint == 35);
    s.title = "Title\npreset=1"; CHECK(save(file,s));
    CHECK(load(file).title == "Titlepreset=1"); CHECK(load(file).preset == Preset::Custom);
    s.title = "Second choice"; CHECK(save(file,s)); CHECK(load(file).title == "Second choice");
    std::filesystem::rename(file,file.string()+".previous"); CHECK(load(file).title == "Second choice");
    CHECK(save(file,s)); CHECK(load(file).title == "Second choice");
    { std::ofstream bad(file); bad << "preset=999\nbackground=-2\nwindow_art=900\ntint=9999\naccent_color=#NOPE\n"; }
    loaded = load(file);
    CHECK(loaded.preset == Preset::Custom && loaded.background == Background::Solid);
    CHECK(loaded.window_art == WindowArt::Hidden && loaded.tint == 90);
    CHECK(loaded.custom.accent == dark().accent);
    CHECK(load(file.string()+".missing").preset == Preset::Dark);
    return openrac::test::result();
}
