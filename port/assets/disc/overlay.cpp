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

        // Jump and jump-and-link carry a 26-bit target that moves with the code, so only the opcode
        // is kept.
        if (o == 2 || o == 3) {
            return {w & 0xfc000000, std::nullopt};  // j / jal: the target
        }

        // A lui loads a high half, which the walk tracks per register to pair it with a later low
        // half.
        if (o == 15) {
            const std::size_t r = rt(w);

            // A lui into register 0 writes nothing.
            if (r == 0) {
                return {w, std::nullopt};
            }

            // A lui whose value is an address is masked; its high half is remembered for the next
            // instruction.
            if (lui_is_address(w)) {
                hi[r] = w & 0xffff;
                return {w & 0xffff0000, std::nullopt};
            }

            hi[r].reset();
            return {w, std::nullopt};
        }

        // A SPECIAL instruction: only moves and register arithmetic can carry a tracked high half
        // along.
        if (o == 0) {
            const u32 f = w & 63;
            const std::size_t s = rs(w), t = rt(w), d = rd(w);
            // move (addu / daddu / or with $zero) keeps the state; any other
            // write clears it.
            std::optional<u32> moved;

            // The or, addu and daddu forms (0x25, 0x21, 0x2d) copy a register when one operand is
            // zero.
            if (f == 0x21 || f == 0x2d || f == 0x25) {
                // With a zero second operand the first register is copied, high half and all.
                if (t == 0) {
                    moved = hi[s];
                } else if (s == 0) {
                    moved = hi[t];
                }
            }

            // The destination now holds the copy, or nothing known if it was overwritten.
            if (d != 0) {
                hi[d] = moved;
            }

            return {w, std::nullopt};
        }

        // A branch writes no register that is tracked.
        if (is_branch(o)) {
            return {w, std::nullopt};
        }

        // A coprocessor instruction may write a general register.
        if (o >= 0x10 && o <= 0x13) {
            // Coprocessor moves to a GPR (mfc, dmfc, qmfc, cfc: rs 0..2) write rt.
            if (rs(w) <= 2 && rt(w) != 0) {
                hi[rt(w)].reset();
            }

            return {w, std::nullopt};
        }

        // The multimedia instructions write rd and nothing tracked survives there.
        if (o == 0x1c) {
            // Register 0 is never written.
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

        // Register 28 is the global pointer, so its offset forms a full address from a known base.
        if (s == 28) {
            full = gp + sext(w);
        } else if (hi[s]) {
            full = (*hi[s] << 16) + lo;
        }

        const u32 masked = full ? (w & 0xffff0000) : w;

        // An instruction that writes rt makes any high half held there stale.
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

    // Walk the code in order, carrying register state forward.
    for (std::size_t i = 0; i < words.size(); ++i) {
        const u32 w = words[i];

        // A branch seen earlier may have a target here, whose state is merged in.
        if (const auto p = pending.find(i); p != pending.end()) {
            // Fill in each register the walk does not know from the branch's state.
            for (std::size_t r = 0; r < 32; ++r) {
                // Known state wins over the branch's.
                if (!st.hi[r]) {
                    st.hi[r] = p->second.hi[r];
                }
            }

            pending.erase(p);
        }

        out.push_back(st.step(w, gp));

        // After a branch's delay slot, its state reaches the target.
        if (branch_to) {
            // The delay slot has run: the branch's state reaches its target.
            HiState& e = pending[*branch_to];

            // Merge into the target's state each register it does not know yet.
            for (std::size_t r = 0; r < 32; ++r) {
                // Known state wins over the new one.
                if (!e.hi[r]) {
                    e.hi[r] = st.hi[r];
                }
            }

            branch_to.reset();
        }

        const u32 o = op(w);

        // Branches, including the coprocessor-condition ones, may skip ahead.
        if (is_branch(o) || (o == 0x11 && rs(w) == 8)) {
            const s64 t = static_cast<s64>(i) + 1 + static_cast<s16>(w & 0xffff);

            // Only forward targets inside the code are tracked.
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

    // Each section has a 16-byte header: destination, size, kind and entry; read until they run
    // out.
    while (pos + 16 <= overlay.size()) {
        const u32 dest = overlay.u32_at(pos);
        const u32 size = overlay.u32_at(pos + 4);
        const u32 kind = overlay.u32_at(pos + 8);
        const u32 entry = overlay.u32_at(pos + 12);

        // All sections of an overlay share one entry point; a different one starts something else.
        if (!out.empty() && out.front().entry != entry) {
            break;
        }

        out.push_back(
            {dest, kind, entry, overlay.sub(pos + 16, size, "overlay section").to_vector()}
        );
        pos += 16 + std::size_t{size};
    }

    // An overlay with no sections is not an overlay.
    if (out.empty()) {
        fail("the overlay has no sections");
    }

    return out;
}

std::optional<ByteView> read_overlay(
    std::span<const OverlaySection> sections, u32 address, std::size_t length
) {
    // Find the section that holds the address.
    for (const OverlaySection& s : sections) {
        // Skip kind 8 sections and sections that start after the address.
        if (s.kind == 8 || address < s.dest) {
            continue;
        }

        const std::size_t off = address - s.dest;

        // The range must lie wholly inside the section.
        if (off <= s.data.size() && length <= s.data.size() - off) {
            return ByteView(s.data).sub(off, length);
        }
    }

    return std::nullopt;
}

std::vector<u32> mask_code(std::span<const u32> words, u32 gp) {
    std::vector<u32> out;

    // Keep each masked word.
    for (const auto& [m, a] : walk(words, gp)) {
        out.push_back(m);
    }

    return out;
}

std::vector<std::pair<std::size_t, u32>> address_refs(std::span<const u32> words, u32 gp) {
    std::vector<std::pair<std::size_t, u32>> out;
    const auto w = walk(words, gp);

    // Collect each word that carries an address reference.
    for (std::size_t i = 0; i < w.size(); ++i) {
        // Only some words carry a reference.
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
    // The text section is optional in a short overlay.
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

    // Every jal marks a function start at its target.
    for (const u32 w : m_text) {
        // Opcode 3 is jal.
        if (op(w) == 3) {
            const u32 a = (w & 0x03ffffff) << 2;

            // Only targets inside the text count.
            if (inside(a)) {
                m_starts.insert(a);
            }
        }
    }

    // The virtual-method table lists update functions, which are function starts too.
    for (const VtblEntry& e : vtbl()) {
        // Only entries inside the text count.
        if (inside(e.update)) {
            m_starts.insert(e.update);
        }
    }

    // A frame set-up right after a return (jr ra + delay slot, then addiu
    // sp, sp, -n), and code addresses formed with lui/%lo (callbacks).
    for (std::size_t p = 2; p < m_text.size(); ++p) {
        const u32 w = m_text[p];

        // A function starts after a return and the stack pointer adjustment that ends the previous
        // one.
        if (m_text[p - 2] == 0x03e00008 && w >> 16 == 0x27bd && (w & 0x8000) != 0) {
            m_starts.insert(m_text_start + 4 * static_cast<u32>(p));
        }
    }

    // Addresses the code takes by reference are function starts when they lie in the text and are
    // word-aligned.
    for (const auto& [i, a] : address_refs(m_text, m_gp)) {
        // The address must be inside the text and word-aligned to start a function.
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

    // An address outside every section has no word.
    if (!b) {
        return std::nullopt;
    }

    return b->u32_at(0);
}

std::optional<std::span<const u32>> LevelOverlay::code(u32 address, std::size_t count) const {
    // The address must be inside the text and word-aligned.
    if (address < m_text_start || (address - m_text_start) % 4 != 0) {
        return std::nullopt;
    }

    const std::size_t i = (address - m_text_start) / 4;

    // The range must lie wholly inside the text.
    if (i > m_text.size() || count > m_text.size() - i) {
        return std::nullopt;
    }

    return std::span<const u32>(m_text).subspan(i, count);
}

std::vector<VtblEntry> LevelOverlay::vtbl() const {
    std::vector<VtblEntry> out;

    // An overlay without a vtable section has no entries.
    if (m_sections.size() <= kVtblSection) {
        return out;
    }

    const ByteView d(m_sections[kVtblSection].data);

    // Read 12-byte entries until the terminator.
    for (std::size_t at = 0; at + 12 <= d.size(); at += 12) {
        const VtblEntry e{d.s32_at(at), d.u32_at(at + 4), d.u32_at(at + 8)};

        // A class of -1 ends the table.
        if (e.o_class == -1) {
            break;
        }

        out.push_back(e);
    }

    return out;
}

std::vector<CamVtblEntry> LevelOverlay::camvtbl() const {
    std::vector<CamVtblEntry> out;

    // An overlay without a camera table section has no entries.
    if (m_sections.size() <= kCamVtblSection) {
        return out;
    }

    const ByteView d(m_sections[kCamVtblSection].data);

    // Read 0x14-byte entries until the terminator.
    for (std::size_t at = 0; at + 0x14 <= d.size(); at += 0x14) {
        const CamVtblEntry e{
            d.s32_at(at), d.u32_at(at + 4), d.u32_at(at + 8), d.u32_at(at + 12), d.u32_at(at + 16)
        };

        // A class of -1 ends the table.
        if (e.klass == -1) {
            break;
        }

        out.push_back(e);
    }

    return out;
}

std::optional<std::size_t> LevelOverlay::extent(u32 address) const {
    const u32 end = text_end();

    // The address must be inside the text.
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

    // Combine the words into one hash value.
    for (const u32 v : k) {
        h = h * 1000003u ^ v;
    }

    return h;
}

Relocation::Relocation(const LevelOverlay& reference, const LevelOverlay& target)
    : m_reference(&reference),
      m_target(&target),
      m_identity(reference.same_text(target)) {
    // The same text needs no search; every address maps to itself.
    if (m_identity) {
        return;
    }

    // All of the target's .text, indexed by the masked key of every 4 words.
    const u32 base = target.text_start();
    const std::span<const u32> all = target.text();

    // Index every four-word window of the target's text by its masked contents.
    for (std::size_t p = 0; p + 4 <= all.size(); ++p) {
        m_index[key_of(all.subspan(p, 4), target.gp())].push_back(base + 4 * static_cast<u32>(p));
    }
}

std::optional<std::vector<u32>> Relocation::reference_masked(u32 ref_fn) const {
    const auto n = m_reference->extent(ref_fn);

    // A function with no known extent cannot be matched.
    if (!n) {
        return std::nullopt;
    }

    const auto code = m_reference->code(ref_fn, *n);

    // A function with no code cannot be matched.
    if (!code) {
        return std::nullopt;
    }

    return mask_code(*code, m_reference->gp());
}

bool Relocation::same_code(u32 ref_fn, u32 target_fn) const {
    // The same text means the same function address.
    if (m_identity) {
        return ref_fn == target_fn;
    }

    const auto m = reference_masked(ref_fn);

    // A function with no code cannot match.
    if (!m) {
        return false;
    }

    const auto c = m_target->code(target_fn, m->size());
    return c && mask_code(*c, m_target->gp()) == *m;
}

std::vector<u32> Relocation::copies(u32 ref_fn) const {
    // With the same text, a function copies only itself.
    if (m_identity) {
        // The function must exist in the text.
        if (m_reference->code(ref_fn, 1)) {
            return {ref_fn};
        }

        return {};
    }

    const auto m = reference_masked(ref_fn);

    // A match needs the first four words as a key.
    if (!m || m->size() < 4) {
        return {};
    }

    const auto it = m_index.find({(*m)[0], (*m)[1], (*m)[2], (*m)[3]});

    // No function in the target starts with these four words.
    if (it == m_index.end()) {
        return {};
    }

    std::vector<u32> out;

    // Check every candidate against the whole function.
    for (const u32 c : it->second) {
        const auto w = m_target->code(c, m->size());

        // A candidate is a copy when all its masked words equal the reference's.
        if (w && mask_code(*w, m_target->gp()) == *m) {
            out.push_back(c);
        }
    }

    return out;
}

std::optional<u32> Relocation::function(u32 ref_fn) const {
    // The same text means the same function address.
    if (m_identity) {
        return ref_fn;
    }

    const auto c = copies(ref_fn);

    // A function maps only when exactly one copy exists in the target.
    if (c.size() != 1) {
        return std::nullopt;
    }

    return c.front();
}

const std::unordered_map<u32, std::vector<std::pair<u32, std::size_t>>>& Relocation::xrefs() const {
    // Build the cross-reference table on first use and keep it.
    if (!m_xrefs) {
        m_xrefs.emplace();

        // Scan every function in the reference for address references.
        for (const u32 f : m_reference->function_starts()) {
            const auto n = m_reference->extent(f);
            const auto code = n ? m_reference->code(f, *n) : std::nullopt;

            // A function with no code has no references.
            if (!code) {
                continue;
            }

            // Record each reference by its address, with the function and word it sits in.
            for (const auto& [off, a] : address_refs(*code, m_reference->gp())) {
                (*m_xrefs)[a].emplace_back(f, off);
            }
        }
    }

    return *m_xrefs;
}

std::optional<u32> Relocation::data(u32 ref_address) const {
    // The same text means the same data address.
    if (m_identity) {
        return ref_address;
    }

    const auto& x = xrefs();
    const auto sites = x.find(ref_address);

    // An address no function refers to cannot be found this way.
    if (sites == x.end()) {
        return std::nullopt;
    }

    // Use each function that refers to the address.
    for (const auto& [f, off] : sites->second) {
        const auto g = function(f);
        const auto n = m_reference->extent(f);

        // Skip functions that have no counterpart or no known extent.
        if (!g || !n) {
            continue;
        }

        const auto code = m_target->code(*g, *n);

        // A counterpart with no code cannot be compared.
        if (!code) {
            continue;
        }

        // Find the same word's reference in the counterpart.
        for (const auto& [o, a] : address_refs(*code, m_target->gp())) {
            // The word in the same place holds the counterpart's address.
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

        // A function with no known extent cannot be matched.
        if (!extent) {
            continue;
        }

        const std::size_t n = std::min(off + 16, *extent);
        const auto code = m_reference->code(f, n);

        // A function with no code cannot be matched.
        if (!code) {
            continue;
        }

        const auto m = mask_code(*code, m_reference->gp());

        // A match needs the first four words as a key.
        if (m.size() < 4) {
            continue;
        }

        const auto it = m_index.find({m[0], m[1], m[2], m[3]});

        // No function in the target starts with these four words.
        if (it == m_index.end()) {
            continue;
        }

        std::vector<u32> hits;

        // Check every candidate against the whole function.
        for (const u32 c : it->second) {
            const auto w = m_target->code(c, n);

            // A candidate matches when all its masked words equal the reference's.
            if (w && mask_code(*w, m_target->gp()) == m) {
                hits.push_back(c);
            }
        }

        // Only a single match gives an unambiguous counterpart.
        if (hits.size() == 1) {
            // Compare each reference in the counterpart with the reference's.
            for (const auto& [o, a] : address_refs(*m_target->code(hits[0], n), m_target->gp())) {
                // The reference at the same word is the counterpart's address.
                if (o == off) {
                    return a;
                }
            }
        }
    }

    return std::nullopt;
}

}  // namespace openrac::assets::disc
