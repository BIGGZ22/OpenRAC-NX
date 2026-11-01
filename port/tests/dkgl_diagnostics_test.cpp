// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer/dkgl/diagnostics.h"
#include <array>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>
using namespace openrac::dkgl::detail;
std::string read(const char* path) {
    std::ifstream file(path); assert(file);
    return {std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};
}
int main() {
    assert(parse_diagnostic("")==Diagnostic::None);
    assert(parse_diagnostic("none")==Diagnostic::None);
    assert(parse_diagnostic("no-effects")==Diagnostic::NoEffects);
    assert(parse_diagnostic("no-shadows")==Diagnostic::NoShadows);
    assert(parse_diagnostic("bind-pose")==Diagnostic::BindPose);
    assert(parse_diagnostic("typo")==Diagnostic::Invalid);
    const auto direct=read("port/renderer/shaders/direct.vert");
    const auto fragment=read("port/renderer/shaders/direct.frag");
    assert(program_role(direct,fragment)==ProgramRole::Other);
    assert(program_role(read("port/renderer/shaders/effect.vert"),fragment)==ProgramRole::Effect);
    const auto shadow=read("port/renderer/shaders/shadow.frag");
    assert(program_role(read("port/renderer/shaders/shadow.vert"),shadow)==ProgramRole::Shadow);
    assert(program_role(read("port/renderer/shaders/shadow_composite.vert"),shadow)==ProgramRole::Shadow);
    assert(program_role(read("port/viewer/shaders/mesh.vert"),read("port/viewer/shaders/mesh.frag"))==ProgramRole::SkinnedMesh);
    assert(program_role(read("port/viewer/shaders/mesh_depth.vert"),read("port/viewer/shaders/mesh_depth.frag"))==ProgramRole::SkinnedMesh);
    for(auto role:{ProgramRole::Other,ProgramRole::Effect,ProgramRole::Shadow,ProgramRole::SkinnedMesh}) {
        assert(!omit_program(Diagnostic::None,role));
        assert(!omit_program(Diagnostic::BindPose,role));
        assert(omit_program(Diagnostic::NoEffects,role)==(role==ProgramRole::Effect));
        assert(omit_program(Diagnostic::NoShadows,role)==(role==ProgramRole::Shadow));
    }
    std::array<std::uint8_t,32> data; data.fill(0xa5);
    const auto original=data;
    assert(override_skinning(data,8));
    for(unsigned i=0;i<data.size();++i) assert(data[i]==(i>=8 && i<12 ? 0 : original[i]));
    data=original;
    assert(!override_skinning(data,-1)); assert(!override_skinning(data,29)); assert(!override_skinning(data,100));
    assert(data==original);
    std::puts("PASS: actual shader classification, isolated pass omission and safe GPU-only skinning override");
}
