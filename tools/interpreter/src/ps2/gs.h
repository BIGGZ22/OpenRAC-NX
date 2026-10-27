// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The Graphics Synthesizer as a software model.
 *
 * It holds the GS's registers, its local memory and what a drawing kick does to that memory:
 * primitive assembly, a rasteriser for points, lines, triangles and sprites, texturing with
 * colour tables, mipmaps and filtering, fog, the alpha, destination alpha and depth tests,
 * blending, masks, transfers in, out and within local memory, and the display read-out. What the
 * model meets and does not do yet it reports once (`gstodo`) and carries on.
 *
 * Sources: the GS's registers, formats and drawing rules as publicly documented.
 */

#pragma once

#include <array>
#include <atomic>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "gs_memory.h"
#include "types.h"

namespace ps2 {

namespace gsreg {

/**
 * General register addresses, as they appear in A+D data and in the REGS
 * field of a GIF tag.
 */
enum : u8 {
    PRIM = 0x00,
    RGBAQ = 0x01,
    ST = 0x02,
    UV = 0x03,
    XYZF2 = 0x04,
    XYZ2 = 0x05,
    TEX0_1 = 0x06,
    TEX0_2 = 0x07,
    CLAMP_1 = 0x08,
    CLAMP_2 = 0x09,
    FOG = 0x0A,
    XYZF3 = 0x0C,
    XYZ3 = 0x0D,
    TEX1_1 = 0x14,
    TEX1_2 = 0x15,
    TEX2_1 = 0x16,
    TEX2_2 = 0x17,
    XYOFFSET_1 = 0x18,
    XYOFFSET_2 = 0x19,
    PRMODECONT = 0x1A,
    PRMODE = 0x1B,
    TEXCLUT = 0x1C,
    SCANMSK = 0x22,
    MIPTBP1_1 = 0x34,
    MIPTBP1_2 = 0x35,
    MIPTBP2_1 = 0x36,
    MIPTBP2_2 = 0x37,
    TEXA = 0x3B,
    FOGCOL = 0x3D,
    TEXFLUSH = 0x3F,
    SCISSOR_1 = 0x40,
    SCISSOR_2 = 0x41,
    ALPHA_1 = 0x42,
    ALPHA_2 = 0x43,
    DIMX = 0x44,
    DTHE = 0x45,
    COLCLAMP = 0x46,
    TEST_1 = 0x47,
    TEST_2 = 0x48,
    PABE = 0x49,
    FBA_1 = 0x4A,
    FBA_2 = 0x4B,
    FRAME_1 = 0x4C,
    FRAME_2 = 0x4D,
    ZBUF_1 = 0x4E,
    ZBUF_2 = 0x4F,
    BITBLTBUF = 0x50,
    TRXPOS = 0x51,
    TRXREG = 0x52,
    TRXDIR = 0x53,
    HWREG = 0x54,
    SIGNAL = 0x60,
    FINISH = 0x61,
    LABEL = 0x62,
};

}  // namespace gsreg

namespace gspriv {

/** Privileged registers, by their address on the EE bus. */
enum : u32 {
    PMODE = 0x12000000,
    SMODE1 = 0x12000010,
    SMODE2 = 0x12000020,
    SRFSH = 0x12000030,
    SYNCH1 = 0x12000040,
    SYNCH2 = 0x12000050,
    SYNCV = 0x12000060,
    DISPFB1 = 0x12000070,
    DISPLAY1 = 0x12000080,
    DISPFB2 = 0x12000090,
    DISPLAY2 = 0x120000A0,
    EXTBUF = 0x120000B0,
    EXTDATA = 0x120000C0,
    EXTWRITE = 0x120000D0,
    BGCOLOR = 0x120000E0,
    CSR = 0x12001000,
    IMR = 0x12001010,
    BUSDIR = 0x12001040,
    SIGLBLID = 0x12001080,
};

}  // namespace gspriv

namespace gstodo {

/**
 * Things the model met and does not do yet. Each is reported once.
 *
 * The bits are kept in `Gs::todo`. Nothing sets AUTO_MIP_ADDRESS or CLUT_CSM2_WIDE yet.
 */
enum : u32 {
    ANTIALIAS = 1u << 0,  // edge antialiasing, PRIM AA1
    DITHER = 1u << 1,     // dithering, DTHE
    SCANMASK = 1u << 2,   // scan line mask, SCANMSK
    AUTO_MIP_ADDRESS = 1u << 3,
    CLUT_CSM2_WIDE = 1u << 4,
    FEEDBACK_WRITE = 1u << 5,  // feedback write, EXTWRITE
};

}  // namespace gstodo

/** An image for the host: 8-bit RGBA, rows top to bottom. */
struct Image {
    /** Width in pixels. */
    int width = 0;

    /** Height in pixels. */
    int height = 0;

    /** The pixels, `width * height` of them, R in the low byte. */
    std::vector<u32> pixels;
};

/**
 * The Graphics Synthesizer as the games see it: registers, local memory and
 * what a drawing kick does to that memory. This is a software model: every
 * pixel is computed here and stored in GS memory in the frame buffer's own
 * format, so textures drawn into, read back or reused as targets behave as
 * on the machine.
 *
 * Primitives can be drawn as they arrive, or (set_threads) gathered and
 * drawn by other threads while more are gathered, each thread taking bands
 * of scan lines. The result is the same memory either way: a band's
 * primitives are drawn in their order, and the gathered ones are drawn
 * before anything reads or writes memory they use.
 *
 * One thread at a time calls the public functions; the GS does not lock for its callers. The
 * threads that `set_threads` starts are its own: they only draw batches, and the gathering side
 * waits for them (`finish`, `before_read`, `before_write`) before it touches memory they use.
 */
class Gs {
public:
    /** Makes a GS in its reset state, drawing on the caller's thread. */
    Gs();

    /** Stops the drawing threads, if any. */
    ~Gs();

    /** Puts the registers, the colour table, the transfers and the gathered primitives back. */
    void reset();

    /**
     * Chooses how many threads draw. 0: each primitive is drawn at once, by the caller.
     * 1: gathered, and drawn by the caller. More: gathered, and drawn in the
     * background by that many threads.
     *
     * Everything gathered so far is drawn first.
     *
     * @param threads Number of threads that draw.
     */
    void set_threads(unsigned threads);

    /**
     * Have everything gathered so far drawn. For whatever reads `memory` or
     * `stats` from outside.
     */
    void finish();

    /**
     * A write to a general register (from a GIF packet).
     *
     * Writing a vertex register may complete a primitive and draw it.
     *
     * @param reg The register's address, one of `gsreg`; an address past the registers is ignored.
     * @param data The 64-bit value.
     */
    void write(u8 reg, u64 data);

    /**
     * Writes a privileged register (written by the EE directly).
     *
     * @param address The register's address on the EE bus, one of `gspriv`; others are ignored.
     * @param data The 64-bit value.
     */
    void write_privileged(u32 address, u64 data);

    /**
     * Reads a privileged register.
     *
     * @param address The register's address on the EE bus, one of `gspriv`.
     * @return The register's value, or 0 for an address that is none of them.
     */
    u64 read_privileged(u32 address) const;

    /**
     * Image data of a host to local transfer (GIF IMAGE mode), in any chunking.
     *
     * Data that arrives with no transfer active is dropped.
     *
     * @param data The next bytes of the image, in the format of the transfer's destination.
     * @param bytes How many there are.
     */
    void transfer_in(const u8* data, std::size_t bytes);

    /**
     * Image data of a local to host transfer.
     *
     * @param[out] data Where to put the bytes.
     * @param bytes Room at `data`.
     * @return The bytes produced; 0 when no such transfer is active.
     */
    std::size_t transfer_out(u8* data, std::size_t bytes);

    /**
     * The start of a vertical blank: the event bit, and which field follows.
     *
     * Sets bit 3 (VSINT) and bit 13 (FIELD) of CSR (documented).
     *
     * @param odd_field True when the odd field follows.
     */
    void vblank(bool odd_field) { csr_ = (csr_ & ~u64{0x2000}) | 0x8 | (odd_field ? 0x2000 : 0); }

    /**
     * What the display circuits show, or false when no circuit is enabled.
     *
     * Waits for the drawing to finish first.
     *
     * @param[out] out The picture, when there is one.
     * @return True when a circuit is enabled and `out` holds its picture.
     */
    bool display(Image& out);

    /**
     * The same without waiting: the picture as it is after everything given so
     * far is taken when the drawing gets there, into `buffer`, and `done` is
     * called with it, on whichever thread that is.
     *
     * @param buffer An image to take the picture into; its storage is reused.
     * @param done Called with whether a circuit was enabled and with the picture.
     */
    void display_later(Image&& buffer, std::function<void(bool shown, Image& picture)> done);

    /**
     * A rectangle of any buffer as RGBA, for tools and tests.
     *
     * @param fbp Block pointer of the buffer, in blocks.
     * @param fbw Width of the buffer, in units of 64 pixels.
     * @param psm Pixel storage format of the buffer.
     * @param width Width of the rectangle in pixels.
     * @param height Height of the rectangle in pixels.
     * @return The rectangle from the buffer's origin, with alpha forced opaque.
     */
    Image snapshot(u32 fbp, u32 fbw, u32 psm, int width, int height);

    /**
     * For tools: called for every primitive drawn, with a line describing the
     * state it is drawn with (target, scissor, texture, tests, blending) and
     * its bounding box in the target.
     *
     * Called on the thread that gathers primitives, before the primitive is drawn. The two
     * floats are the lowest and highest level of detail at its vertices, 0 when it has none.
     */
    std::function<
        void(const std::string& state, int x0, int y0, int x1, int y1, float lod0, float lod1)>
        on_primitive;

    /** GS local memory. */
    GsMemory memory;

    /** Counters for tools. Read them after `finish`. */
    struct Stats {
        /** Primitives submitted for drawing. */
        u64 primitives = 0;

        /** Pixels that passed every test and were written. */
        u64 pixels = 0;

        /** Transfers started. */
        u64 transfers = 0;

        /** Texture levels decoded (`texture_decodes`) and the texels in them (`texels_decoded`). */
        u64 texture_decodes = 0, texels_decoded = 0;

        /** Batches handed over to be drawn. */
        u64 flushes = 0;
    } stats;

    /** The `gstodo` bits met so far. */
    u32 todo = 0;

private:
    /** A vertex as the queue holds it, with the registers it was given with. */
    struct Vertex {
        /** X and Y in 12.4 fixed point, before the context's offset. */
        u16 x = 0, y = 0;

        /** Depth, 24 or 32 bits as written. */
        u32 z = 0;

        /** Colour, R in the low byte. */
        u32 rgba = 0;

        /** Texture coordinates S and T and the reciprocal Q, as floats. */
        float s = 0, t = 0, q = 1;

        /** Texture coordinates U and V in 10.4 fixed point. */
        u16 u = 0, v = 0;

        /** Fog coefficient, 0-255. */
        u8 fog = 0;
    };

    /** A set of pages of GS memory (512 of 8 KB). */
    struct Pages {
        /** One bit for each of the 512 pages. */
        std::array<u64, 8> bits{};

        /** Adds a page to the set; the number wraps at 512. */
        void set(u32 page) { bits[(page >> 6) & 7] |= u64{1} << (page & 63); }

        /** Adds all the pages of another set. */
        void add(const Pages& o) {
            for (unsigned n = 0; n < 8; n++) {
                bits[n] |= o.bits[n];
            }
        }

        /** True when the two sets have a page in common. */
        bool intersects(const Pages& o) const {
            u64 any = 0;
            for (unsigned n = 0; n < 8; n++) {
                any |= bits[n] & o.bits[n];
            }
            return any != 0;
        }

        /** Empties the set. */
        void clear() { bits = {}; }
    };

    /**
     * Adds the pages a rectangle of a buffer lies in.
     *
     * @param[out] pages The set to add to.
     * @param psm Pixel storage format of the buffer.
     * @param bp Block pointer of the buffer, in blocks.
     * @param bw Width of the buffer, in units of 64 pixels.
     * @param x0 Left edge of the rectangle, in pixels, inclusive.
     * @param y0 Top edge, inclusive.
     * @param x1 Right edge, inclusive.
     * @param y1 Bottom edge, inclusive.
     */
    static void add_pages(Pages& pages, u32 psm, u32 bp, u32 bw, s32 x0, s32 y0, s32 x1, s32 y1);

    /** The colour table as it was at some moment, for the primitives given then. */
    struct ClutCopy {
        /** The 256 entries as 8-bit RGBA colours. */
        std::array<u32, 256> colours;

        /** Hash of all 256 entries, once something has asked. */
        u64 hash = 0;

        /** True once `hash` has been worked out. */
        bool hashed = false;
    };

    /** The texture state of a primitive: TEX0, TEX1, TEX2, CLAMP and TEXA, decoded. */
    struct Texture {
        /** Pixel storage format (TEX0 PSM). */
        u32 psm = 0;

        /** Base block pointer of each level (TEX0 TBP0, then the MIPTBP registers). */
        std::array<u32, 7> tbp{};

        /** Width of each level's buffer, in units of 64 pixels. */
        std::array<u32, 7> tbw{};

        /** Log2 of the texture's width (`tw`) and height (`th`) in texels, at most 10. */
        u32 tw = 0, th = 0;

        /** Texture colour component: whether the texture's alpha is used (TCC). */
        bool tcc = false;

        /** Texture function (TFX): 0 modulate, 1 decal, 2 highlight, 3 highlight 2. */
        u32 tfx = 0;

        /** Colour table entry offset, in units of 16 entries (CSA). */
        u32 csa = 0;

        /** The CLAMP fields: wrap modes in S and T (`wms`, `wmt`) and the region limits. */
        u32 wms = 0, wmt = 0, minu = 0, maxu = 0, minv = 0, maxv = 0;

        /** LCM: the level of detail comes from K, not from Q. */
        bool lcm = false;

        /** TEX1 fields: the highest mipmap level (`mxl`), the magnify and minify filters, and L. */
        u32 mxl = 0, mmag = 0, mmin = 0, l = 0;

        /** K, the level of detail bias, in levels. */
        float k = 0;

        /** Worked out once a primitive: the address tables of the texture's format. */
        const GsMemory::Layout* layout = nullptr;

        /** How a stored texel becomes a colour (see gs.cpp). */
        unsigned kind = 0;

        /** The level or the filter depends on each pixel's Q. */
        bool lod_per_pixel = false;

        /** TEXA: the alpha of a texel with alpha bit 0 (`ta0`) and 1 (`ta1`). */
        u32 ta0 = 0, ta1 = 0;

        /** TEXA AEM: a black texel of a 16 or 24-bit format is made transparent. */
        bool aem = false;

        /** The colour table as it was when the primitive was given. */
        const u32* clut = nullptr;

        /** The copy `clut` points into. */
        ClutCopy* clut_source = nullptr;

        /**
         * Each level as plain colours, taken when a primitive first needs it (null: the level is
         * read from GS memory texel by texel).
         */
        std::array<const u32*, 7> decoded{};
    };

    /** Everything a primitive needs, decoded from the registers of its context. */
    struct Env {
        /** FRAME: the frame buffer's block pointer, width (64-pixel units), format, write mask. */
        u32 fbp = 0, fbw = 0, fpsm = 0, fbmsk = 0;

        /** ZBUF: the depth buffer's block pointer and format, and the largest depth value. */
        u32 zbp = 0, zpsm = 0, zmax = 0;

        /** ZBUF ZMSK: depth values are not written. */
        bool zmsk = false;

        /** XYOFFSET: the drawing offset in 12.4 fixed point. */
        s32 ofx = 0, ofy = 0;

        /** SCISSOR: the rectangle drawn in, in pixels, edges inclusive. */
        s32 sx0 = 0, sx1 = 0, sy0 = 0, sy1 = 0;

        /** TEST: alpha test, destination alpha test, its mode, and depth test enables. */
        bool ate = false, date = false, datm = false, zte = false;

        /** TEST: alpha test mode and reference value, its fail action, and the depth test mode. */
        u32 atst = 0, aref = 0, afail = 0, ztst = 0;

        /** Alpha blending (PRIM ABE), per-pixel blending (PABE), colour clamp, forced alpha FBA. */
        bool abe = false, pabe = false, colclamp = true, fba = false;

        /** ALPHA: the selectors A, B, C and D of the blend equation, and the fixed value. */
        u32 ba = 0, bb = 0, bc = 0, bd = 0, fix = 0;

        /** PRIM: Gouraud shading, texturing, fogging, and texture coordinates as UV, not STQ. */
        bool iip = false, tme = false, fge = false, fst = false;

        /** FOGCOL: the fog colour, RGB. */
        u32 fogcol = 0;

        /** The texture state, filled only when texturing is on. */
        Texture tex;

        /** Address tables of the frame buffer's format and of the depth buffer's. */
        const GsMemory::Layout* flayout = nullptr;
        const GsMemory::Layout* zlayout = nullptr;

        /** The frame buffer is 16-bit or 24-bit, the depth buffer is 16-bit or 24-bit. */
        bool f16 = false, f24 = false, z16 = false, z24 = false;

        /** Which buffers it draws to. */
        u64 target = 0;

        /**
         * About the texture's levels, filled in as primitives need them: the levels found
         * (`looked_at`) and the levels read from GS memory (`in_place`).
         */
        u32 looked_at = 0, in_place = 0;

        /** The pages each level lies in. */
        std::array<Pages, 7> level_pages{};

        /** The levels the last primitive needed. */
        u32 last_need = 0;

        /** Their pages: of those decoded, of those read in place. */
        Pages decoded_pages, in_place_pages;
    };

    /** A primitive waiting to be drawn. */
    struct Queued {
        /** The state it is drawn with; owned by the batch. */
        const Env* env;

        /** The primitive type: point, line, triangle or sprite. */
        u8 kind;

        /** Its vertices, as many as the type has. */
        Vertex v[3];
    };

    /**
     * Primitives gathered to be drawn together, with everything they refer
     * to. Once handed over to be drawn, nothing in it changes.
     */
    struct Batch {
        /**
         * Its primitives depend on each other's pixels: they are drawn one
         * after the other, each top to bottom, not by bands.
         */
        bool serial = false;

        /** Not primitives but something to do in their place in the order. */
        std::function<void()> task;

        /** The primitives, in the order given. */
        std::vector<Queued> primitives;

        /** By 16 scan lines: the primitives that reach each band. */
        std::array<std::vector<u32>, 128> bands;

        /** The bands that have any primitive. */
        std::vector<u16> used_bands;

        /** The states the primitives refer to. */
        std::deque<Env> envs;

        /** The colour table copies the states refer to. */
        std::deque<ClutCopy> cluts;

        /** The decoded levels its states use. */
        std::vector<std::shared_ptr<std::vector<u32>>> textures;

        /** Empties the batch for reuse. */
        void clear();
    };

    /** The progress of a host to local or local to host transfer. */
    struct Transfer {
        /**
         * Whether the transfer is still running (`active`), and for host to local, whether
         * anything written differed from what was there (`changed`).
         */
        bool active = false, changed = false;

        /** TRXDIR direction; 3 means none. */
        u32 dir = 3;

        /** Pixels done in the current row, and rows done. */
        u32 x = 0, y = 0;

        /** Size of the area in pixels. */
        u32 width = 0, height = 0;

        /** Bytes received and not yet whole pixels, or bytes produced and not yet given out. */
        std::vector<u8> pending;
    };

    /**
     * Reports once that the model meets something it does not do yet.
     *
     * @param what The `gstodo` bit; a bit already set reports nothing.
     * @param text What was met, for the message on stderr.
     */
    void note(u32 what, const char* text);

    // Primitive assembly.

    /**
     * Takes a vertex into the queue and completes a primitive when the queue is full.
     *
     * @param x X in 12.4 fixed point.
     * @param y Y in 12.4 fixed point.
     * @param z Depth.
     * @param draw False for XYZ3 and XYZF3, which queue the vertex without drawing.
     */
    void vertex(u16 x, u16 y, u32 z, bool draw);

    /**
     * Draws a point. Rows outside clip0..clip1 are left to whoever has that band.
     *
     * @param e The state to draw with.
     * @param a The vertex.
     * @param clip0 First row this call may draw.
     * @param clip1 Last row this call may draw.
     */
    void draw_point(const Env& e, const Vertex& a, s32 clip0, s32 clip1);

    /**
     * Draws a line; rows outside clip0..clip1 are left to whoever has that band.
     *
     * @param e The state to draw with.
     * @param a First vertex.
     * @param b Second vertex.
     * @param clip0 First row this call may draw.
     * @param clip1 Last row this call may draw.
     */
    void draw_line(const Env& e, const Vertex& a, const Vertex& b, s32 clip0, s32 clip1);

    /**
     * Draws a triangle; rows outside clip0..clip1 are left to whoever has that band.
     *
     * @param e The state to draw with.
     * @param a First vertex.
     * @param b Second vertex.
     * @param c Third vertex.
     * @param clip0 First row this call may draw.
     * @param clip1 Last row this call may draw.
     */
    void draw_triangle(
        const Env& e, const Vertex& a, const Vertex& b, const Vertex& c, s32 clip0, s32 clip1
    );

    /**
     * Draws a sprite; rows outside clip0..clip1 are left to whoever has that band.
     *
     * @param e The state to draw with.
     * @param a First corner.
     * @param b Second corner, which also carries the colour.
     * @param clip0 First row this call may draw.
     * @param clip1 Last row this call may draw.
     */
    void draw_sprite(const Env& e, const Vertex& a, const Vertex& b, s32 clip0, s32 clip1);

    /**
     * Draws one queued primitive.
     *
     * @param q The primitive.
     * @param clip0 First row this call may draw.
     * @param clip1 Last row this call may draw.
     */
    void draw(const Queued& q, s32 clip0, s32 clip1);

    /**
     * Takes the primitive in the queue: draws it now, or adds it to the batch being gathered.
     *
     * @param kind The primitive type.
     * @param count How many vertices it has.
     */
    void submit(unsigned kind, unsigned count);

    /**
     * Draws a batch: with its task, serially, or by bands.
     *
     * @param batch The batch.
     */
    void render(const Batch& batch);

    /**
     * Draws the part of a batch that lies in one band of scan lines.
     *
     * @param batch The batch.
     * @param band Number of the band, in units of 16 scan lines.
     */
    void render_band(const Batch& batch, unsigned band);

    /**
     * Decodes the registers of the current context into a state.
     *
     * @return The state for the primitives given from now on.
     */
    Env environment();

    /** Makes sure there is a current state in the batch being gathered. */
    void ensure_env();

    /**
     * Calls `on_primitive` for the primitive in the queue.
     *
     * @param e The state it is drawn with.
     * @param count How many vertices it has.
     */
    void report(const Env& e, unsigned count) const;

    /**
     * Gives the PRIM register as the current primitive uses it.
     *
     * @return PRIM, with the attribute bits from PRMODE when PRMODECONT says so.
     */
    u32 prim_bits() const;

    // Per pixel.

    /**
     * Runs one pixel through the tests, the blend and the writes.
     *
     * @param e The state to draw with.
     * @param frow The frame buffer's row of the pixel.
     * @param zrow The depth buffer's row of the pixel.
     * @param x The pixel's column.
     * @param z Its depth.
     * @param rgba Its colour after texturing and fog.
     */
    void pixel(
        const Env& e, const GsMemory::Row& frow, const GsMemory::Row& zrow, s32 x, u32 z, u32 rgba
    );

    /**
     * Applies the texture and the fog to a vertex colour.
     *
     * @param e The state.
     * @param rgba The interpolated vertex colour.
     * @param u Texture coordinate U, in texels.
     * @param v Texture coordinate V, in texels.
     * @param lod Level of detail.
     * @param fog Fog coefficient, 0-255.
     * @return The colour to draw.
     */
    u32 shade(const Env& e, u32 rgba, float u, float v, float lod, u32 fog) const;

    /**
     * Samples the texture at a point, choosing the level and the filter.
     *
     * @param t The texture.
     * @param u Texture coordinate U, in texels of the first level.
     * @param v Texture coordinate V, in texels of the first level.
     * @param lod Level of detail.
     * @return The colour.
     */
    u32 sample(const Texture& t, float u, float v, float lod) const;

    /**
     * Samples one level of the texture.
     *
     * @param t The texture.
     * @param level The level.
     * @param u Texture coordinate U, in texels of the first level.
     * @param v Texture coordinate V, in texels of the first level.
     * @param linear True for bilinear filtering, false for the nearest texel.
     * @return The colour.
     */
    u32 sample_level(const Texture& t, u32 level, float u, float v, bool linear) const;

    /**
     * Reads one texel as a colour, applying the wrap mode.
     *
     * @param t The texture.
     * @param level The level.
     * @param iu Texel column.
     * @param iv Texel row.
     * @param decoded The level as plain colours, or null to read GS memory.
     * @return The colour.
     */
    u32 texel(const Texture& t, u32 level, s32 iu, s32 iv, const u32* decoded) const;

    /**
     * Reads a pixel of the frame buffer as 8-bit RGBA.
     *
     * @param e The state.
     * @param row The pixel's row.
     * @param x The pixel's column.
     * @return The colour; for a 16-bit format the alpha bit becomes 0 or 0x80.
     */
    u32 frame_read(const Env& e, const GsMemory::Row& row, s32 x) const;

    /**
     * Writes a pixel of the frame buffer.
     *
     * @param e The state.
     * @param row The pixel's row.
     * @param x The pixel's column.
     * @param rgba The colour in 8-bit RGBA.
     * @param mask The bits of the 32-bit colour to write.
     */
    void frame_write(const Env& e, const GsMemory::Row& row, s32 x, u32 rgba, u32 mask);

    /**
     * Decoded textures. A level is decoded once and kept until something
     * writes to the pages it lies in, the colour table it used changes, or
     * TEXA changes under a format that needs it.
     */
    struct CachedTexture {
        /** The level as plain colours. */
        std::shared_ptr<std::vector<u32>> texels;

        /** The pages it was decoded from. */
        Pages pages;

        /** The value of the clock when it was decoded. */
        u64 stamp = 0;
    };

    /**
     * Gives a level as plain colours, decoding it if no valid copy is kept.
     *
     * @param t The texture.
     * @param level The level.
     * @return The decoded level.
     */
    std::shared_ptr<std::vector<u32>> cached_level(const Texture& t, u32 level);

    /**
     * Which levels of its texture the primitive in the queue can read.
     *
     * @param e The state.
     * @param count How many vertices the primitive has.
     * @return A bit for each level.
     */
    u32 levels_needed(const Env& e, unsigned count) const;

    /**
     * Have a current state with those levels found: decoded copies, or in place.
     *
     * @param need A bit for each level.
     */
    void prepare_levels(u32 need);

    /**
     * Finds the pages the primitive reads of a level it reads in place, when its coordinates say
     * so plainly.
     *
     * @param e The state.
     * @param count How many vertices the primitive has.
     * @param need A bit for each level the primitive needs.
     * @param[out] pages The pages, when the answer is true.
     * @return False when the whole level has to count.
     */
    bool in_place_reach(const Env& e, unsigned count, u32 need, Pages& pages) const;

    /**
     * Note a write to pages: decoded copies of them are stale from now on.
     *
     * @param pages The pages written.
     */
    void stamp(const Pages& pages);

    /**
     * A level is known by where and how it is stored, and by what turns its
     * texels into colours: the table entries it uses, or TEXA.
     */
    struct TextureKey {
        /**
         * Where and how the level is stored (`place`: block pointer, width, format, size, offset),
         * and what turns texels into colours (`colours`: TEXA fields, or a hash of the table
         * entries used).
         */
        u64 place = 0, colours = 0;

        /** Keys are equal when both parts are. */
        bool operator==(const TextureKey&) const = default;
    };

    /** Hash for `TextureKey`. */
    struct TextureKeyHash {
        /** Mixes the two parts of the key into one hash. */
        std::size_t operator()(const TextureKey& k) const {
            return static_cast<std::size_t>(k.place * 0x9E3779B97F4A7C15ull ^ k.colours);
        }
    };

    /** Decoded levels that are kept. */
    std::unordered_map<TextureKey, CachedTexture, TextureKeyHash> texture_cache_;

    /** The value of `clock_` when each page was last written. */
    std::array<u64, 512> page_stamp_{};

    /** A counter that goes up with each write to GS memory. */
    u64 clock_ = 1;

    // Colour lookup table.

    /**
     * Notes a write to TEX0 or TEX2 that may ask for the colour table to be loaded.
     *
     * @param tex0 The value of TEX0, or the TEX0 as TEX2 changed it.
     */
    void load_clut(u64 tex0);

    /** Loads the colour table that was put off, if there is one. */
    void do_clut_load();

    /** A colour table load that has been asked for and not done. */
    struct ClutLoad {
        /** Whether a load is waiting. */
        bool waiting = false;

        /** The TEX0 and TEXCLUT values that asked for it. */
        u64 tex0 = 0, texclut = 0;

        /** The pages it reads. */
        Pages source;

        /** The entries it loads: the first, and how many. */
        u32 first = 0, count = 0;
    };

    /** The pending colour table load. */
    ClutLoad clut_load_;

    /** Turns the raw table entries into colours with the current TEXA. */
    void rebuild_clut();

    /**
     * Turns a 16-bit colour into 8-bit RGBA.
     *
     * @param c The colour, 5:5:5:1.
     * @param ta0 Alpha for a colour with its alpha bit 0.
     * @param ta1 Alpha for a colour with its alpha bit 1.
     * @param aem True to make a black colour transparent.
     * @return The colour.
     */
    static u32 expand16(u16 c, u32 ta0, u32 ta1, bool aem);

    /*
     * Primitives are gathered into a batch; a full batch is drawn by a thread
     * of its own while the next is gathered, its bands shared out among the
     * pool. A batch being drawn touches only the pages it draws to and the
     * textures it reads in place, so whoever gathers may read and write every
     * other page meanwhile, and waits before touching those.
     */

    /** The threads that share out the bands of a batch; defined in gs.cpp. */
    class Pool;

    /** The thread that draws batches in order; defined in gs.cpp. */
    class Raster;

    /** The pool, or null when fewer than two threads draw. */
    std::unique_ptr<Pool> pool_;

    /** How many threads draw (see `set_threads`). */
    unsigned threads_ = 0;

    /** The batch being gathered. */
    std::unique_ptr<Batch> batch_;

    /** What it will write, and read in place. */
    Pages pending_write_, pending_read_;

    /** What it will write, by which side: colour and depth. */
    Pages pending_colour_, pending_depth_;

    /** The same for batches handed over and maybe not drawn yet. */
    Pages inflight_write_, inflight_read_;

    /** The buffers the batch being gathered draws to (`Env::target`). */
    u64 pending_target_ = 0;

    /** The current state, in `batch_`, or null when it must be made again. */
    Env* env_ = nullptr;

    /** Whether the registers changed since `env_` was made, and whether the colour table did. */
    bool env_dirty_ = true, clut_copy_dirty_ = true;

    /** Pixels written, summed from the drawing threads; atomic because they add to it. */
    std::atomic<u64> pixels_{0};

    /**
     * Hand the gathered batch over (or draw it here, with no thread for that).
     * The current state is gone afterwards: `ensure_env` makes the next.
     */
    void flush();

    /** Waits until every batch handed over has been drawn. */
    void wait_for_drawing();

    /** What the display circuits are set to show. */
    struct Shown {
        /** Whether a circuit is enabled and has a picture. */
        bool on = false;

        /** The buffer: block pointer, width in 64-pixel units, format, and the offset into it. */
        u32 bp = 0, bw = 0, psm = 0, x = 0, y = 0;

        /** The size of the picture in pixels. */
        int width = 0, height = 0;
    };

    /**
     * Reads what the display circuits are set to show now.
     *
     * @return The first enabled circuit's picture, with `on` false when there is none.
     */
    Shown shown() const;

    /**
     * Copies the displayed buffer into an image.
     *
     * @param what What to copy, from `shown`.
     * @param[out] out The image to fill; its size is set.
     */
    void copy_shown(const Shown& what, Image& out) const;

    /**
     * Before reading pages of GS memory outside drawing: draws what is to be drawn there.
     *
     * @param pages The pages about to be read.
     */
    void before_read(const Pages& pages);

    /**
     * Before writing pages of GS memory outside drawing: draws what reads or writes there.
     *
     * @param pages The pages about to be written.
     */
    void before_write(const Pages& pages);

    /** The thread that draws batches; last: it stops before what it uses goes. */
    std::unique_ptr<Raster> raster_;

    // Transfers.

    /** Starts the transfer that TRXDIR asked for. */
    void start_transfer();

    /** Runs a local to local transfer. */
    void copy_local();

    /** The general registers, by address. */
    std::array<u64, 0x80> reg_{};

    /** The privileged registers from PMODE to BGCOLOR, by (address >> 4). */
    std::array<u64, 0x12> priv_{};

    /** CSR, IMR, BUSDIR and SIGLBLID. */
    u64 csr_ = 0, imr_ = 0, busdir_ = 0, siglblid_ = 0;

    /** The vertices of the primitive being assembled. */
    std::array<Vertex, 3> queue_{};

    /** How many vertices are in `queue_`. */
    int count_ = 0;

    /** The fog coefficient the next vertex takes. */
    u8 fog_ = 0;

    /** The colour table as loaded from memory, before conversion. */
    std::array<u32, 256> clut_raw_{};

    /** The colour table as 8-bit RGBA. */
    std::array<u32, 256> clut_{};

    /** The pixel format of the table as loaded. */
    u32 clut_psm_ = PSMCT32;

    /** The table base remembered by the two "load if it changed" kinds of load (CBP0 and CBP1). */
    std::array<u32, 2> clut_cbp_{};

    /** The host to local transfer. */
    Transfer in_;

    /** The local to host transfer. */
    Transfer out_;

    /** The pages the host to local transfer writes. */
    Pages in_pages_;
};

}  // namespace ps2
