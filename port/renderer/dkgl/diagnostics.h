// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <string_view>
#include <span>
#include <cstdint>
#include <cstring>
namespace openrac::dkgl::detail {
enum class Diagnostic { None, NoEffects, NoShadows, BindPose, Invalid };
enum class ProgramRole { Other, Effect, Shadow, SkinnedMesh };
constexpr Diagnostic parse_diagnostic(std::string_view value) {
    if(value.empty() || value=="none") return Diagnostic::None;
    if(value=="no-effects") return Diagnostic::NoEffects;
    if(value=="no-shadows") return Diagnostic::NoShadows;
    if(value=="bind-pose") return Diagnostic::BindPose;
    return Diagnostic::Invalid;
}
// Match the actual Switch shader interfaces, not a numeric pack key that changes
// with source edits. Unknown/new programs remain enabled.
constexpr ProgramRole program_role(std::string_view vertex,std::string_view fragment) {
    if(fragment.find("uniform float opacity;")!=std::string_view::npos) return ProgramRole::Shadow;
    if(vertex.find("uniform mat4 view_projection;")!=std::string_view::npos
        && vertex.find("out vec3 v_stq;")!=std::string_view::npos) return ProgramRole::Effect;
    if(vertex.find("uniform int skinning;")!=std::string_view::npos
        && vertex.find("uniform sampler2D palette_texture;")!=std::string_view::npos) return ProgramRole::SkinnedMesh;
    return ProgramRole::Other;
}
constexpr bool omit_program(Diagnostic mode,ProgramRole role) {
    return (mode==Diagnostic::NoEffects && role==ProgramRole::Effect)
        || (mode==Diagnostic::NoShadows && role==ProgramRole::Shadow);
}
inline bool override_skinning(std::span<std::uint8_t> uploaded,int offset) {
    if(offset<0 || std::size_t(offset)>uploaded.size() || uploaded.size()-std::size_t(offset)<sizeof(std::int32_t)) return false;
    const std::int32_t off=0;
    std::memcpy(uploaded.data()+offset,&off,sizeof(off));
    return true;
}
}
