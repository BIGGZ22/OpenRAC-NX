// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

#include "renderer/direct.h"

#include <cstdlib>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <format>

#include "common/log.h"
#include "renderer/gl.h"
#include "renderer_shaders.h"

namespace openrac::renderer {

using namespace openrac::gl;

namespace {

// A scissor this many lines tall or more covers the frame (the chip's frame is 448 lines; a game
// may leave a few out at the edges).
constexpr std::int64_t kFrameFillLines = 400;

// A frame buffer is read back as a texture only in a colour format; an indexed texture (PSMT8,
// PSMT4) at the same address is a texture of its own that the game pages in there.
bool is_colour_format(std::uint8_t psm) {
    return psm == 0x00 || psm == 0x01 || psm == 0x02 || psm == 0x0A;
}

std::uint64_t read64(std::span<const std::uint8_t> data, std::size_t offset) {
    std::uint64_t v = 0;
    std::memcpy(&v, data.data() + offset, 8);
    return v;
}

std::uint32_t read32(std::span<const std::uint8_t> data, std::size_t offset) {
    std::uint32_t v = 0;
    std::memcpy(&v, data.data() + offset, 4);
    return v;
}

float as_float(std::uint32_t bits) {
    return std::bit_cast<float>(bits);
}

// TEST for the draw dumps: alpha test, its fail mode, destination alpha test, depth test.
std::string test_text(const gs::Test& t) {
    return std::format("a{}{}/{:02x}/f{} d{}{} z{}{}", t.ate ? 1 : 0, static_cast<int>(t.atst), t.aref,
                       static_cast<int>(t.afail), t.date ? 1 : 0, t.datm ? 1 : 0, t.zte ? 1 : 0,
                       static_cast<int>(t.ztst));
}

}  // namespace

// ---------------------------------------------------------------------------
// GifInterpreter

GifInterpreter::GifInterpreter(TexturePool& textures, DirectConfig config)
    : m_textures(textures),
      m_config(config) {
    // What the game's draw environment sets up (RENDERER.md section 5): the
    // picture centred at (2048, 2048) of the chip's coordinate space, the
    // whole screen as the scissor, 24-bit depth, the depth test passing.
    for (Context& c : m_context) {
        c.xyoffset =
            {static_cast<std::uint32_t>((2048 - config.screen_width / 2) << 4),
             static_cast<std::uint32_t>((2048 - config.screen_height / 2) << 4)};
        c.scissor =
            {0,
             static_cast<std::uint32_t>(config.screen_width - 1),
             0,
             static_cast<std::uint32_t>(config.screen_height - 1)};
        c.test.zte = true;
        c.test.ztst = gs::DepthTest::Always;
        c.zbuf.psm = 1;
    }
}

void GifInterpreter::clear() {
    m_gif_pending.clear();
    m_targets.clear();
    m_vertices.clear();
    m_draws.clear();
    m_error.clear();
}

bool GifInterpreter::fail(std::string message) {
    m_error = std::move(message);
    return false;
}

const gs::Prim& GifInterpreter::attributes() const {
    return m_use_prim_attributes ? m_prim : m_prmode;
}

const GifInterpreter::Context& GifInterpreter::context() const {
    return m_context[attributes().ctxt ? 1 : 0];
}

bool GifInterpreter::gif(std::span<const std::uint8_t> input) {
    // A tag whose data the last packet did not finish goes on in this one: the GIF keeps its
    // state between transfers (an image upload is often split over several VIF DIRECTs).
    std::vector<std::uint8_t> joined;
    std::span<const std::uint8_t> packet = input;
    if (!m_gif_pending.empty()) {
        joined.swap(m_gif_pending);
        joined.insert(joined.end(), input.begin(), input.end());
        packet = joined;
    }
    const auto carry = [&](std::size_t from) {
        m_gif_pending.assign(packet.begin() + static_cast<std::ptrdiff_t>(from), packet.end());
        return true;
    };
    // A register tag (PACKED, REGLIST) cut off by the end of a VIF DIRECT, or image data with no
    // transfer open: the game's own packets always finish those inside the DIRECT (only a
    // transfer's image data runs on into the next one), so this is a transfer of something that
    // is not GIF data (a reference to memory the port has put other bytes in). Dropped, so that
    // the next DIRECT is read from its first tag again.
    const auto cut = [&](std::size_t from) {
        if (!m_in_direct) {
            return carry(from);
        }
        log::debug("a GIF register tag runs past the end of its DIRECT ({} bytes left): dropped",
                   packet.size() - from);
        return true;
    };
    std::size_t at = 0;
    while (at + 16 <= packet.size()) {
        const gs::GifTag tag = gs::GifTag::decode(read64(packet, at), read64(packet, at + 8));
        const std::size_t tag_at = at;
        at += 16;
        switch (tag.flg) {
            case gs::GifTag::kPacked: {
                const std::size_t size = std::size_t{tag.nloop} * tag.nreg * 16;
                if (at + size > packet.size()) {
                    return cut(tag_at);
                }
                if (tag.pre) {
                    write_prim(tag.prim);
                }
                for (std::uint32_t loop = 0; loop < tag.nloop; ++loop) {
                    for (std::uint32_t r = 0; r < tag.nreg; ++r) {
                        const std::uint64_t lo = read64(packet, at);
                        const std::uint64_t hi = read64(packet, at + 8);
                        at += 16;
                        switch (tag.reg(r)) {
                            case gs::kPackedPrim:
                                write_prim(lo & 0x7FF);
                                break;
                            case gs::kPackedRgbaq:
                                m_current.rgba[0] = static_cast<std::uint8_t>(lo);
                                m_current.rgba[1] = static_cast<std::uint8_t>(lo >> 32);
                                m_current.rgba[2] = static_cast<std::uint8_t>(hi);
                                m_current.rgba[3] = static_cast<std::uint8_t>(hi >> 32);
                                m_current.q = m_packed_q;
                                break;
                            case gs::kPackedSt:
                                m_current.s = as_float(static_cast<std::uint32_t>(lo));
                                m_current.t = as_float(static_cast<std::uint32_t>(lo >> 32));
                                m_packed_q = as_float(static_cast<std::uint32_t>(hi));
                                break;
                            case gs::kPackedUv:
                                m_current.u = gs::bits(lo, 0, 14);
                                m_current.v = gs::bits(lo, 32, 14);
                                break;
                            case gs::kPackedXyzf2:
                            case gs::kPackedXyzf3: {
                                m_current.x = gs::bits(lo, 0, 16);
                                m_current.y = gs::bits(lo, 32, 16);
                                m_current.z = gs::bits(hi, 4, 24);
                                m_current.fog = static_cast<std::uint8_t>(gs::bits(hi, 36, 8));
                                const bool adc = gs::bits(hi, 47, 1) != 0;
                                kick(tag.reg(r) == gs::kPackedXyzf2 && !adc);
                                break;
                            }
                            case gs::kPackedXyz2:
                            case gs::kPackedXyz3: {
                                m_current.x = gs::bits(lo, 0, 16);
                                m_current.y = gs::bits(lo, 32, 16);
                                m_current.z = gs::bits(hi, 0, 32);
                                const bool adc = gs::bits(hi, 47, 1) != 0;
                                kick(tag.reg(r) == gs::kPackedXyz2 && !adc);
                                break;
                            }
                            case gs::kPackedTex0_1:
                            case gs::kPackedTex0_2:
                            case gs::kPackedClamp1:
                            case gs::kPackedClamp2:
                                // These descriptors are the registers' own addresses.
                                write_register(static_cast<std::uint8_t>(tag.reg(r)), lo);
                                break;
                            case gs::kPackedFog:
                                m_current.fog = static_cast<std::uint8_t>(gs::bits(hi, 36, 8));
                                break;
                            case gs::kPackedAd:
                                write_register(static_cast<std::uint8_t>(hi & 0xFF), lo);
                                break;
                            default:  // NOP, and the reserved 0xB
                                break;
                        }
                    }
                }
                break;
            }
            case gs::GifTag::kReglist: {
                const std::size_t words = std::size_t{tag.nloop} * tag.nreg;
                const std::size_t size = ((words + 1) / 2) * 16;  // padded to whole quadwords
                if (at + size > packet.size()) {
                    return cut(tag_at);
                }
                for (std::size_t i = 0; i < words; ++i) {
                    const auto reg =
                        static_cast<std::uint8_t>(tag.reg(static_cast<std::uint32_t>(i % tag.nreg))
                        );
                    // In a REGLIST the descriptor is the register's address;
                    // A+D and NOP write nothing.
                    if (reg != gs::kPackedAd && reg != gs::kPackedNop) {
                        write_register(reg, read64(packet, at + i * 8));
                    }
                }
                at += size;
                break;
            }
            case gs::GifTag::kImage:
            case gs::GifTag::kDisable: {
                const std::size_t size = std::size_t{tag.nloop} * 16;
                if (at + size > packet.size()) {
                    // Image data runs on into the next DIRECT only for a transfer the game opened
                    // (TRXDIR); without one this is not GIF data either.
                    return m_upload_bytes == 0 ? cut(tag_at) : carry(tag_at);
                }
                image_data(packet.subspan(at, size));
                at += size;
                break;
            }
        }
    }
    if (at != packet.size()) {
        return fail(std::format("GIF packet of {} bytes ends inside a tag", packet.size()));
    }
    return true;
}

bool GifInterpreter::vif(std::span<const std::uint8_t> stream) {
    std::size_t at = 0;
    while (at + 4 <= stream.size()) {
        const std::uint32_t code = read32(stream, at);
        const std::size_t code_at = at;
        at += 4;
        const std::uint32_t command = (code >> 24) & 0x7F;
        const std::uint32_t immediate = code & 0xFFFF;
        switch (command) {
            case 0x00:  // NOP
            case 0x01:  // STCYCL
            case 0x02:  // OFFSET
            case 0x03:  // BASE
            case 0x04:  // ITOP
            case 0x05:  // STMOD
            case 0x06:  // MSKPATH3
            case 0x07:  // MARK
            case 0x10:  // FLUSHE
            case 0x11:  // FLUSH
            case 0x13:  // FLUSHA
                break;
            case 0x20:  // STMASK, one word
                at += 4;
                break;
            case 0x30:  // STROW
            case 0x31:  // STCOL, four words
                at += 16;
                break;
            case 0x50:    // DIRECT
            case 0x51: {  // DIRECTHL
                const std::size_t size = std::size_t{immediate == 0 ? 65536u : immediate} * 16;
                if (at + size > stream.size()) {
                    return fail(std::format("VIF DIRECT at {:#x} runs past the stream", code_at));
                }
                m_in_direct = true;
                const bool read = gif(stream.subspan(at, size));
                m_in_direct = false;
                if (!read) {
                    return false;
                }
                at += size;
                break;
            }
            default:
                return fail(std::format(
                    "VIF command {:#04x} at {:#x} feeds a VU program; the direct renderer reads "
                    "only DIRECT data",
                    command,
                    code_at
                ));
        }
    }
    return true;
}

void GifInterpreter::write_prim(std::uint64_t value) {
    m_prim = gs::Prim::decode(value);
    m_queued = 0;  // a PRIM write starts a new primitive
    m_state_dirty = true;
}

void GifInterpreter::write_register(std::uint8_t address, std::uint64_t value) {
    if (g_dump_draws
        && (address == gs::kFrame1 || address == gs::kFrame2 || address == gs::kTrxdir
            || address == gs::kXyoffset1 || address == gs::kScissor1 || address == gs::kTex0_1 || address == gs::kTex0_2 || address == 0x19 || address == 0x41 || address == 0x4d)) {
        log::info(
            "reg {:#04x} = {:#018x} (bitbltbuf sbp {:#x} dbp {:#x} dbw {} trxpos {},{}->{},{} trxreg {}x{})",
            address, value, m_bitbltbuf.sbp, m_bitbltbuf.dbp, m_bitbltbuf.dbw, m_trxpos.ssax, m_trxpos.ssay,
            m_trxpos.dsax, m_trxpos.dsay, m_trxreg.rrw, m_trxreg.rrh
        );
        if (address == gs::kTex0_1 || address == gs::kTex0_2) {
            const gs::Tex0 t = gs::Tex0::decode(value);
            log::info("  tex0 tbp {:#x} psm {:#x} cbp {:#x} csa {} cld {}", t.tbp0, t.psm, t.cbp, t.csa, t.cld);
        }
    }
    Context& c1 = m_context[0];
    Context& c2 = m_context[1];
    switch (address) {
        case 0x7F:  // NOP; the port's marker for a quad of the 2D layer drawn natively
            if ((value >> 48) == kUiQuadMarker && (value & 0xFFFFFFu) < m_ui_quads.size()) {
                native_quad(m_ui_quads[value & 0xFFFFFFu]);
            } else if ((value >> 48) == kMobyMarker && offscreen_target() != 0) {
                DirectDraw d;
                d.state.target = offscreen_target();
                d.first = static_cast<std::uint32_t>(m_vertices.size());
                d.mobys = value;
                m_draws.push_back(d);
            }
            return;
        case gs::kPrim:
            write_prim(value);
            return;
        case gs::kRgbaq:
            for (int i = 0; i < 4; ++i) {
                m_current.rgba[i] = static_cast<std::uint8_t>(value >> (i * 8));
            }
            m_current.q = as_float(gs::bits(value, 32, 32));
            return;
        case gs::kSt:
            m_current.s = as_float(gs::bits(value, 0, 32));
            m_current.t = as_float(gs::bits(value, 32, 32));
            return;
        case gs::kUv:
            m_current.u = gs::bits(value, 0, 14);
            m_current.v = gs::bits(value, 16, 14);
            return;
        case gs::kXyzf2:
        case gs::kXyzf3:
            m_current.x = gs::bits(value, 0, 16);
            m_current.y = gs::bits(value, 16, 16);
            m_current.z = gs::bits(value, 32, 24);
            m_current.fog = static_cast<std::uint8_t>(gs::bits(value, 56, 8));
            kick(address == gs::kXyzf2);
            return;
        case gs::kXyz2:
        case gs::kXyz3:
            m_current.x = gs::bits(value, 0, 16);
            m_current.y = gs::bits(value, 16, 16);
            m_current.z = gs::bits(value, 32, 32);
            kick(address == gs::kXyz2);
            return;
        case gs::kFog:
            m_current.fog = static_cast<std::uint8_t>(gs::bits(value, 56, 8));
            return;
        case gs::kTex0_1:
            c1.tex0 = gs::Tex0::decode(value);
            break;
        case gs::kTex0_2:
            c2.tex0 = gs::Tex0::decode(value);
            break;
        case gs::kTex2_1:
        case gs::kTex2_2: {
            // TEX2 rewrites TEX0's format and CLUT fields, nothing else.
            Context& c = address == gs::kTex2_1 ? c1 : c2;
            const gs::Tex0 t = gs::Tex0::decode(value);
            c.tex0.psm = t.psm;
            c.tex0.cbp = t.cbp;
            c.tex0.cpsm = t.cpsm;
            c.tex0.csm = t.csm;
            c.tex0.csa = t.csa;
            c.tex0.cld = t.cld;
            break;
        }
        case gs::kTex1_1:
            c1.tex1 = gs::Tex1::decode(value);
            break;
        case gs::kTex1_2:
            c2.tex1 = gs::Tex1::decode(value);
            break;
        case gs::kClamp1:
            c1.clamp = gs::Clamp::decode(value);
            break;
        case gs::kClamp2:
            c2.clamp = gs::Clamp::decode(value);
            break;
        case gs::kXyoffset1:
            c1.xyoffset = gs::Xyoffset::decode(value);
            break;
        case gs::kXyoffset2:
            c2.xyoffset = gs::Xyoffset::decode(value);
            break;
        case gs::kScissor1:
            c1.scissor = gs::Scissor::decode(value);
            break;
        case gs::kScissor2:
            c2.scissor = gs::Scissor::decode(value);
            break;
        case gs::kAlpha1:
            c1.alpha = gs::Alpha::decode(value);
            break;
        case gs::kAlpha2:
            c2.alpha = gs::Alpha::decode(value);
            break;
        case gs::kTest1:
            c1.test = gs::Test::decode(value);
            break;
        case gs::kTest2:
            c2.test = gs::Test::decode(value);
            break;
        case gs::kFrame1:
            c1.frame = gs::Frame::decode(value);
            m_targets.push_back(c1.frame.fbp * 32);
            if (is_offscreen(c1.frame)) {
                if (std::find(m_offscreen.begin(), m_offscreen.end(), c1.frame.fbp * 32) == m_offscreen.end()) {
                    m_offscreen.push_back(c1.frame.fbp * 32);
                }
            } else if (std::none_of(m_frame_buffers.begin(), m_frame_buffers.end(), [&](const FrameBuffer& f) {
                    return f.block == c1.frame.fbp * 32;
                })) {
                m_frame_buffers.push_back({c1.frame.fbp * 32, c1.frame.fbw});
            }
            break;
        case gs::kFrame2:
            c2.frame = gs::Frame::decode(value);
            m_targets.push_back(c2.frame.fbp * 32);
            if (is_offscreen(c2.frame)) {
                if (std::find(m_offscreen.begin(), m_offscreen.end(), c2.frame.fbp * 32) == m_offscreen.end()) {
                    m_offscreen.push_back(c2.frame.fbp * 32);
                }
            } else if (std::none_of(m_frame_buffers.begin(), m_frame_buffers.end(), [&](const FrameBuffer& f) {
                    return f.block == c2.frame.fbp * 32;
                })) {
                m_frame_buffers.push_back({c2.frame.fbp * 32, c2.frame.fbw});
            }
            break;
        case gs::kZbuf1:
            c1.zbuf = gs::Zbuf::decode(value);
            break;
        case gs::kZbuf2:
            c2.zbuf = gs::Zbuf::decode(value);
            break;
        case gs::kPrmodecont:
            m_use_prim_attributes = (value & 1) != 0;
            break;
        case gs::kPrmode:
            m_prmode = gs::Prim::decode(value);
            break;
        case gs::kTexa:
            m_texa = gs::Texa::decode(value);
            break;
        case gs::kFogcol:
            m_fog_colour = gs::bits(value, 0, 24);
            break;
        case gs::kBitbltbuf:
            m_bitbltbuf = gs::Bitbltbuf::decode(value);
            return;
        case gs::kTrxpos:
            m_trxpos = gs::Trxpos::decode(value);
            return;
        case gs::kTrxreg:
            m_trxreg = gs::Trxreg::decode(value);
            return;
        case gs::kTrxdir: {
            const std::uint32_t direction = gs::bits(value, 0, 2);
            if (direction == 0) {
                // Host to chip: the IMAGE data that follows is the rectangle.
                m_upload = ImageUpload{
                    m_bitbltbuf.dbp,
                    m_bitbltbuf.dbw,
                    m_bitbltbuf.dpsm,
                    m_trxpos.dsax,
                    m_trxpos.dsay,
                    m_trxreg.rrw,
                    m_trxreg.rrh,
                    {}
                };
                m_upload_bytes = image_bytes(
                    m_bitbltbuf.dpsm, static_cast<int>(m_trxreg.rrw), static_cast<int>(m_trxreg.rrh)
                );
                m_upload.data.reserve(m_upload_bytes);
            } else if (direction == 2) {
                // A copy inside the chip's memory. Out of a frame buffer, it is the game taking
                // part of the frame as a texture: kept as where that texture's texels are in the
                // frame, for the draws that read it. Other copies move textures the pool does not
                // follow yet.
                int x = 0;
                int y = 0;
                std::erase_if(m_frame_copies, [&](const FrameCopy& f) { return f.dbp == m_bitbltbuf.dbp; });
                if (frame_source(m_bitbltbuf.sbp, x, y)) {
                    m_frame_copies.push_back(
                        {m_bitbltbuf.dbp,
                         x + static_cast<int>(m_trxpos.ssax) - static_cast<int>(m_trxpos.dsax),
                         y + static_cast<int>(m_trxpos.ssay) - static_cast<int>(m_trxpos.dsay)}
                    );
                } else {
                    log::debug(
                        "GS local-to-local transfer ignored (block {:#x} to {:#x})",
                        m_bitbltbuf.sbp,
                        m_bitbltbuf.dbp
                    );
                }
            }
            return;
        }
        default:
            // TEXCLUT, MIPTBP, TEXFLUSH, DIMX, DTHE, COLCLAMP, PABE, FBA,
            // SCANMSK, HWREG, SIGNAL, FINISH, LABEL: nothing a draw call
            // depends on yet (colour is always clamped; dithering is off).
            return;
    }
    m_state_dirty = true;
}

void GifInterpreter::image_data(std::span<const std::uint8_t> data) {
    if (m_upload_bytes == 0) {
        log::debug("GIF IMAGE data without a transfer ({} bytes)", data.size());
        return;
    }
    const std::size_t take = std::min(data.size(), m_upload_bytes - m_upload.data.size());
    m_upload.data.insert(
        m_upload.data.end(), data.begin(), data.begin() + static_cast<std::ptrdiff_t>(take)
    );
    if (m_upload.data.size() >= m_upload_bytes) {
        if (g_dump_draws) {
            log::info(
                "upload to block {:#x} width {} psm {:#x} at {},{} size {}x{}",
                m_upload.dbp, m_upload.dbw, m_upload.dpsm, m_upload.x, m_upload.y, m_upload.width,
                m_upload.height
            );
        }
        std::erase_if(m_frame_copies, [&](const FrameCopy& f) { return f.dbp == m_upload.dbp; });
        if (m_upload.dpsm == gs::kPsmct32 && m_upload.dbp >= g_draw_buffer_block
            && m_upload.dbp < g_draw_buffer_block + kDrawBufferWidth * (g_draw_buffer_lines / 32) * 32) {
            draw_frame_upload(m_upload);
        }
        m_textures.upload(std::move(m_upload));
        m_upload = ImageUpload{};
        m_upload_bytes = 0;
        m_state_dirty = true;  // the bound texture may be the one just replaced
    }
}

bool frame_pixel(std::uint32_t base, std::uint32_t width, int x, int y, int& frame_x, int& frame_y) {
    if (base < g_draw_buffer_block || (base - g_draw_buffer_block) % 32 != 0 || width == 0) {
        return false;
    }
    const int page = static_cast<int>((base - g_draw_buffer_block) / 32) + (y / 32) * static_cast<int>(width) + x / 64;
    frame_x = (page % static_cast<int>(kDrawBufferWidth)) * 64 + x % 64;
    frame_y = (page / static_cast<int>(kDrawBufferWidth)) * 32 + y % 32;
    return frame_y < g_draw_buffer_lines;
}

void GifInterpreter::draw_frame_upload(const ImageUpload& upload) {
    int fx = 0;
    int fy = 0;
    if (!frame_pixel(upload.dbp, upload.dbw, static_cast<int>(upload.x), static_cast<int>(upload.y), fx, fy)) {
        return;
    }
    // The pixels the port answered a read-back with: drawn from the frame as it was drawn.
    const bool snapshot =
        g_frame_snapshot.texture != 0
        && std::find(g_frame_snapshot.answered.begin(), g_frame_snapshot.answered.end(),
                     hash_bytes(upload.data.data(), upload.data.size()))
               != g_frame_snapshot.answered.end();
    if (g_dump_draws) {
        log::info("frame upload to block {:#x}: frame pixel {},{}, {} (snapshot of {} blocks)", upload.dbp, fx, fy,
                  snapshot ? "from the snapshot" : "its own pixels", g_frame_snapshot.answered.size());
    }
    DirectState s;
    s.textured = true;
    s.snapshot = snapshot;
    if (!snapshot) {
        // The transfer itself, as a texture of its own size at its block.
        gs::Tex0 t{};
        t.tbp0 = upload.dbp;
        t.tbw = std::max<std::uint32_t>(upload.dbw, 1);
        t.psm = gs::kPsmct32;
        t.tw = static_cast<std::uint32_t>(std::bit_width(std::max<std::uint32_t>(upload.width, 1) - 1));
        t.th = static_cast<std::uint32_t>(std::bit_width(std::max<std::uint32_t>(upload.height, 1) - 1));
        t.tcc = false;
        // Keyed past the chip's 14-bit blocks, so that the rectangle stays one texture.
        ImageUpload copy = upload;
        copy.dbp = 0x10000u + upload.dbp;
        copy.x = 0;
        copy.y = 0;
        m_textures.upload(std::move(copy));
        t.tbp0 = 0x10000u + upload.dbp;
        s.texture = m_textures.resolve(t, m_texa);
    }
    s.tfx = gs::Tfx::Decal;
    s.tcc = false;
    s.clamp_s = true;
    s.clamp_t = true;
    s.depth_write = false;
    s.scissor = {0, static_cast<std::uint32_t>(m_config.screen_width - 1), 0,
                 static_cast<std::uint32_t>(m_config.screen_height - 1)};
    m_draws.push_back({s, static_cast<std::uint32_t>(m_vertices.size()), 0});
    const float tw = static_cast<float>(std::bit_ceil(std::max<std::uint32_t>(upload.width, 1)));
    const float th = static_cast<float>(std::bit_ceil(std::max<std::uint32_t>(upload.height, 1)));
    // One quad per run of a page row: 64 pixels across at most, 32 lines down at most.
    for (std::uint32_t y0 = 0; y0 < upload.height; y0 = (y0 / 32 + 1) * 32) {
        const std::uint32_t y1 = std::min(upload.height, (y0 / 32 + 1) * 32);
        for (std::uint32_t x0 = 0; x0 < upload.width; x0 = (x0 / 64 + 1) * 64) {
            const std::uint32_t x1 = std::min(upload.width, (x0 / 64 + 1) * 64);
            if (!frame_pixel(upload.dbp, upload.dbw, static_cast<int>(upload.x + x0),
                             static_cast<int>(upload.y + y0), fx, fy)) {
                continue;
            }
            const float px0 = static_cast<float>(fx);
            const float py0 = static_cast<float>(fy);
            const float px1 = px0 + static_cast<float>(x1 - x0);
            const float py1 = py0 + static_cast<float>(y1 - y0);
            DirectVertex corners[4]{};
            const float xs[2] = {px0, px1};
            const float ys[2] = {py0, py1};
            const float us[2] = {static_cast<float>(x0), static_cast<float>(x1)};
            const float vs[2] = {static_cast<float>(y0), static_cast<float>(y1)};
            for (int k = 0; k < 4; ++k) {
                DirectVertex& v = corners[k];
                const int i = k & 1;
                const int j = k >> 1;
                v.x = xs[i] / static_cast<float>(m_config.screen_width) * 2.0f - 1.0f;
                // The draw buffer fills the picture whatever its height (416 lines in a 60 Hz mode).
                v.y = 1.0f - ys[j] / static_cast<float>(g_draw_buffer_lines) * 2.0f;
                v.z = -1.0f;
                if (snapshot) {
                    v.s = xs[i] / static_cast<float>(m_config.screen_width);
                    v.t = 1.0f - ys[j] / static_cast<float>(g_draw_buffer_lines);
                } else {
                    v.s = us[i] / tw;
                    v.t = vs[j] / th;
                }
                v.q = 1.0f;
                v.rgba[0] = v.rgba[1] = v.rgba[2] = v.rgba[3] = 0x80;
                v.fog = 1.0f;
            }
            const DirectVertex order[6] = {corners[0], corners[1], corners[2], corners[1], corners[3], corners[2]};
            m_vertices.insert(m_vertices.end(), order, order + 6);
            m_draws.back().count += 6;
        }
    }
    m_state_dirty = true;  // the next primitive's state is its own again
}

// A quad of the 2D layer: as a textured, blended triangle strip with texel coordinates through its
// four corners, its texture and colour, drawn from the port's own record.
void GifInterpreter::native_quad(const UiQuad& quad) {
    write_register(gs::kTex0_1, quad.tex0);
    const bool sprite = (quad.flags & kUiSprite) != 0;
    write_prim(sprite ? 0x156 : 0x154);
    m_glyph = (quad.flags & kUiGlyph) != 0;
    m_in_native = true;
    for (int i = 0; i < 4; ++i) {
        if (sprite && (i == 1 || i == 2)) {
            continue;
        }
        m_current.rgba[0] = static_cast<std::uint8_t>(quad.rgba);
        m_current.rgba[1] = static_cast<std::uint8_t>(quad.rgba >> 8);
        m_current.rgba[2] = static_cast<std::uint8_t>(quad.rgba >> 16);
        m_current.rgba[3] = static_cast<std::uint8_t>(quad.rgba >> 24);
        m_current.q = 1.0f;
        m_current.u = quad.u[i];
        m_current.v = quad.v[i];
        m_current.x = quad.x[i];
        m_current.y = quad.y[i];
        m_current.z = quad.z;
        kick(true);
    }
    m_in_native = false;
    if (m_glyph) {
        m_glyph = false;
        m_state_dirty = true;
    }
}

// OPENRAC_UI_RESIDUAL: each kind of 2D primitive still drawn from the game's packets (not a native
// quad), once: what is left to move off the GS packets.
static void log_residual(const gs::Prim& prim, const gs::Tex0& tex0, bool native) {
    static const bool on = std::getenv("OPENRAC_UI_RESIDUAL") != nullptr;
    if (!on || native) {
        return;
    }
    static std::vector<std::uint64_t> seen;
    const std::uint64_t key = static_cast<std::uint64_t>(prim.kind) | (static_cast<std::uint64_t>(prim.tme) << 4)
                              | (static_cast<std::uint64_t>(prim.tme ? tex0.tbp0 : 0) << 8);
    if (std::find(seen.begin(), seen.end(), key) != seen.end()) {
        return;
    }
    seen.push_back(key);
    log::info("ui residual: prim {} textured {} tbp {:#x} psm {:#x}", static_cast<int>(prim.kind), prim.tme ? 1 : 0,
              prim.tme ? tex0.tbp0 : 0u, prim.tme ? tex0.psm : 0u);
}

void GifInterpreter::kick(bool draw) {
    if (draw) {
        log_residual(attributes(), context().tex0, m_in_native);
    }
    m_queue[m_queued++] = m_current;
    if (g_dump_draws && m_prim.kind != gs::PrimKind::Sprite) {
        const gs::Alpha& al = context().alpha;
        log::info(
            "prim {} ({},{}) uv ({},{}) stq ({},{},{}) fst {} rgba {:02x}{:02x}{:02x}{:02x} tme {} psm {:#x} cbp {:#x} tbw {} tbp {:#x} tw {} th {} abe {} alpha {}{}{}{} fix {:#x} fbp {:#x} fbmsk {:#x} test {}",
            static_cast<int>(m_prim.kind), m_current.x / 16.0, m_current.y / 16.0, m_current.u / 16.0, m_current.v / 16.0, m_current.s, m_current.t, m_current.q, attributes().fst, m_current.rgba[0],
            m_current.rgba[1], m_current.rgba[2], m_current.rgba[3], attributes().tme,
            context().tex0.psm, context().tex0.cbp, context().tex0.tbw, context().tex0.tbp0, context().tex0.width(), context().tex0.height(), attributes().abe,
            al.a, al.b, al.c, al.d, al.fix, context().frame.fbp, context().frame.fbmsk, test_text(context().test)
        );
    }
    switch (m_prim.kind) {
        case gs::PrimKind::Point:
            if (draw) {
                emit_point(m_queue[0]);
            }
            m_queued = 0;
            break;
        case gs::PrimKind::Line:
            if (m_queued == 2) {
                if (draw) {
                    emit_line(m_queue[0], m_queue[1]);
                }
                m_queued = 0;
            }
            break;
        case gs::PrimKind::LineStrip:
            if (m_queued == 2) {
                if (draw) {
                    emit_line(m_queue[0], m_queue[1]);
                }
                m_queue[0] = m_queue[1];
                m_queued = 1;
            }
            break;
        case gs::PrimKind::Triangle:
            if (m_queued == 3) {
                if (draw) {
                    emit_triangle(m_queue[0], m_queue[1], m_queue[2], m_queue[2]);
                }
                m_queued = 0;
            }
            break;
        case gs::PrimKind::TriangleStrip:
            if (m_queued == 3) {
                if (draw) {
                    emit_triangle(m_queue[0], m_queue[1], m_queue[2], m_queue[2]);
                }
                m_queue[0] = m_queue[1];
                m_queue[1] = m_queue[2];
                m_queued = 2;
            }
            break;
        case gs::PrimKind::TriangleFan:
            if (m_queued == 3) {
                if (draw) {
                    emit_triangle(m_queue[0], m_queue[1], m_queue[2], m_queue[2]);
                }
                m_queue[1] = m_queue[2];
                m_queued = 2;
            }
            break;
        case gs::PrimKind::Sprite:
            if (m_queued == 2) {
                if (draw) {
                    emit_sprite(m_queue[0], m_queue[1]);
                }
                m_queued = 0;
            }
            break;
        case gs::PrimKind::Invalid:
            m_queued = 0;
            break;
    }
}

bool GifInterpreter::is_offscreen(const gs::Frame& f) const {
    const std::uint32_t block = f.fbp * 32;
    return static_cast<int>(f.fbw) * 64 < m_config.screen_width
        || (block != m_config.display_block && block != g_draw_buffer_block);
}

std::uint32_t GifInterpreter::offscreen_target() const {
    const gs::Frame& f = context().frame;
    return is_offscreen(f) ? f.fbp * 32 : 0;
}

bool GifInterpreter::frame_source(std::uint32_t tbp, int& x, int& y) const {
    for (const FrameCopy& f : m_frame_copies) {
        if (f.dbp == tbp) {
            x = f.x;
            y = f.y;
            return true;
        }
    }
    for (const FrameBuffer& f : m_frame_buffers) {
        // A frame buffer is rows of pages (64 x 32 pixels, 32 blocks each), `width` pages a row:
        // a texture that starts on one of its page rows is the frame from that row down.
        const std::uint32_t row = std::max<std::uint32_t>(f.width, 1) * 32;
        if (tbp >= f.block && tbp < f.block + row * 16 && (tbp - f.block) % row == 0) {
            x = 0;
            y = static_cast<int>((tbp - f.block) / row * 32);
            return true;
        }
    }
    return false;
}

void GifInterpreter::screen_position(const GsVertex& v, float& x, float& y) const {
    // Each buffer the games draw into is centred on the chip's coordinate 2048 (its XYOFFSET is
    // 2048 minus half its size), and each fills the whole picture: on PAL the 448-line draw buffer
    // is stretched into the 512-line display buffer, where the text and menus are drawn after the
    // copy, and the TV shows all 512 lines at 4:3. So a buffer's own pixels map onto the picture by
    // its own size, taken from its offset.
    const Context& c = context();
    const float ofx = static_cast<float>(c.xyoffset.ofx) / 16.0f;
    const float ofy = static_cast<float>(c.xyoffset.ofy) / 16.0f;
    if (offscreen_target() != 0) {
        // An off-screen target keeps its own pixels (convert maps them onto its square).
        x = static_cast<float>(v.x) / 16.0f - ofx;
        y = static_cast<float>(v.y) / 16.0f - ofy;
        return;
    }
    const float width = 2.0f * (2048.0f - ofx);
    const float height = 2.0f * (2048.0f - ofy);
    const float sw = static_cast<float>(m_config.screen_width);
    const float sh = static_cast<float>(m_config.screen_height);
    x = static_cast<float>(v.x) / 16.0f - ofx;
    y = static_cast<float>(v.y) / 16.0f - ofy;
    if (width >= 64.0f && width <= 1024.0f) {
        x *= sw / width;
    }
    if (height >= 64.0f && height <= 1024.0f) {
        y *= sh / height;
    }
}

DirectVertex GifInterpreter::convert(const GsVertex& v) const {
    const Context& c = context();
    DirectVertex out{};
    float px = 0.0f;
    float py = 0.0f;
    screen_position(v, px, py);
    // The chip samples a pixel at its integer coordinates, GL at the pixel's centre: the shader
    // moves every vertex a quarter of a pixel of the target on (pixel_offset), so that at the
    // console's size a rectangle from 0 to 511.5 covers pixels 0 to 511 as it does on the console,
    // and no pixel's centre lies on an edge. Half a pixel of the game's picture instead put the 2D
    // layer up to 2.5 pixels off the world at 2560 across (a line of the world showed at the top
    // and left of the vendor's monitors); half a pixel of the target put the first row of an
    // off-screen panel on its clear's edge, and it was never cleared.
    const bool offscreen = offscreen_target() != 0;
    const float target_w = offscreen ? static_cast<float>(kOffscreenPixels) : static_cast<float>(m_config.screen_width);
    const float target_h = offscreen ? static_cast<float>(kOffscreenPixels) : static_cast<float>(m_config.screen_height);
    out.x = px / target_w * 2.0f - 1.0f;
    out.y = 1.0f - py / target_h * 2.0f;
    // The chip's depth, bigger nearer, straight into GL's [0, 1] window
    // depth: with the buffer cleared to 0 and GEQUAL, as on the console.
    const double depth = std::clamp(static_cast<double>(v.z) / c.zbuf.max_z(), 0.0, 1.0);
    out.z = static_cast<float>(depth * 2.0 - 1.0);
    int frame_x = 0;
    int frame_y = 0;
    if (attributes().tme
        && std::find(m_offscreen.begin(), m_offscreen.end(), c.tex0.tbp0) != m_offscreen.end()) {
        // A texel of an off-screen target: its pixel, over the target's square (first row on top,
        // as it was drawn).
        const float u = attributes().fst ? static_cast<float>(v.u) / 16.0f
                                         : v.s / v.q * static_cast<float>(c.tex0.width());
        const float w = attributes().fst ? static_cast<float>(v.v) / 16.0f
                                         : v.t / v.q * static_cast<float>(c.tex0.height());
        out.s = u / static_cast<float>(kOffscreenPixels);
        out.t = 1.0f - w / static_cast<float>(kOffscreenPixels);
        out.q = 1.0f;
    } else if (attributes().tme && is_colour_format(c.tex0.psm) && frame_source(c.tex0.tbp0, frame_x, frame_y)) {
        // A texel of the frame: its pixel, over the frame's size; the copy of the render target
        // the draw samples has its first row at the bottom, as GL keeps it.
        const float u = attributes().fst ? static_cast<float>(v.u) / 16.0f
                                         : v.s / v.q * static_cast<float>(c.tex0.width());
        const float w = attributes().fst ? static_cast<float>(v.v) / 16.0f
                                         : v.t / v.q * static_cast<float>(c.tex0.height());
        out.s = (u + static_cast<float>(frame_x)) / static_cast<float>(m_config.screen_width);
        out.t = 1.0f - (w + static_cast<float>(frame_y)) / static_cast<float>(m_config.screen_height);
        out.q = 1.0f;
    } else if (attributes().fst) {
        // Texel coordinates in 12.4 fixed point, over the texture's size.
        out.s = static_cast<float>(v.u) / 16.0f / static_cast<float>(c.tex0.width());
        out.t = static_cast<float>(v.v) / 16.0f / static_cast<float>(c.tex0.height());
        out.q = 1.0f;
    } else {
        out.s = v.s;
        out.t = v.t;
        out.q = v.q;
    }
    std::memcpy(out.rgba, v.rgba, 4);
    out.fog = static_cast<float>(v.fog) / 255.0f;
    return out;
}

void GifInterpreter::ensure_draw() {
    if (m_state_dirty) {
        const gs::Prim& p = attributes();
        const Context& c = context();
        DirectState s;
        s.textured = p.tme;
        int frame_x = 0;
        int frame_y = 0;
        s.target = offscreen_target();
        s.source = s.textured && std::find(m_offscreen.begin(), m_offscreen.end(), c.tex0.tbp0) != m_offscreen.end()
                       ? c.tex0.tbp0
                       : 0;
        s.frame_source = s.source == 0 && s.textured && is_colour_format(c.tex0.psm)
                         && frame_source(c.tex0.tbp0, frame_x, frame_y);
        if (s.source != 0 || s.frame_source) {
            s.tcc = c.tex0.tcc;
            s.tfx = c.tex0.tfx;
            s.linear_mag = c.tex1.mmag;
            s.linear_min = c.tex1.mmin == 1 || c.tex1.mmin == 4 || c.tex1.mmin == 5;
            s.clamp_s = true;
            s.clamp_t = true;
        } else if (s.textured) {
            s.texture = m_textures.resolve(c.tex0, m_texa);
            if (m_glyph && s.texture != m_textures.placeholder()) {
                s.field = m_textures.distance_field(s.texture);
                s.distance_field = s.field != s.texture;
            }
            s.texture_full_alpha = m_textures.alpha_scale(s.texture) == AlphaScale::Full;
            s.tcc = c.tex0.tcc;
            s.tfx = c.tex0.tfx;
            s.linear_mag = c.tex1.mmag;
            s.linear_min = c.tex1.mmin == 1 || c.tex1.mmin == 4 || c.tex1.mmin == 5;
            if (s.distance_field) {
                s.linear_mag = true;
                s.linear_min = true;
            }
            // Region clamp and repeat have no GL equivalent; clamp is the
            // closer of the two for the 2D path.
            s.clamp_s = c.clamp.wms != gs::Wrap::Repeat;
            s.clamp_t = c.clamp.wmt != gs::Wrap::Repeat;
        }
        s.blend = p.abe;
        if (s.blend) {
            s.alpha = c.alpha;
        }
        s.test = c.test;
        s.depth_write = !c.zbuf.zmsk;
        s.fbmsk = c.frame.fbmsk;
        // A texture nothing has been put at yet (the game draws before its upload arrives): the
        // chip would show whatever its memory held, so the draw writes nothing, rather than the
        // placeholder's magenta (OPENRAC_SHOW_MISSING shows it, for finding missing textures). A
        // draw of an off-screen target or of the frame has no texture of the pool (its handle is
        // the placeholder's 0) and is drawn.
        static const bool show_missing = std::getenv("OPENRAC_SHOW_MISSING") != nullptr;
        if (s.textured && s.source == 0 && !s.frame_source && s.texture == m_textures.placeholder() && !show_missing) {
            s.fbmsk = 0xFFFFFFFFu;
            s.depth_write = false;
        }
        // The scissor is in this buffer's own pixels: scaled onto the picture as its draws are
        // (screen_position), so it clips where they land.
        {
            const float ofx = static_cast<float>(c.xyoffset.ofx) / 16.0f;
            const float ofy = static_cast<float>(c.xyoffset.ofy) / 16.0f;
            const float width = 2.0f * (2048.0f - ofx);
            const float height = 2.0f * (2048.0f - ofy);
            const float kx = width >= 64.0f && width <= 1024.0f ? static_cast<float>(m_config.screen_width) / width : 1.0f;
            const float ky = height >= 64.0f && height <= 1024.0f ? static_cast<float>(m_config.screen_height) / height : 1.0f;
            auto scale = [](std::uint32_t v, float k, bool end) {
                const float r = end ? (static_cast<float>(v) + 1.0f) * k - 1.0f : static_cast<float>(v) * k;
                return static_cast<std::uint32_t>(r < 0.0f ? 0.0f : r);
            };
            s.scissor = s.target != 0 ? c.scissor
                                      : gs::Scissor{scale(c.scissor.x0, kx, false), scale(c.scissor.x1, kx, true),
                                                    scale(c.scissor.y0, ky, false), scale(c.scissor.y1, ky, true)};
        }
        s.fog = p.fge;
        if (s.fog) {
            s.fog_colour = m_fog_colour;
        }
        m_state = s;
        m_state_dirty = false;
    }
    if (m_draws.empty() || !(m_draws.back().state == m_state) || m_draws.back().mobys != 0) {
        m_draws.push_back({m_state, static_cast<std::uint32_t>(m_vertices.size()), 0});
    }
}

void GifInterpreter::emit_triangle(
    const GsVertex& a, const GsVertex& b, const GsVertex& c, const GsVertex& colour_from
) {
    ensure_draw();
    DirectVertex v[3] = {convert(a), convert(b), convert(c)};
    if (!attributes().iip) {
        // Flat shading: the colour of the vertex that completed the primitive.
        for (DirectVertex& x : v) {
            std::memcpy(x.rgba, colour_from.rgba, 4);
        }
    }
    m_vertices.insert(m_vertices.end(), v, v + 3);
    m_draws.back().count += 3;
}

void GifInterpreter::emit_quad(const DirectVertex corners[4]) {
    ensure_draw();
    // Corners 0 1 / 2 3: two triangles.
    const DirectVertex order[6] =
        {corners[0], corners[1], corners[2], corners[1], corners[3], corners[2]};
    m_vertices.insert(m_vertices.end(), order, order + 6);
    m_draws.back().count += 6;
}

void GifInterpreter::emit_sprite(const GsVertex& a, const GsVertex& b) {
    if (g_dump_draws) {
        const gs::Alpha& al = context().alpha;
        const gs::Scissor& sc = context().scissor;
        log::info(
            "sprite ({},{})-({},{}) rgba {:02x}{:02x}{:02x}{:02x} tme {} psm {:#x} cbp {:#x} tbw {} tbp {:#x} abe {} alpha {}{}{}{} fix {:#x} scissor {}-{}x{}-{} fbp {:#x} fbmsk {:#x} test {}",
            a.x / 16.0, a.y / 16.0, b.x / 16.0, b.y / 16.0, b.rgba[0], b.rgba[1], b.rgba[2], b.rgba[3],
            attributes().tme, context().tex0.psm, context().tex0.cbp, context().tex0.tbw, context().tex0.tbp0, attributes().abe, al.a, al.b, al.c, al.d, al.fix,
            sc.x0, sc.x1, sc.y0, sc.y1, context().frame.fbp, context().frame.fbmsk, test_text(context().test)
        );
    }
    // An untextured sprite as tall as a whole-frame scissor fills the frame (a clear, in strips, or a
    // full-screen fade). The game draws those before its world; the port's world renderers draw
    // before the 2D path, which would put the fill on top. The frame is cleared by the renderer,
    // so fills are left out until the direct path is ordered with the world buckets. A sprite
    // clipped to a smaller scissor is an ordinary rectangle and is drawn, and so is one blended
    // at less than full strength: a tint over the finished frame (the title's blue wash, a fade
    // part way). The game also clears with blending on at full alpha, which replaces the frame
    // all the same: that is a clear.
    const gs::Alpha& al = context().alpha;
    const bool standard = al.a == 0 && al.b == 1 && al.d == 1;  // (Cs - Cd) * C + Cd
    const bool replaces = !attributes().abe
                          || (standard && ((al.c == 0 && b.rgba[3] >= 0x80) || (al.c == 2 && al.fix >= 0x80)));
    // (Only in the frame: an off-screen panel's clear is drawn, however tall the panel. The
    // Load Game page's slot list is 512 lines: its clear left old highlight frames in place.)
    if (!attributes().tme && replaces && offscreen_target() == 0) {
        const gs::Scissor& sc = context().scissor;
        const std::int64_t scissor_height = static_cast<std::int64_t>(sc.y1) - static_cast<std::int64_t>(sc.y0);
        const std::int64_t height = std::llabs(static_cast<std::int64_t>(b.y) - static_cast<std::int64_t>(a.y)) / 16;
        if (scissor_height >= kFrameFillLines && height + 2 >= scissor_height) {
            return;
        }
    }
    // The copy of the finished draw buffer into the display buffer, in strips: stretched in the menus,
    // 1:1 between borders in play. The picture already is that frame (both buffers map onto it by
    // their sizes), so drawing the copy would only shrink it into the middle of itself.
    // The same for a copy of the draw buffer onto itself in strips, which the game makes in its
    // 448-line (60 Hz) mode: the picture already holds it, and the pool has no texture for it,
    // so drawn it laid stripes of stale texels over the world.
    const std::uint32_t target = context().frame.fbp * 32;
    if (attributes().tme && context().tex0.tbp0 == g_draw_buffer_block
        && (target == m_config.display_block || target == g_draw_buffer_block)
        && std::llabs(static_cast<std::int64_t>(b.y) - static_cast<std::int64_t>(a.y)) / 16 >= kFrameFillLines) {
        return;
    }
    // A sprite is a rectangle from two corners; depth, colour and fog come
    // from the second vertex. Texture coordinates go to S/Q, T/Q per corner
    // (a sprite has no perspective).
    DirectVertex p = convert(a);
    DirectVertex q = convert(b);
    const float s0 = p.s / p.q;
    const float t0 = p.t / p.q;
    const float s1 = q.s / q.q;
    const float t1 = q.t / q.q;
    DirectVertex corners[4];
    for (int i = 0; i < 4; ++i) {
        corners[i] = q;
        corners[i].q = 1.0f;
    }
    corners[0].x = p.x;
    corners[0].y = p.y;
    corners[0].s = s0;
    corners[0].t = t0;
    corners[1].x = q.x;
    corners[1].y = p.y;
    corners[1].s = s1;
    corners[1].t = t0;
    corners[2].x = p.x;
    corners[2].y = q.y;
    corners[2].s = s0;
    corners[2].t = t1;
    corners[3].x = q.x;
    corners[3].y = q.y;
    corners[3].s = s1;
    corners[3].t = t1;
    emit_quad(corners);
}

void GifInterpreter::emit_line(const GsVertex& a, const GsVertex& b) {
    // A line is a quad one pixel wide (GL core profile draws no wide lines,
    // and one draw call takes triangles only).
    float ax = 0.0f;
    float ay = 0.0f;
    float bx = 0.0f;
    float by = 0.0f;
    screen_position(a, ax, ay);
    screen_position(b, bx, by);
    float nx = -(by - ay);
    float ny = bx - ax;
    const float length = std::sqrt(nx * nx + ny * ny);
    if (length > 0.0f) {
        nx = nx / length * 0.5f;
        ny = ny / length * 0.5f;
    } else {
        nx = 0.5f;
        ny = 0.0f;
    }
    const float dx = nx / static_cast<float>(m_config.screen_width) * 2.0f;
    const float dy = -ny / static_cast<float>(m_config.screen_height) * 2.0f;
    DirectVertex p = convert(a);
    DirectVertex q = convert(b);
    if (!attributes().iip) {
        std::memcpy(p.rgba, q.rgba, 4);
    }
    DirectVertex corners[4] = {p, p, q, q};
    corners[0].x += dx;
    corners[0].y += dy;
    corners[1].x -= dx;
    corners[1].y -= dy;
    corners[2].x += dx;
    corners[2].y += dy;
    corners[3].x -= dx;
    corners[3].y -= dy;
    emit_quad(corners);
}

void GifInterpreter::emit_point(const GsVertex& a) {
    DirectVertex p = convert(a);
    const float w = 2.0f / static_cast<float>(m_config.screen_width);
    const float h = 2.0f / static_cast<float>(m_config.screen_height);
    DirectVertex corners[4] = {p, p, p, p};
    corners[1].x += w;
    corners[2].y -= h;
    corners[3].x += w;
    corners[3].y -= h;
    emit_quad(corners);
}

// ---------------------------------------------------------------------------
// DirectRenderer

GlBlend translate_blend(const gs::Alpha& alpha) {
    auto coefficient = [&](std::uint8_t which, int& k, int& m) {
        k = (alpha.a == which ? 1 : 0) - (alpha.b == which ? 1 : 0);
        m = alpha.d == which ? 1 : 0;
    };
    int ks = 0;
    int ms = 0;
    int kd = 0;
    int md = 0;
    coefficient(0, ks, ms);  // Cs
    coefficient(1, kd, md);  // Cd

    GlBlend out;
    GLenum factor = GL_SRC_ALPHA;
    GLenum one_minus = GL_ONE_MINUS_SRC_ALPHA;
    if (alpha.c == 1) {
        factor = GL_DST_ALPHA;
        one_minus = GL_ONE_MINUS_DST_ALPHA;
    } else if (alpha.c == 2) {
        factor = GL_CONSTANT_ALPHA;
        one_minus = GL_ONE_MINUS_CONSTANT_ALPHA;
        out.constant = std::min(1.0f, static_cast<float>(alpha.fix) / 128.0f);
    }
    // One term's factor and sign.
    auto term = [&](int k, int m, GLenum& f, int& sign) {
        sign = 1;
        if (k == 0) {
            f = m != 0 ? GL_ONE : GL_ZERO;
        } else if (k == 1 && m == 0) {
            f = factor;
        } else if (k == -1 && m == 1) {
            f = one_minus;
        } else if (k == -1 && m == 0) {
            f = factor;
            sign = -1;
        } else {  // C + 1: more than GL's factors reach
            f = GL_ONE;
            out.exact = false;
        }
    };
    int src_sign = 1;
    int dst_sign = 1;
    term(ks, ms, out.src, src_sign);
    term(kd, md, out.dst, dst_sign);
    if (src_sign > 0 && dst_sign > 0) {
        out.equation = GL_FUNC_ADD;
    } else if (src_sign < 0 && dst_sign > 0) {
        out.equation = GL_FUNC_REVERSE_SUBTRACT;  // dst - src
    } else if (src_sign > 0 && dst_sign < 0) {
        out.equation = GL_FUNC_SUBTRACT;  // src - dst
    } else {
        // Both negative: the result is never above 0, and colour clamps.
        out.src = GL_ZERO;
        out.dst = GL_ZERO;
        out.equation = GL_FUNC_ADD;
    }
    return out;
}

namespace {

GLenum depth_func(gs::DepthTest test) {
    switch (test) {
        case gs::DepthTest::Never:
            return GL_NEVER;
        case gs::DepthTest::Always:
            return GL_ALWAYS;
        case gs::DepthTest::Gequal:
            return GL_GEQUAL;
        case gs::DepthTest::Greater:
            return GL_GREATER;
    }
    return GL_ALWAYS;
}

}  // namespace

DirectRenderer::DirectRenderer(std::string name, Bucket bucket, DirectConfig config, Input input)
    : BucketRenderer(std::move(name), bucket),
      m_config(config),
      m_input(input) {}

DirectRenderer::~DirectRenderer() = default;

bool DirectRenderer::init(RenderState& state, std::string& error) {
    m_interpreter = std::make_unique<GifInterpreter>(state.textures, m_config);
    if (!m_shader.build("direct", shaders::direct_vert, shaders::direct_frag, error)) {
        return false;
    }
    m_uniforms.textured = m_shader.uniform("textured");
    m_uniforms.tcc = m_shader.uniform("tcc");
    m_uniforms.tfx = m_shader.uniform("tfx");
    m_uniforms.tex_alpha_scale = m_shader.uniform("tex_alpha_scale");
    m_uniforms.fog_enable = m_shader.uniform("fog_enable");
    m_uniforms.fog_colour = m_shader.uniform("fog_colour");
    m_uniforms.alpha_test = m_shader.uniform("alpha_test");
    m_uniforms.alpha_ref = m_shader.uniform("alpha_ref");
    m_uniforms.alpha_keep_failing = m_shader.uniform("alpha_keep_failing");
    m_uniforms.distance_field = m_shader.uniform("distance_field");
    m_uniforms.tex_T1 = m_shader.uniform("tex_T1");
    m_uniforms.pixel_offset = m_shader.uniform("pixel_offset");

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    constexpr auto stride = static_cast<GLsizei>(sizeof(DirectVertex));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(DirectVertex, x))
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(DirectVertex, s))
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        4,
        GL_UNSIGNED_BYTE,
        GL_TRUE,
        stride,
        reinterpret_cast<const void*>(offsetof(DirectVertex, rgba))
    );
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, 1, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const void*>(offsetof(DirectVertex, fog))
    );
    glBindVertexArray(0);
    return true;
}

void DirectRenderer::release() {
    m_shader.release();
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_frame_copy != 0) {
        glDeleteTextures(1, &m_frame_copy);
        m_frame_copy = 0;
    }
    for (auto& [block, o] : m_offscreen_targets) {
        (void)block;
        glDeleteFramebuffers(1, &o.framebuffer);
        glDeleteRenderbuffers(1, &o.depth);
        glDeleteTextures(1, &o.texture);
    }
    m_offscreen_targets.clear();
}

bool DirectRenderer::submit(std::span<const std::uint8_t> packets) {
    const bool ok =
        m_input == Input::Vif ? m_interpreter->vif(packets) : m_interpreter->gif(packets);
    if (!ok && m_interpreter->error() != m_last_error) {
        m_last_error = m_interpreter->error();
        log::warn("{}: {}", name(), m_last_error);
    }
    return ok;
}

void DirectRenderer::render(const FrameInput& input, RenderState& state) {
    const auto packets = input.packets[static_cast<std::size_t>(bucket())];
    if (packets.empty()) {
        return;
    }
    m_interpreter->set_ui_quads(input.ui_quads);
    m_frame = &input;
    submit(packets);
    flush(state);
}

const DirectRenderer::Offscreen& DirectRenderer::offscreen(std::uint32_t block) {
    for (const auto& [b, o] : m_offscreen_targets) {
        if (b == block) {
            return o;
        }
    }
    const int size = kOffscreenPixels * kOffscreenScale;
    Offscreen o;
    glGenTextures(1, &o.texture);
    glBindTexture(GL_TEXTURE_2D, o.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenRenderbuffers(1, &o.depth);
    glBindRenderbuffer(GL_RENDERBUFFER, o.depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
    glGenFramebuffers(1, &o.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, o.framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, o.texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, o.depth);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_offscreen_targets.emplace_back(block, o);
    return m_offscreen_targets.back().second;
}

void DirectRenderer::apply(const DirectState& s, RenderState& render_state) {
    // Where the draw goes: an off-screen target's square, or the frame.
    if (s.target != 0) {
        const Offscreen& o = offscreen(s.target);
        glBindFramebuffer(GL_FRAMEBUFFER, o.framebuffer);
        glViewport(0, 0, kOffscreenPixels * kOffscreenScale, kOffscreenPixels * kOffscreenScale);
        const float side = static_cast<float>(kOffscreenPixels * kOffscreenScale);
        glUniform2f(m_uniforms.pixel_offset, 0.5f / side, -0.5f / side);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, render_state.target.framebuffer);
        glViewport(0, 0, render_state.target.width, render_state.target.height);
        glUniform2f(m_uniforms.pixel_offset, 0.5f / static_cast<float>(std::max(render_state.target.width, 1)),
                    -0.5f / static_cast<float>(std::max(render_state.target.height, 1)));
    }
    glUniform1i(m_uniforms.textured, s.textured ? 1 : 0);
    if (s.textured) {
        glActiveTexture(GL_TEXTURE0);
        if (s.snapshot) {
            glBindTexture(GL_TEXTURE_2D, g_frame_snapshot.texture);
        } else if (s.source != 0) {
            // A panel the game rendered off-screen, as the chip would read it from its memory.
            glBindTexture(GL_TEXTURE_2D, offscreen(s.source).texture);
        } else if (s.frame_source) {
            // What the frame holds now, as the chip would read it from its memory.
            const int w = render_state.target.width;
            const int h = render_state.target.height;
            if (m_frame_copy == 0) {
                glGenTextures(1, &m_frame_copy);
            }
            glBindTexture(GL_TEXTURE_2D, m_frame_copy);
            if (w != m_frame_copy_width || h != m_frame_copy_height) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                m_frame_copy_width = w;
                m_frame_copy_height = h;
            }
            // (A multisampled frame is resolved first: readable() leaves the bindings changed.)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, readable(render_state.target));
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, w, h);
            glBindFramebuffer(GL_FRAMEBUFFER,
                              s.target != 0 ? offscreen(s.target).framebuffer : render_state.target.framebuffer);
        } else {
            glBindTexture(GL_TEXTURE_2D, render_state.textures.gl_texture(s.texture));
        }
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            static_cast<GLint>(s.linear_mag ? GL_LINEAR : GL_NEAREST)
        );
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            static_cast<GLint>(s.linear_min ? GL_LINEAR : GL_NEAREST)
        );
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            static_cast<GLint>(s.clamp_s ? GL_CLAMP_TO_EDGE : GL_REPEAT)
        );
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            static_cast<GLint>(s.clamp_t ? GL_CLAMP_TO_EDGE : GL_REPEAT)
        );
        glUniform1i(m_uniforms.tcc, s.tcc ? 1 : 0);
        glUniform1i(m_uniforms.tfx, static_cast<GLint>(s.tfx));
        glUniform1f(m_uniforms.tex_alpha_scale, s.texture_full_alpha ? 0.5f : 1.0f);
        glUniform1i(m_uniforms.distance_field, s.distance_field ? 1 : 0);
        if (s.distance_field) {
            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, render_state.textures.gl_texture(s.field));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glUniform1i(m_uniforms.tex_T1, 1);
            glActiveTexture(GL_TEXTURE0);
        }
    }
    glUniform1i(m_uniforms.fog_enable, s.fog ? 1 : 0);
    glUniform3f(
        m_uniforms.fog_colour,
        static_cast<float>(s.fog_colour & 0xFF),
        static_cast<float>((s.fog_colour >> 8) & 0xFF),
        static_cast<float>((s.fog_colour >> 16) & 0xFF)
    );

    if (s.blend) {
        const GlBlend b = translate_blend(s.alpha);
        if (!b.exact) {
            log::debug(
                "blend mode A={} B={} C={} D={} approximated",
                s.alpha.a,
                s.alpha.b,
                s.alpha.c,
                s.alpha.d
            );
        }
        glEnable(GL_BLEND);
        glBlendEquation(b.equation);
        // The chip writes the source alpha as it is; it never blends alpha.
        glBlendFuncSeparate(b.src, b.dst, GL_ONE, GL_ZERO);
        glBlendColor(0.0f, 0.0f, 0.0f, b.constant);
    } else {
        glDisable(GL_BLEND);
    }

    if (!s.test.zte) {
        glDisable(GL_DEPTH_TEST);
    } else {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(depth_func(s.test.ztst));
    }

    // The scissor, from the chip's pixels to the target's: the picture's 512 x 448 onto the frame,
    // or an off-screen target's square onto its texture.
    const int target_width = s.target != 0 ? kOffscreenPixels * kOffscreenScale : render_state.target.width;
    const int target_height = s.target != 0 ? kOffscreenPixels * kOffscreenScale : render_state.target.height;
    const float sx = static_cast<float>(target_width)
                     / static_cast<float>(s.target != 0 ? kOffscreenPixels : m_config.screen_width);
    const float sy = static_cast<float>(target_height)
                     / static_cast<float>(s.target != 0 ? kOffscreenPixels : m_config.screen_height);
    auto x0 = static_cast<GLint>(std::floor(static_cast<float>(s.scissor.x0) * sx));
    auto x1 = static_cast<GLint>(std::ceil(static_cast<float>(s.scissor.x1 + 1) * sx));
    // On a target wider than the picture, the picture's scissor goes with the picture, to the
    // target's middle (flush() says which triangles are the whole target's instead).
    const float k = render_state.target.picture_scale_x;
    if (s.target == 0 && k < 1.0f && !m_whole_target) {
        const float middle = 0.5f * static_cast<float>(target_width);
        x0 = static_cast<GLint>(std::floor(middle + (static_cast<float>(s.scissor.x0) * sx - middle) * k));
        x1 = static_cast<GLint>(std::ceil(middle + (static_cast<float>(s.scissor.x1 + 1) * sx - middle) * k));
        // The HUD at the target's sides (RenderTarget::wide_2d): the television's whole width is
        // the target's whole width.
        if (render_state.target.wide_2d && s.scissor.x0 <= 0
            && s.scissor.x1 + 1 >= static_cast<int>(m_config.screen_width)) {
            x0 = 0;
            x1 = target_width;
        }
    }
    const auto y0 = static_cast<GLint>(std::floor(static_cast<float>(s.scissor.y0) * sy));
    const auto y1 = static_cast<GLint>(std::ceil(static_cast<float>(s.scissor.y1 + 1) * sy));
    glEnable(GL_SCISSOR_TEST);
    glScissor(x0, target_height - y1, std::max(0, x1 - x0), std::max(0, y1 - y0));
}

void DirectRenderer::draw(const DirectDraw& d, RenderState& render_state) {
    const DirectState& s = d.state;
    if (d.mobys != 0) {
        // Mobys the game drew into this target: the frontend draws them, with the target bound;
        // then this renderer's own state again.
        if (m_frame != nullptr && m_frame->target_mobys && s.target != 0) {
            const Offscreen& o = offscreen(s.target);
            glBindFramebuffer(GL_FRAMEBUFFER, o.framebuffer);
            m_frame->target_mobys(d.mobys, o.framebuffer, kOffscreenPixels * kOffscreenScale);
            glDisable(GL_CULL_FACE);
            glDisable(GL_SCISSOR_TEST);
            m_shader.use();
            glBindVertexArray(m_vao);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            glActiveTexture(GL_TEXTURE0);
        }
        return;
    }
    apply(s, render_state);

    // FRAME.FBMSK: a channel whose bits are all masked is not written.
    const GLboolean red = (s.fbmsk & 0x000000FFu) != 0x000000FFu ? GL_TRUE : GL_FALSE;
    const GLboolean green = (s.fbmsk & 0x0000FF00u) != 0x0000FF00u ? GL_TRUE : GL_FALSE;
    const GLboolean blue = (s.fbmsk & 0x00FF0000u) != 0x00FF0000u ? GL_TRUE : GL_FALSE;
    const GLboolean alpha = (s.fbmsk & 0xFF000000u) != 0xFF000000u ? GL_TRUE : GL_FALSE;
    const GLboolean depth = s.depth_write && !m_background ? GL_TRUE : GL_FALSE;

    auto call = [&](int test,
                    bool keep_failing,
                    GLboolean colour,
                    GLboolean write_alpha,
                    GLboolean write_depth) {
        glUniform1i(m_uniforms.alpha_test, test);
        glUniform1f(m_uniforms.alpha_ref, static_cast<float>(s.test.aref));
        glUniform1i(m_uniforms.alpha_keep_failing, keep_failing ? 1 : 0);
        glColorMask(red & colour, green & colour, blue & colour, alpha & write_alpha);
        glDepthMask(depth & write_depth);
        glDrawArrays(GL_TRIANGLES, static_cast<GLint>(d.first), static_cast<GLsizei>(d.count));
        ++render_state.stats.draw_calls;
        render_state.stats.triangles += d.count / 3;
    };

    const bool testing = s.test.ate && s.test.atst != gs::AlphaTest::Always;
    if (!testing) {
        call(0, false, GL_TRUE, GL_TRUE, GL_TRUE);
        return;
    }
    // What a pixel that fails still writes: colour, depth, alpha.
    GLboolean fail_colour = GL_FALSE;
    GLboolean fail_alpha = GL_FALSE;
    GLboolean fail_depth = GL_FALSE;
    switch (s.test.afail) {
        case gs::AlphaFail::Keep:
            break;
        case gs::AlphaFail::FbOnly:
            fail_colour = GL_TRUE;
            fail_alpha = GL_TRUE;
            break;
        case gs::AlphaFail::ZbOnly:
            fail_depth = GL_TRUE;
            break;
        case gs::AlphaFail::RgbOnly:
            fail_colour = GL_TRUE;
            break;
    }
    if (s.test.atst == gs::AlphaTest::Never) {
        // Every pixel fails: one draw with the fail mode's writes (the
        // game's quads use NEVER with FB_ONLY for colour without depth).
        if (s.test.afail != gs::AlphaFail::Keep) {
            call(0, false, fail_colour, fail_alpha, fail_depth);
        }
        return;
    }
    // OpenGOAL's double draw: the passing pixels with every write, then, if
    // failing ones still write something, those with only that.
    call(static_cast<int>(s.test.atst), false, GL_TRUE, GL_TRUE, GL_TRUE);
    if (s.test.afail != gs::AlphaFail::Keep) {
        call(static_cast<int>(s.test.atst), true, fail_colour, fail_alpha, fail_depth);
    }
}

void DirectRenderer::flush(RenderState& state, bool background) {
    m_background = background;
    const auto& vertices = m_interpreter->vertices();
    if (vertices.empty()) {
        m_interpreter->clear();
        return;
    }
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    const std::size_t bytes = vertices.size() * sizeof(DirectVertex);
    // A fresh store each frame, so the driver need not wait on the last one.
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(std::max(bytes, m_vbo_bytes)),
        nullptr,
        GL_STREAM_DRAW
    );
    const float k = state.target.picture_scale_x;
    // On a target wider than the game's picture: each draw as runs of triangles that are the
    // picture's (in the target's middle) or the whole target's, in the order they came.
    struct Run {
        const DirectDraw* draw;
        std::uint32_t first, count;
        bool whole;
    };
    std::vector<Run> runs;
    if (k < 1.0f) {
        // The picture goes to the target's middle, at its own shape, and is cut at its own sides
        // as the television's edge cut it (apply()'s scissor). A triangle is moved whole, so
        // nothing is stretched, except what covered the whole screen: a plain fill that spans the
        // picture's whole width (a fade, a clear, a letterbox bar) and the frame itself drawn
        // again. Those are the whole target's. A texture of the pool across the whole width (a
        // loading picture's band) keeps its shape in the middle.
        std::vector<DirectVertex> placed(vertices.begin(), vertices.end());
        for (const DirectDraw& d : m_interpreter->draws()) {
            const DirectState& s = d.state;
            if (s.target != 0 || d.count == 0) {
                if (d.count != 0 || d.mobys != 0) {
                    runs.push_back({&d, d.first, d.count, true});  // an off-screen panel's own square
                }
                continue;
            }
            // The frame itself, read back or read now, is sent in strips that together are the
            // whole screen: each is its part of the whole target.
            const bool of_frame = s.snapshot || s.frame_source;
            for (std::uint32_t i = d.first; i + 3 <= d.first + d.count && i + 3 <= placed.size(); i += 3) {
                const float lo = std::min({placed[i].x, placed[i + 1].x, placed[i + 2].x});
                const float hi = std::max({placed[i].x, placed[i + 1].x, placed[i + 2].x});
                // A plain fill of the screen's whole width, or of its whole height (the game clears
                // its frame in strips 32 pixels wide): its part of the whole target.
                const float lo_y = std::min({placed[i].y, placed[i + 1].y, placed[i + 2].y});
                const float hi_y = std::max({placed[i].y, placed[i + 1].y, placed[i + 2].y});
                const bool whole = of_frame
                    || (!s.textured && ((lo <= -0.995f && hi >= 0.995f) || (lo_y <= -0.995f && hi_y >= 0.995f)));
                // What lies along one side of the picture (the black line the game draws down the
                // screen's first column) was at the television's edge; in the middle of a wider
                // target it would be a line through the world, and it is left out.
                constexpr float kEdge = 1.0f - 3.0f / 512.0f;
                if (!s.textured && ((lo <= -kEdge && hi <= -kEdge) || (lo >= kEdge && hi >= kEdge))) {
                    continue;
                }
                if (!runs.empty() && runs.back().draw == &d && runs.back().whole == whole) {
                    runs.back().count += 3;
                } else {
                    runs.push_back({&d, i, 3, whole});
                }
                if (whole) {
                    continue;
                }
                for (std::uint32_t j = i; j < i + 3; ++j) {
                    placed[j].x *= k;
                }
            }
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(bytes), placed.data());
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(bytes), vertices.data());
    }
    m_vbo_bytes = std::max(bytes, m_vbo_bytes);
    m_shader.use();
    if (k < 1.0f) {
        for (const Run& r : runs) {
            m_whole_target = r.whole;
            draw(DirectDraw{r.draw->state, r.first, r.count, r.draw->mobys}, state);
        }
        m_whole_target = true;
    } else {
        for (const DirectDraw& d : m_interpreter->draws()) {
            if (d.count != 0 || d.mobys != 0) {
                draw(d, state);
            }
        }
    }
    // Leave GL as the next renderer expects it.
    glBindFramebuffer(GL_FRAMEBUFFER, state.target.framebuffer);
    glViewport(0, 0, state.target.width, state.target.height);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GEQUAL);
    glBindVertexArray(0);
    m_interpreter->clear();
}

}  // namespace openrac::renderer
