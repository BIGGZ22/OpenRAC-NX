// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// Every texture the renderer draws with, converted to RGBA8 and uploaded to
// the GPU once. After OpenGOAL's TexturePool (OPENGOAL_NOTES.md, section 5.2),
// with its central rule: a texture's base pointer in the chip's memory (TBP)
// is an identifier, not an address. Ratchet & Clank streams each pass's
// textures into the same area of that memory (RENDERER.md, section 5), so
// the pool never keeps an image of the memory; it keeps, per base pointer, what
// the game last put there:
//
//   - a texture converted ahead of time, from the assets extracted from the
//     player's disc (add() then place(): the way the game's texture paging
//     will hand the renderer its textures), or
//   - the pixels of an image transfer the game sent itself (upload(): the 2D
//     path's fonts and HUD images, sent in GIF IMAGE packets).
//
// A draw asks for a texture by its TEX0 register (resolve()); the pool finds
// what is at that base pointer, converts it with the CLUT at the CLUT pointer,
// and caches the result by content, so a texture the game sends every frame
// is converted once.

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "renderer/gs.h"
#include "renderer/texture.h"

namespace openrac::renderer {

// A rectangle the game sent to the chip with an image transfer.
struct ImageUpload {
    std::uint32_t dbp = 0;  // destination base, in 256-byte blocks
    std::uint32_t dbw = 1;  // destination buffer width, in 64-pixel units
    std::uint8_t dpsm = gs::kPsmct32;
    std::uint32_t x = 0;  // destination rectangle
    std::uint32_t y = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<std::uint8_t> data;  // raster order, in dpsm
};

// A converted texture, valid for the life of the pool (or until end_frame()
// drops it after a long time unused). 0 is never a texture.
using TextureHandle = std::uint32_t;

class TexturePool {
public:
    TexturePool();
    ~TexturePool();
    TexturePool(const TexturePool&) = delete;
    TexturePool& operator=(const TexturePool&) = delete;

    // ---- What the game puts in the chip's memory ----

    // An image transfer. It replaces whatever was last uploaded at the same
    // base pointer.
    void upload(ImageUpload upload);

    // A texture converted ahead of time. `scale` says how its alpha runs;
    // `mipmaps` gives its GL texture a mipmap chain and trilinear filtering
    // (3D geometry), otherwise it has one level, filtered as each draw says (nearest until then;
    // `linear`: bilinear, as the GS samples a texture whose TEX1 has MXL 0 and linear filters).
    TextureHandle add(Rgba8Image image, AlphaScale scale, std::string name, bool mipmaps = false,
                      bool linear = false);

    // The anisotropic filter of every texture with a mipmap chain (the world's): 1 is none, up to
    // what the driver offers (max_anisotropy(); 1 when it has no such filter). The textures on the
    // GPU now are set again; later ones get it as they are uploaded. The context must be current.
    void set_anisotropy(float samples);
    float anisotropy() const { return m_anisotropy; }
    static float max_anisotropy();

    // The game has put `texture` at base pointer `tbp`; draws that sample
    // there get it, whatever their TEX0 says about format and CLUT.
    void place(std::uint32_t tbp, TextureHandle texture);
    void clear_placements();

    // A TEX0 with CLD set has loaded the CLUT at its CBP into the chip's CLUT buffer: the draws
    // after it read their palette from what was there then, even after the game puts something
    // else at that block (cld: 1 to 3 always load, 4 and 5 when CBP differs from CBP0 or CBP1,
    // which the caller decides).
    void load_clut(const gs::Tex0& tex0);

    // ---- What a draw samples ----

    // The texture TEX0 describes, converted with TEXA. Never 0: when nothing
    // is known at that base pointer, the placeholder (a checkerboard).
    TextureHandle resolve(const gs::Tex0& tex0, const gs::Texa& texa);

    TextureHandle placeholder() const { return m_placeholder; }

    // A distance field of `source` for drawing text sharp at any size, four times its size: in
    // alpha the signed distance to the glyphs' edge (0.5 on the edge, 1/8 per source texel), in red
    // the glyphs' own opacity (0..1) held up to that edge; made once per content of the source.
    TextureHandle distance_field(TextureHandle source);

    const Rgba8Image& image(TextureHandle texture) const;
    AlphaScale alpha_scale(TextureHandle texture) const;
    const std::string& name(TextureHandle texture) const;

    // ---- The GPU side; the GL context must be current ----

    // The GL texture, created on first use. Filtering and wrapping are set
    // by whoever binds it.
    unsigned gl_texture(TextureHandle texture);

    // Delete every GL texture (before the context goes away).
    void release_gl();

    // Once per frame: forget conversions of game uploads that no draw has
    // used for kKeepFrames frames.
    void end_frame();

    static constexpr std::uint32_t kKeepFrames = 120;

    // Live textures, the placeholder included.
    std::size_t size() const;

private:
    struct Key {
        std::uint32_t tbp, tbw, tw, th, cbp;
        std::uint8_t psm, cpsm;
        gs::Texa texa;
        std::uint64_t pixels_hash, clut_hash;

        bool operator==(const Key&) const = default;
    };

    struct KeyHash {
        std::size_t operator()(const Key& key) const;
    };

    struct Stored {
        ImageUpload upload;
        std::uint64_t hash = 0;
        std::uint64_t sequence = 0;  // uploads in order: the later of two over one block is there
        std::uint32_t end = 0;       // past the last block its pages reach
    };

    // A CLUT as loaded into the chip's buffer: its rectangle in its format.
    struct Clut {
        std::vector<std::uint8_t> data;
        std::uint8_t cpsm = 0;
        int width = 0;
        int entries = 0;
        std::uint64_t hash = 0;
    };

    // The upload a texture or CLUT at `block` is in: the one there, else the latest whose pages
    // reach over it (`offset` says how far into it, in blocks). Null when there is none.
    const Stored* upload_at(std::uint32_t block, std::uint32_t& offset) const;
    // The CLUT at tex0.cbp as it is in memory now; false when nothing was put there.
    bool clut_in_memory(const gs::Tex0& tex0, Clut& out) const;

    struct Entry {
        bool live = false;
        bool from_upload = false;  // converted from game uploads: may be dropped
        bool mipmaps = false;
        bool linear = false;
        Rgba8Image image;
        AlphaScale scale = AlphaScale::Gs;
        std::string name;
        unsigned gl = 0;
        std::uint64_t last_used = 0;
        Key key{};
    };

    TextureHandle new_entry();
    TextureHandle convert_upload(const gs::Tex0& tex0, const gs::Texa& texa);
    std::vector<std::uint8_t> texture_raster(const Stored& stored, const gs::Tex0& tex0) const;
    std::vector<std::uint32_t> palette(
        const gs::Tex0& tex0, const gs::Texa& texa, std::uint64_t& hash
    );

    std::vector<Entry> m_entries;  // handle - 1
    float m_anisotropy = 1.0f;
    std::vector<TextureHandle> m_free;
    std::unordered_map<std::uint32_t, Stored> m_uploads;        // by base pointer
    std::uint64_t m_sequence = 0;
    std::unordered_map<std::uint32_t, Clut> m_cluts;            // loaded, by CBP (load_clut)
    std::unordered_map<std::uint32_t, TextureHandle> m_placed;  // by base pointer
    std::unordered_map<Key, TextureHandle, KeyHash> m_converted;
    TextureHandle m_placeholder = 0;
    std::uint64_t m_frame = 0;
    std::unordered_map<std::uint64_t, TextureHandle> m_distance_fields;  // by the source's content
    // The field last given for a handle, with a sample of its texels (distance_field).
    std::unordered_map<TextureHandle, std::pair<std::uint64_t, TextureHandle>> m_field_of_handle;
};

// FNV-1a, for the cache keys.
std::uint64_t hash_bytes(const std::uint8_t* data, std::size_t size);

}  // namespace openrac::renderer
