// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Adapted from ReRAC (https://github.com/re-rac/rerac), crates/rc-formats/src/level_overlay.rs
// and crates/rc-formats/src/font.rs: ISC License, Copyright (c) 2026 ReRAC contributors.
//
// The overlay's sections, the MIPS code walk that masks relocated fields, and
// the relocator.

#include "assets/disc/overlay.h"

#include <algorithm>
#include <cstring>

namespace openrac::assets::disc {

namespace {

constexpr u32 op(u32 w) {
    return w >> 26;
}

constexpr std::size_t rs(u32 w) {
    return (w >> 21) & 31;
}

constexpr std::size_t rt(u32 w) {
    return (w >> 16) & 31;
}

constexpr std::size_t rd(u32 w) {
    return (w >> 11) & 31;
}

constexpr u32 sext(u32 w) {
    return static_cast<u32>(static_cast<s32>(static_cast<s16>(w & 0xffff)));
}

// Branches: relative offsets, not relocated.
constexpr bool is_branch(u32 o) {
    return o == 1 || (o >= 4 && o <= 7) || (o >= 0x14 && o <= 0x17);
}

// I-type instructions that write rt.
constexpr bool writes_rt(u32 o) {
    return (o >= 8 && o <= 15) || (o >= 0x18 && o <= 0x1b) || o == 0x1e || (o >= 0x20 && o <= 0x27)
           || o == 0x37;
}

// A lui of a main-RAM address: the only kind a relink changes.
constexpr bool lui_is_address(u32 w) {
    return (w & 0xffff) >= 0x0010 && (w & 0xffff) < 0x0040;
}

// Per register: "holds a lui of an address". Calls and branches do not reset
// it (a lui in a delay slot is used at the branch target); a register keeps
// its state until an instruction writes it.
struct HiState {
    std::array<std::optional<u32>, 32> hi{};

    // The masked word and the address it completes, if any.
    std::pair<u32, std::optional<u32>> step(u32 w, u32 gp) {
        const u32 o = op(w);

        if (o == 2 || o == 3) {
            return {w & 0xfc000000, std::nullopt};  // j / jal: the target
        }

        if (o == 15) {
            const std::size_t r = rt(w);

            if (r == 0) {
                return {w, std::nullopt};
            }

            if (lui_is_address(w)) {
                hi[r] = w & 0xffff;
                return {w & 0xffff0000, std::nullopt};
            }

            hi[r].reset();
            return {w, std::nullopt};
        }

        if (o == 0) {
            const u32 f = w & 63;
            const std::size_t s = rs(w), t = rt(w), d = rd(w);
            // move (addu / daddu / or with $zero) keeps the state; any other
            // write clears it.
            std::optional<u32> moved;

            if (f == 0x21 || f == 0x2d || f == 0x25) {
                if (t == 0) {
                    moved = hi[s];
                } else if (s == 0) {
                    moved = hi[t];
                }
            }

            if (d != 0) {
                hi[d] = moved;
            }

            return {w, std::nullopt};
        }

        if (is_branch(o)) {
            return {w, std::nullopt};
        }

        if (o >= 0x10 && o <= 0x13) {
            // Coprocessor moves to a GPR (mfc, dmfc, qmfc, cfc: rs 0..2) write rt.
            if (rs(w) <= 2 && rt(w) != 0) {
                hi[rt(w)].reset();
            }

            return {w, std::nullopt};
        }

        if (o == 0x1c) {
            if (rd(w) != 0) {
                hi[rd(w)].reset();  // MMI writes rd
            }

            return {w, std::nullopt};
        }

        const std::size_t s = rs(w);
        // ori completes a lui with the zero-extended immediate, the others
        // sign-extend it.
        const u32 lo = o == 13 ? (w & 0xffff) : sext(w);
        std::optional<u32> full;

        if (s == 28) {
            full = gp + sext(w);
        } else if (hi[s]) {
            full = (*hi[s] << 16) + lo;
        }

        const u32 masked = full ? (w & 0xffff0000) : w;

        if (writes_rt(o) && rt(w) != 0) {
            hi[rt(w)].reset();
        }

        return {masked, full};
    }
};

// Walks `words` from a function start in address order; at a forward branch
// target the state also takes the registers the branch brought in (after its
// delay slot), so a lui in a delay slot reaches its use.
std::vector<std::pair<u32, std::optional<u32>>> walk(std::span<const u32> words, u32 gp) {
    HiState st;
    std::map<std::size_t, HiState> pending;
    std::optional<std::size_t> branch_to;
    std::vector<std::pair<u32, std::optional<u32>>> out;
    out.reserve(words.size());

    for (std::size_t i = 0; i < words.size(); ++i) {
        const u32 w = words[i];

        if (const auto p = pending.find(i); p != pending.end()) {
            for (std::size_t r = 0; r < 32; ++r) {
                if (!st.hi[r]) {
                    st.hi[r] = p->second.hi[r];
                }
            }

            pending.erase(p);
        }

        out.push_back(st.step(w, gp));

        if (branch_to) {
            // The delay slot has run: the branch's state reaches its target.
            HiState& e = pending[*branch_to];

            for (std::size_t r = 0; r < 32; ++r) {
                if (!e.hi[r]) {
                    e.hi[r] = st.hi[r];
                }
            }

            branch_to.reset();
        }

        const u32 o = op(w);

        if (is_branch(o) || (o == 0x11 && rs(w) == 8)) {
            const s64 t = static_cast<s64>(i) + 1 + static_cast<s16>(w & 0xffff);

            if (t > static_cast<s64>(i) + 1 && static_cast<std::size_t>(t) < words.size()) {
                branch_to = static_cast<std::size_t>(t);
            }
        }
    }

    return out;
}

std::array<u32, 4> key_of(std::span<const u32> w, u32 gp) {
    const auto m = walk(w.first(4), gp);
    return {m[0].first, m[1].first, m[2].first, m[3].first};
}

}  // namespace

std::vector<OverlaySection> parse_overlay_sections(ByteView overlay) {
    std::vector<OverlaySection> out;
    std::size_t pos = 0;

    while (pos + 16 <= overlay.size()) {
        const u32 dest = overlay.u32_at(pos);
        const u32 size = overlay.u32_at(pos + 4);
        const u32 kind = overlay.u32_at(pos + 8);
        const u32 entry = overlay.u32_at(pos + 12);

        if (!out.empty() && out.front().entry != entry) {
            break;
        }

        out.push_back(
            {dest, kind, entry, overlay.sub(pos + 16, size, "overlay section").to_vector()}
        );
        pos += 16 + std::size_t{size};
    }

    if (out.empty()) {
        fail("the overlay has no sections");
    }

    return out;
}

std::optional<ByteView> read_overlay(
    std::span<const OverlaySection> sections, u32 address, std::size_t length
) {
    for (const OverlaySection& s : sections) {
        if (s.kind == 8 || address < s.dest) {
            continue;
        }

        const std::size_t off = address - s.dest;

        if (off <= s.data.size() && length <= s.data.size() - off) {
            return ByteView(s.data).sub(off, length);
        }
    }

    return std::nullopt;
}

std::vector<u32> mask_code(std::span<const u32> words, u32 gp) {
    std::vector<u32> out;

    for (const auto& [m, a] : walk(words, gp)) {
        out.push_back(m);
    }

    return out;
}

std::vector<std::pair<std::size_t, u32>> address_refs(std::span<const u32> words, u32 gp) {
    std::vector<std::pair<std::size_t, u32>> out;
    const auto w = walk(words, gp);

    for (std::size_t i = 0; i < w.size(); ++i) {
        if (w[i].second) {
            out.emplace_back(i, *w[i].second);
        }
    }

    return out;
}

LevelOverlay LevelOverlay::parse(ByteView overlay, u32 gp) {
    return LevelOverlay(parse_overlay_sections(overlay), gp);
}

LevelOverlay::LevelOverlay(std::vector<OverlaySection> sections, u32 gp)
    : m_sections(std::move(sections)),
      m_gp(gp) {
    if (m_sections.size() > kTextSection) {
        const OverlaySection& t = m_sections[kTextSection];
        m_text_start = t.dest;
        m_text.resize(t.data.size() / 4);
        std::memcpy(m_text.data(), t.data.data(), m_text.size() * 4);
    }

    const u32 end = text_end();
    auto inside = [&](u32 a) {
        return a >= m_text_start && a < end;
    };

    for (const u32 w : m_text) {
        if (op(w) == 3) {
            const u32 a = (w & 0x03ffffff) << 2;

            if (inside(a)) {
                m_starts.insert(a);
            }
        }
    }

    for (const VtblEntry& e : vtbl()) {
        if (inside(e.update)) {
            m_starts.insert(e.update);
        }
    }

    // A frame set-up right after a return (jr ra + delay slot, then addiu
    // sp, sp, -n), and code addresses formed with lui/%lo (callbacks).
    for (std::size_t p = 2; p < m_text.size(); ++p) {
        const u32 w = m_text[p];

        if (m_text[p - 2] == 0x03e00008 && w >> 16 == 0x27bd && (w & 0x8000) != 0) {
            m_starts.insert(m_text_start + 4 * static_cast<u32>(p));
        }
    }

    for (const auto& [i, a] : address_refs(m_text, m_gp)) {
        if (inside(a) && a % 4 == 0) {
            m_starts.insert(a);
        }
    }
}

std::optional<ByteView> LevelOverlay::read(u32 address, std::size_t length) const {
    return read_overlay(m_sections, address, length);
}

std::optional<u32> LevelOverlay::word(u32 address) const {
    const auto b = read(address, 4);

    if (!b) {
        return std::nullopt;
    }

    return b->u32_at(0);
}

std::optional<std::span<const u32>> LevelOverlay::code(u32 address, std::size_t count) const {
    if (address < m_text_start || (address - m_text_start) % 4 != 0) {
        return std::nullopt;
    }

    const std::size_t i = (address - m_text_start) / 4;

    if (i > m_text.size() || count > m_text.size() - i) {
        return std::nullopt;
    }

    return std::span<const u32>(m_text).subspan(i, count);
}

std::vector<VtblEntry> LevelOverlay::vtbl() const {
    std::vector<VtblEntry> out;

    if (m_sections.size() <= kVtblSection) {
        return out;
    }

    const ByteView d(m_sections[kVtblSection].data);

    for (std::size_t at = 0; at + 12 <= d.size(); at += 12) {
        const VtblEntry e{d.s32_at(at), d.u32_at(at + 4), d.u32_at(at + 8)};

        if (e.o_class == -1) {
            break;
        }

        out.push_back(e);
    }

    return out;
}

std::vector<CamVtblEntry> LevelOverlay::camvtbl() const {
    std::vector<CamVtblEntry> out;

    if (m_sections.size() <= kCamVtblSection) {
        return out;
    }

    const ByteView d(m_sections[kCamVtblSection].data);

    for (std::size_t at = 0; at + 0x14 <= d.size(); at += 0x14) {
        const CamVtblEntry e{
            d.s32_at(at), d.u32_at(at + 4), d.u32_at(at + 8), d.u32_at(at + 12), d.u32_at(at + 16)
        };

        if (e.klass == -1) {
            break;
        }

        out.push_back(e);
    }

    return out;
}

std::optional<std::size_t> LevelOverlay::extent(u32 address) const {
    const u32 end = text_end();

    if (address < m_text_start || address >= end) {
        return std::nullopt;
    }

    const auto next_it = m_starts.upper_bound(address + 3);
    const u32 next = next_it == m_starts.end() ? end : *next_it;
    const u64 limit = std::min<u64>({next, u64{address} + kMaxExtent, end});
    return static_cast<std::size_t>((limit - address) / 4);
}

bool LevelOverlay::same_text(const LevelOverlay& other) const {
    return m_text_start == other.m_text_start && m_text == other.m_text
           && m_sections == other.m_sections;
}

std::size_t Relocation::KeyHash::operator()(const Key& k) const {
    std::size_t h = 0;

    for (const u32 v : k) {
        h = h * 1000003u ^ v;
    }

    return h;
}

Relocation::Relocation(const LevelOverlay& reference, const LevelOverlay& target)
    : m_reference(&reference),
      m_target(&target),
      m_identity(reference.same_text(target)) {
    if (m_identity) {
        return;
    }

    // All of the target's .text, indexed by the masked key of every 4 words.
    const u32 base = target.text_start();
    const std::span<const u32> all = target.text();

    for (std::size_t p = 0; p + 4 <= all.size(); ++p) {
        m_index[key_of(all.subspan(p, 4), target.gp())].push_back(base + 4 * static_cast<u32>(p));
    }
}

std::optional<std::vector<u32>> Relocation::reference_masked(u32 ref_fn) const {
    const auto n = m_reference->extent(ref_fn);

    if (!n) {
        return std::nullopt;
    }

    const auto code = m_reference->code(ref_fn, *n);

    if (!code) {
        return std::nullopt;
    }

    return mask_code(*code, m_reference->gp());
}

bool Relocation::same_code(u32 ref_fn, u32 target_fn) const {
    if (m_identity) {
        return ref_fn == target_fn;
    }

    const auto m = reference_masked(ref_fn);

    if (!m) {
        return false;
    }

    const auto c = m_target->code(target_fn, m->size());
    return c && mask_code(*c, m_target->gp()) == *m;
}

std::vector<u32> Relocation::copies(u32 ref_fn) const {
    if (m_identity) {
        if (m_reference->code(ref_fn, 1)) {
            return {ref_fn};
        }

        return {};
    }

    const auto m = reference_masked(ref_fn);

    if (!m || m->size() < 4) {
        return {};
    }

    const auto it = m_index.find({(*m)[0], (*m)[1], (*m)[2], (*m)[3]});

    if (it == m_index.end()) {
        return {};
    }

    std::vector<u32> out;

    for (const u32 c : it->second) {
        const auto w = m_target->code(c, m->size());

        if (w && mask_code(*w, m_target->gp()) == *m) {
            out.push_back(c);
        }
    }

    return out;
}

std::optional<u32> Relocation::function(u32 ref_fn) const {
    if (m_identity) {
        return ref_fn;
    }

    const auto c = copies(ref_fn);

    if (c.size() != 1) {
        return std::nullopt;
    }

    return c.front();
}

const std::unordered_map<u32, std::vector<std::pair<u32, std::size_t>>>& Relocation::xrefs() const {
    if (!m_xrefs) {
        m_xrefs.emplace();

        for (const u32 f : m_reference->function_starts()) {
            const auto n = m_reference->extent(f);
            const auto code = n ? m_reference->code(f, *n) : std::nullopt;

            if (!code) {
                continue;
            }

            for (const auto& [off, a] : address_refs(*code, m_reference->gp())) {
                (*m_xrefs)[a].emplace_back(f, off);
            }
        }
    }

    return *m_xrefs;
}

std::optional<u32> Relocation::data(u32 ref_address) const {
    if (m_identity) {
        return ref_address;
    }

    const auto& x = xrefs();
    const auto sites = x.find(ref_address);

    if (sites == x.end()) {
        return std::nullopt;
    }

    for (const auto& [f, off] : sites->second) {
        const auto g = function(f);
        const auto n = m_reference->extent(f);

        if (!g || !n) {
            continue;
        }

        const auto code = m_target->code(*g, *n);

        if (!code) {
            continue;
        }

        for (const auto& [o, a] : address_refs(*code, m_target->gp())) {
            if (o == off) {
                return a;
            }
        }
    }

    // No copy of a whole referencing function (its extent runs into level
    // code): the code around one reference, from its function start to 16
    // words past it, found once in the target.
    for (const auto& [f, off] : sites->second) {
        const auto extent = m_reference->extent(f);

        if (!extent) {
            continue;
        }

        const std::size_t n = std::min(off + 16, *extent);
        const auto code = m_reference->code(f, n);

        if (!code) {
            continue;
        }

        const auto m = mask_code(*code, m_reference->gp());

        if (m.size() < 4) {
            continue;
        }

        const auto it = m_index.find({m[0], m[1], m[2], m[3]});

        if (it == m_index.end()) {
            continue;
        }

        std::vector<u32> hits;

        for (const u32 c : it->second) {
            const auto w = m_target->code(c, n);

            if (w && mask_code(*w, m_target->gp()) == m) {
                hits.push_back(c);
            }
        }

        if (hits.size() == 1) {
            for (const auto& [o, a] : address_refs(*m_target->code(hits[0], n), m_target->gp())) {
                if (o == off) {
                    return a;
                }
            }
        }
    }

    return std::nullopt;
}

}  // namespace openrac::assets::disc
