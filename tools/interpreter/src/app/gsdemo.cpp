// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * openrac-gsdemo: draws a scene of its own through the same path a game's
 * frame takes. It writes a display list into guest memory as a VIF1 DMA
 * chain (CNT and REF tags carrying VIF codes, GIF packets sent with DIRECT,
 * a texture and its colour table sent as image data, a VU1 microprogram and
 * the vertices it transforms), runs the chain through the DMA walker, VIF1,
 * VU1, the GIF and the software GS, and shows what the GS's display circuit
 * reads. Nothing here comes from a game: the microprogram is written below.
 *
 * It shows the picture in a window, or writes it to a file when run headless. It leaves out
 * input, sound and timing: a frame is drawn as fast as the model runs.
 *
 * Sources: the DMA tag, VIF code, GIF tag and GS register layouts as publicly documented.
 */

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <vector>

#include "ps2/dma.h"
#include "ps2/graphics.h"
#include "ps2/memory.h"
#include "ps2/vu_asm.h"

#ifndef OPENRAC_NO_WINDOW
#include "host/window.h"
#endif

using namespace ps2;

namespace {

/** Size of the picture in pixels (`kWidth`, `kHeight`). */
constexpr int kWidth = 512, kHeight = 448;

/**
 * Where things sit in GS local memory. The colour buffer takes pages 0 to 111 (512 x 448 x 32
 * bits is 112 pages of 8 KB) and the Z buffer the 112 after it. The texture comes next, in blocks
 * of 256 bytes (32 to a page): a 64 x 64 8-bit texture is 16 blocks, and its colour table follows.
 */
constexpr u32 kFramePage = 0;
constexpr u32 kZPage = 112;
constexpr u32 kTextureBlock = 224 * 32;
constexpr u32 kClutBlock = kTextureBlock + 16;

/**
 * The drawing offset (XYOFFSET) that puts the picture's top left corner at (0, 0), when the centre
 * of the picture is the centre of the GS coordinate space, (2048, 2048).
 */
constexpr u32 kOffsetX = 2048 - kWidth / 2, kOffsetY = 2048 - kHeight / 2;

/** Guest addresses of the display list, of the uploads it refers to, and of the microprogram. */
constexpr u32 kListAddress = 0x00400000;
constexpr u32 kUploadAddress = 0x00500000;
constexpr u32 kProgramAddress = 0x00520000;

/**
 * VIF codes, with the CMD field in bits 24-30 (documented): NOP, STCYCL, MSCAL, MPG, DIRECT, and
 * UNPACK in the V4-32 format.
 */
constexpr u32 kVifNop = 0, kVifStcycl = 0x01000000, kVifMscal = 0x14000000, kVifMpg = 0x4A000000,
              kVifDirect = 0x50000000, kVifUnpackV4_32 = 0x6C000000;

/** PRIM: the primitive type, field PRIM bits 0-2: triangle, triangle strip, sprite (documented). */
constexpr u32 kTriangle = 3, kTriangleStrip = 4, kSprite = 6;

/**
 * PRIM: the attribute bits: IIP (Gouraud) bit 3, TME bit 4, FGE bit 5, ABE bit 6, FST (UV) bit 8
 * (documented).
 */
constexpr u32 kGouraud = 1 << 3, kTextured = 1 << 4, kFogged = 1 << 5, kBlended = 1 << 6,
              kUv = 1 << 8;

/**
 * A display list in guest memory, written the way a game writes one.
 *
 * It appends quadwords at a growing address. A DMA tag and the packet it announces are written
 * as a `begin_*` call, the quadwords, and an `end_*` call that fills the tag's count in.
 */
class List {
public:
    /**
     * Starts a list.
     *
     * @param memory The guest memory to write into.
     * @param address Where the list starts, a multiple of 16.
     */
    List(GuestMemory& memory, u32 address) : memory_(memory), at_(address) {}

    /** The address the next quadword goes to. */
    u32 address() const { return at_; }

    /**
     * Appends one quadword.
     *
     * @param lo Bits 0-63 of the quadword.
     * @param hi Bits 64-127.
     */
    void quad(u64 lo, u64 hi) {
        memory_.write<u64>(at_, lo);
        memory_.write<u64>(at_ + 8, hi);
        at_ += 16;
    }

    /**
     * Opens a CNT tag whose two spare words are VIF codes: NOP, then DIRECT for the
     * quadwords that follow. The count is filled in by end_direct().
     */
    void begin_direct() {
        open_ = at_;
        quad(0, 0);
    }

    /**
     * Closes the tag opened by `begin_direct`: sets its quadword count and the DIRECT code's.
     *
     * @param irq True to set the tag's IRQ bit, which stops the walker when TIE is on.
     */
    void end_direct(bool irq = false) {
        // The quadwords written since the tag, not counting the tag.
        u32 quadwords = (at_ - open_) / 16 - 1;

        // DMA tag: QWC bits 0-15, ID bits 28-30, IRQ bit 31 (documented).
        u64 tag = quadwords | (u64{dmatag::CNT} << 28) | (u64{irq} << 31);

        // The DIRECT code goes in the second spare word, bits 96-127 of the tag quadword.
        u64 vif = u64{kVifDirect | quadwords} << 32;
        memory_.write<u64>(open_, tag);
        memory_.write<u64>(open_ + 8, vif);
    }

    /**
     * Appends a REF tag: the quadwords are elsewhere in memory.
     *
     * @param address Where the quadwords are, outside the list.
     * @param quadwords How many there are.
     */
    void ref_direct(u32 address, u32 quadwords) {
        // DMA tag: QWC bits 0-15, ID bits 28-30, ADDR bits 32-62 (documented).
        quad(
            quadwords | (u64{dmatag::REF} << 28) | (u64{address} << 32),
            u64{kVifDirect | quadwords} << 32
        );
    }

    /** Appends the END tag that stops the walker. */
    void end() { quad(u64{dmatag::END} << 28, 0); }

    /**
     * Opens a CNT tag with any two VIF codes in its spare words, for data that is
     * not a GIF packet. The count is filled in by end_data().
     *
     * @param vif0 The VIF code in bits 64-95 of the tag.
     * @param vif1 The VIF code in bits 96-127.
     */
    void begin_data(u32 vif0, u32 vif1) {
        open_ = at_;
        quad(0, vif0 | (u64{vif1} << 32));
    }

    /** Closes the tag opened by `begin_data`: sets its quadword count. */
    void end_data() {
        u32 quadwords = (at_ - open_) / 16 - 1;
        memory_.write<u64>(open_, quadwords | (u64{dmatag::CNT} << 28));
    }

    /**
     * Appends a REF tag with any two VIF codes.
     *
     * @param address Where the quadwords are.
     * @param quadwords How many there are.
     * @param vif0 The VIF code in bits 64-95 of the tag.
     * @param vif1 The VIF code in bits 96-127.
     */
    void ref(u32 address, u32 quadwords, u32 vif0, u32 vif1) {
        quad(quadwords | (u64{dmatag::REF} << 28) | (u64{address} << 32), vif0 | (u64{vif1} << 32));
    }

    /**
     * Appends four floats as one quadword, x in the lowest word.
     *
     * @param x Word 0.
     * @param y Word 1.
     * @param z Word 2.
     * @param w Word 3.
     */
    void floats(float x, float y, float z, float w) {
        quad(as_u32(x) | (u64{as_u32(y)} << 32), as_u32(z) | (u64{as_u32(w)} << 32));
    }

    /**
     * Appends four 32-bit words as one quadword, x in the lowest word.
     *
     * @param x Word 0.
     * @param y Word 1.
     * @param z Word 2.
     * @param w Word 3.
     */
    void words(u32 x, u32 y, u32 z, u32 w) { quad(x | (u64{y} << 32), z | (u64{w} << 32)); }

    /**
     * Appends a GIF tag.
     *
     * @param loops NLOOP, the number of loops or image quadwords.
     * @param eop True for the last tag of its packet.
     * @param flg The data layout: 0 PACKED, 1 REGLIST, 2 IMAGE.
     * @param regs The register descriptors, at most 16.
     * @param pre True to have the tag write `prim` to PRIM.
     * @param prim The PRIM value, when `pre` is set.
     */
    void gif_tag(
        u32 loops, bool eop, u32 flg, std::initializer_list<u8> regs, bool pre = false, u32 prim = 0
    ) {
        // GIF tag: NLOOP bits 0-14, EOP 15, PRE 46, PRIM 47-57, FLG 58-59, NREG 60-63 (documented).
        u64 lo = loops | (u64{eop} << 15) | (u64{pre} << 46) | (u64{prim} << 47) | (u64{flg} << 58)
                 | (u64{regs.size() & 0xF} << 60);

        // REGS: one 4-bit descriptor for each register, the first in the lowest bits.
        u64 hi = 0;
        unsigned n = 0;
        for (u8 r : regs) {
            hi |= u64{r} << (n++ * 4);
        }

        quad(lo, hi);
    }

    /**
     * Appends an A+D quadword: a write of one register by its address.
     *
     * @param reg The register's address.
     * @param value The 64-bit value.
     */
    void ad(u8 reg, u64 value) { quad(value, reg); }

    /**
     * Appends a PACKED RGBAQ quadword: the packed forms of the vertex registers start here.
     *
     * @param r Red, 0-255.
     * @param g Green, 0-255.
     * @param b Blue, 0-255.
     * @param a Alpha, 0-255 (0x80 is opaque).
     */
    void rgbaq(u32 r, u32 g, u32 b, u32 a) { quad(r | (u64{g} << 32), b | (u64{a} << 32)); }

    /**
     * Appends a PACKED ST quadword.
     *
     * @param s Texture coordinate S.
     * @param t Texture coordinate T.
     * @param q The reciprocal of W.
     */
    void st(float s, float t, float q) { quad(as_u32(s) | (u64{as_u32(t)} << 32), as_u32(q)); }

    /**
     * Appends a PACKED UV quadword.
     *
     * @param u Texel column, in texels.
     * @param v Texel row, in texels.
     */
    void uv(float u, float v) { quad(fixed(u) | (u64{fixed(v)} << 32), 0); }

    /**
     * Appends a PACKED XYZ2 quadword.
     *
     * @param x Screen X from the centre, in pixels.
     * @param y Screen Y from the centre, in pixels.
     * @param z Depth.
     */
    void xyz(float x, float y, u32 z) { quad(screen_x(x) | (u64{screen_y(y)} << 32), z); }

    /**
     * Appends a PACKED XYZF2 quadword.
     *
     * @param x Screen X from the centre, in pixels.
     * @param y Screen Y from the centre, in pixels.
     * @param z Depth, 24 bits.
     * @param fog Fog coefficient, 0-255.
     */
    void xyzf(float x, float y, u32 z, u32 fog) {
        // Z is bits 68-91 and F bits 100-107 of the quadword (documented).
        quad(screen_x(x) | (u64{screen_y(y)} << 32), (u64{z & 0xFFFFFF} << 4) | (u64{fog} << 36));
    }

private:
    /**
     * Turns a number into 12.4 fixed point (or 10.4 for UV), rounded to nearest.
     *
     * @param v The value.
     * @return The fixed-point value; 16 units make 1.0.
     */
    static u32 fixed(float v) { return static_cast<u32>(std::lround(v * 16.0f)); }

    /**
     * Turns a screen X into a GS vertex X.
     *
     * Coordinates are given from the centre of the screen, which sits at the
     * centre of the GS coordinate space (2048, 2048); XYOFFSET then puts the
     * buffer's top left corner at (0, 0).
     *
     * @param x Pixels from the centre.
     * @return X in 12.4 fixed point, 16 bits.
     */
    static u32 screen_x(float x) {
        return static_cast<u32>(std::lround((x + 2048.0f) * 16.0f)) & 0xFFFF;
    }

    /**
     * Turns a screen Y into a GS vertex Y, as `screen_x` does for X.
     *
     * @param y Pixels from the centre.
     * @return Y in 12.4 fixed point, 16 bits.
     */
    static u32 screen_y(float y) {
        return static_cast<u32>(std::lround((y + 2048.0f) * 16.0f)) & 0xFFFF;
    }

    /** The guest memory the list is written into. */
    GuestMemory& memory_;

    /** The address the next quadword goes to. */
    u32 at_;

    /** The address of the tag opened by the last `begin_*` call. */
    u32 open_ = 0;
};

/**
 * Packs a TEX0 value.
 *
 * @param tbp Texture base block pointer.
 * @param tbw Texture buffer width, in units of 64 pixels.
 * @param psm Texture pixel format.
 * @param tw Log2 of the texture width.
 * @param th Log2 of the texture height.
 * @param tcc True to use the texture's alpha.
 * @param tfx Texture function: 0 modulate, 1 decal.
 * @param cbp Colour table base block pointer.
 * @param cpsm Colour table pixel format.
 * @param cld Colour table load control.
 * @return The register value.
 */
u64 tex0(u32 tbp, u32 tbw, u32 psm, u32 tw, u32 th, bool tcc, u32 tfx, u32 cbp, u32 cpsm, u32 cld) {
    /*
     * TEX0 fields, by their lowest bit: TBP 0, TBW 14, PSM 20, TW 26, TH 30, TCC 34, TFX 35,
     * CBP 37, CPSM 51, CLD 61.
     */
    return tbp | (u64{tbw} << 14) | (u64{psm} << 20) | (u64{tw} << 26) | (u64{th} << 30)
           | (u64{tcc} << 34) | (u64{tfx} << 35) | (u64{cbp} << 37) | (u64{cpsm} << 51)
           | (u64{cld} << 61);
}

/**
 * Writes one host to local transfer, as a packet of its own: four register writes,
 * then the pixels as image data.
 *
 * @param memory The guest memory to write into.
 * @param address Where the packet goes.
 * @param block Destination block pointer in GS memory.
 * @param width_units Destination buffer width, in units of 64 pixels.
 * @param psm Destination pixel format.
 * @param width Width of the area in pixels.
 * @param height Height of the area in pixels.
 * @param pixels The image data, in the destination's format.
 * @return The packet's length in quadwords.
 */
u32 write_upload(
    GuestMemory& memory,
    u32 address,
    u32 block,
    u32 width_units,
    u32 psm,
    u32 width,
    u32 height,
    const std::vector<u8>& pixels
) {
    List list(memory, address);

    // Four A+D writes set up the transfer: BITBLTBUF, TRXPOS, TRXREG, TRXDIR.
    list.gif_tag(4, false, 0, {0xE});

    // BITBLTBUF: DBP bits 32-45, DBW 48-53, DPSM 56-61 (documented).
    list.ad(gsreg::BITBLTBUF, (u64{block} << 32) | (u64{width_units} << 48) | (u64{psm} << 56));
    list.ad(gsreg::TRXPOS, 0);

    // TRXREG: width bits 0-11, height bits 32-43; TRXDIR 0 is host to local.
    list.ad(gsreg::TRXREG, width | (u64{height} << 32));
    list.ad(gsreg::TRXDIR, 0);

    // The image data, in quadwords rounded up, after an IMAGE tag (FLG 2) that ends the packet.
    u32 quadwords = static_cast<u32>((pixels.size() + 15) / 16);
    list.gif_tag(quadwords, true, 2, {});
    std::memcpy(memory.ram(list.address()), pixels.data(), pixels.size());

    return (list.address() - address) / 16 + quadwords;
}

/** Where the uploads sit in guest memory, and how long each is. */
struct Uploads {
    /** The texture's packet: its address and length in quadwords. */
    u32 texture_address = 0, texture_quadwords = 0;

    /** The colour table's packet. */
    u32 clut_address = 0, clut_quadwords = 0;

    /** The microprogram, with the VIF code that loads it. */
    u32 program_address = 0, program_quadwords = 0;
};

/**
 * Where the microprogram finds its input, from the address XTOP gives it,
 * and where it builds its packet. Quadword offsets in VU1 data memory: the matrix takes four, the
 * GIF tag and the vertex count one each, the vertices follow at three a vertex, and the packet is
 * built at 512.
 */
constexpr u32 kVuMatrix = 0, kVuTag = 4, kVuCount = 5, kVuVertices = 6, kVuOutput = 512;

/**
 * Builds the microprogram.
 *
 * For each vertex (texture coordinates, colour, position)
 * multiply the position by a 4 x 4 matrix, divide by W, turn X and Y into
 * the GS's fixed point and Z into an integer, and write the three quadwords
 * of a packed GIF vertex. Then kick the packet.
 *
 * @return The program, as pairs of instructions.
 */
vuasm::Program vertex_program() {
    using namespace vuasm;
    Program p;

    // Setup: VI1 gets the input base, the matrix goes in VF1-VF4 and the GIF tag in VF10.
    p.lo(xtop(1));
    p.lo(lq(XYZW, 1, kVuMatrix + 0, 1));
    p.lo(lq(XYZW, 2, kVuMatrix + 1, 1));
    p.lo(lq(XYZW, 3, kVuMatrix + 2, 1));
    p.lo(lq(XYZW, 4, kVuMatrix + 3, 1));
    p.lo(lq(XYZW, 10, kVuTag, 1));

    // VI3 counts the vertices left; VI2 is the packet's base, which starts with the tag.
    p.lo(ilw(X, 3, kVuCount, 1));
    p.lo(iaddiu(2, 1, kVuOutput));
    p.lo(sq(XYZW, 10, 0, 2));
    p.lo(iaddiu(5, 2, 1));            // where the next vertex is written
    p.lo(iaddiu(4, 1, kVuVertices));  // where the next vertex is read

    // One pass of the loop transforms one vertex.
    u32 loop = p.here();
    p.lo(lq(XYZW, 20, 2, 4));                            // position
    p.add(mulabc(XYZW, 1, 20, 0), lq(XYZW, 21, 0, 4));   // ACC = column 0 * x; texture coordinates
    p.add(maddabc(XYZW, 2, 20, 1), lq(XYZW, 22, 1, 4));  // + column 1 * y; colour
    p.hi(maddabc(XYZW, 3, 20, 2));                       // + column 2 * z
    p.hi(maddbc(XYZW, 23, 4, 20, 3));                    // + column 3 * w
    p.lo(div(0, 3, 23, 3));                              // Q = 1 / W
    p.lo(waitq());
    p.hi(mulq(XYZ, 23, 23));  // the perspective divide
    p.hi(mulq(XYZ, 21, 21));  // S/W, T/W, 1/W

    // X and Y to 12.4 fixed point (the 4 in ftoi4), Z to an integer; texture and colour are stored.
    p.add(ftoi4(XY, 24, 23), sq(XYZW, 22, 1, 5));
    p.add(ftoi0(Z, 24, 23), sq(XYZ, 21, 0, 5));
    p.lo(iaddiu(4, 4, 3));
    p.lo(iaddi(3, 3, -1));
    p.lo(sq(XYZ, 24, 2, 5));

    // Loop while vertices are left; the offset is counted from the instruction after the branch.
    p.lo(ibne(3, 0, static_cast<s32>(loop) - static_cast<s32>(p.here() + 1)));
    p.lo(iaddiu(5, 5, 3));  // in the branch's delay slot

    // Kick the packet and end the program (the E bit), with an empty pair after it.
    p.lo(xgkick(2));
    p.add(nop() | E, lnop());
    p.hi(nop());
    return p;
}

/**
 * Builds the uploads and writes them into guest memory.
 *
 * A 64 x 64 texture of 8-bit indices (tiles with a ring in each) and its
 * table of 256 colours, laid out as the GS expects a table to be stored, and the microprogram.
 *
 * @param memory The guest memory to write into.
 * @return Where each upload is, and how long.
 */
Uploads prepare_uploads(GuestMemory& memory) {
    // The texture: 4 by 4 tiles of 16 by 16 texels, each with a ring, in two halves of colour.
    std::vector<u8> texels(64 * 64);
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            // Tiles alternate like a chessboard (16 texels a tile).
            int tile = ((x >> 4) + (y >> 4)) & 1;

            // The ring is a circle of radius 5 around the middle of the tile, drawn as shade 15.
            float dx = static_cast<float>((x & 15) - 8) + 0.5f,
                  dy = static_cast<float>((y & 15) - 8) + 0.5f;
            float ring = std::fabs(std::sqrt(dx * dx + dy * dy) - 5.0f);
            int shade = ring < 1.5f ? 15 : static_cast<int>(std::min(14.0f, ring * 2.0f));

            // The index: shade in bits 0-3, the tile in bit 4, the 32-texel quadrant in bit 5.
            texels[static_cast<std::size_t>(y * 64 + x)] =
                static_cast<u8>(tile * 16 + shade + ((x >> 5) ^ (y >> 5)) * 32);
        }
    }

    // The colour table: 256 entries of 32 bits, built from the index fields above.
    std::vector<u8> table(256 * 4);
    for (u32 i = 0; i < 256; i++) {
        u32 shade = i & 15, tile = (i >> 4) & 1, tint = (i >> 5) & 1;
        u32 r = tile ? 40 + shade * 12 : 200 - shade * 8;
        u32 g = tint ? 60 + shade * 10 : 150 - shade * 4;
        u32 b = tile ? 220 - shade * 6 : 50 + shade * 9;

        // The ring is more transparent: 0x40, where 0x80 is opaque.
        u32 a = shade == 15 ? 0x40 : 0x80;

        // A 256-entry table is stored with entries 8-15 and 16-23 of every 32 swapped.
        u32 position = (i & 0xE7) | ((i & 0x08) << 1) | ((i & 0x10) >> 1);
        store<u32>(&table[position * 4], r | (g << 8) | (b << 16) | (a << 24));
    }

    Uploads u;
    u.texture_address = kUploadAddress;
    u.texture_quadwords =
        write_upload(memory, u.texture_address, kTextureBlock, 1, PSMT8, 64, 64, texels);

    // The table goes 64 KB on, a 16 by 16 block of 32-bit pixels.
    u.clut_address = kUploadAddress + 0x10000;
    u.clut_quadwords = write_upload(memory, u.clut_address, kClutBlock, 1, PSMCT32, 16, 16, table);

    /*
     * The microprogram as a game stores one: the VIF code that loads it, then
     * the program, as whole quadwords ready to be referenced from a list.
     * MPG's NUM is the instruction count; an instruction is two words, and the codes before it
     * fill a quadword.
     */
    vuasm::Program program = vertex_program();
    std::vector<u32> blob =
        {kVifNop, kVifNop, kVifNop, kVifMpg | (static_cast<u32>(program.words().size() / 2) << 16)};
    blob.insert(blob.end(), program.words().begin(), program.words().end());

    // Pad to whole quadwords (4 words) with NOPs.
    blob.resize((blob.size() + 3) & ~std::size_t{3}, kVifNop);
    std::memcpy(memory.ram(kProgramAddress), blob.data(), blob.size() * 4);
    u.program_address = kProgramAddress;
    u.program_quadwords = static_cast<u32>(blob.size() / 4);
    return u;
}

/** A point in the scene, in camera space. */
struct Point {
    /** X to the right, Y up, Z away from the camera. */
    float x, y, z;
};

/**
 * A pinhole camera at the origin looking down +z, onto the 512 x 448 buffer
 * (whose pixels are wider than tall on a 4:3 screen).
 */
struct Projected {
    /** Screen position from the centre, in pixels (`x`, `y`), and 1/z (`q`). */
    float x, y, q;

    /** Depth for the Z buffer, 24 bits, larger when nearer. */
    u32 z;
};

/**
 * Projects a point onto the screen.
 *
 * @param p The point, with z greater than 0.
 * @return Its screen position, Q and depth.
 */
Projected project(Point p) {
    float q = 1.0f / p.z;
    Projected out;

    // 300 and 262 scale to pixels: the picture is 512 by 448 and its pixels are wider than tall.
    out.x = p.x * q * 300.0f;
    out.y = -p.y * q * 262.0f;
    out.q = q;

    // 16777215 is the largest 24-bit depth; nearer points have larger q and larger depth.
    out.z = static_cast<u32>(std::min(1.0f, q) * 16777215.0f);
    return out;
}

/**
 * Writes the display list of one frame.
 *
 * @param memory The guest memory to write the list into, at `kListAddress`.
 * @param uploads The uploads, referenced from the first frame's list.
 * @param frame The frame number; the scene moves with it, at 60 frames a second.
 */
void build_frame(GuestMemory& memory, const Uploads& uploads, int frame) {
    List list(memory, kListAddress);
    float time = static_cast<float>(frame) / 60.0f;

    // Drawing environment: one 32-bit colour buffer, a 24-bit Z buffer.
    list.begin_direct();
    list.gif_tag(11, true, 0, {0xE});
    list.ad(gsreg::PRMODECONT, 1);

    // FRAME: FBP bits 0-8, FBW 16-21, PSM 24-29; ZBUF: ZBP bits 0-8, PSM 24-27, value 1 is Z24.
    list.ad(gsreg::FRAME_1, kFramePage | (u64{kWidth / 64} << 16) | (u64{PSMCT32} << 24));
    list.ad(gsreg::ZBUF_1, kZPage | (u64{1} << 24));

    // XYOFFSET in 12.4; SCISSOR X1 at bit 16 and Y1 at bit 48, the whole picture.
    list.ad(gsreg::XYOFFSET_1, (kOffsetX * 16) | (u64{kOffsetY * 16} << 32));
    list.ad(gsreg::SCISSOR_1, (u64{kWidth - 1} << 16) | (u64{kHeight - 1} << 48));
    list.ad(gsreg::TEST_1, (u64{1} << 16) | (u64{1} << 17));  // depth test on, always pass
    list.ad(gsreg::COLCLAMP, 1);

    // TEXA: TA0 and TA1 (bit 32) both 0x80, opaque.
    list.ad(gsreg::TEXA, 0x80 | (u64{0x80} << 32));

    // FOGCOL: R in bits 0-7, G in 8-15, B in 16-23.
    list.ad(gsreg::FOGCOL, 0x30 | (0x48 << 8) | (0x70 << 16));
    list.ad(gsreg::ALPHA_1, 0x44);  // (Cs - Cd) * As + Cd
    list.ad(gsreg::CLAMP_1, 0);
    list.end_direct();

    // The texture and its table go up once, by reference to packets built
    // outside the list.
    if (frame == 0) {
        list.ref_direct(uploads.texture_address, uploads.texture_quadwords);
        list.ref_direct(uploads.clut_address, uploads.clut_quadwords);
    }

    // Clear colour and depth with one sprite: sky above, a darker band below.
    list.begin_direct();
    list.gif_tag(2, true, 0, {gsreg::RGBAQ, gsreg::XYZ2, gsreg::XYZ2}, true, kSprite);
    list.rgbaq(0x30, 0x48, 0x70, 0x80);
    list.xyz(-256, -224, 0);
    list.xyz(256, 0, 0);
    list.rgbaq(0x10, 0x18, 0x28, 0x80);
    list.xyz(-256, 0, 0);
    list.xyz(256, 224, 0);
    list.end_direct();

    // A textured floor running to the horizon: perspective-correct ST with Q,
    // bilinear filtering, fog by distance, depth tested.
    list.begin_direct();
    list.gif_tag(3, false, 0, {0xE});

    // TEST ZTE bit 16 on and ZTST bits 17-18 = 2 (GEQUAL).
    list.ad(gsreg::TEST_1, (u64{1} << 16) | (u64{2} << 17));  // pass when nearer or equal

    // TEX1: MMAG bit 5 and MMIN bit 6 both 1 (LINEAR).
    list.ad(gsreg::TEX1_1, (u64{1} << 5) | (u64{1} << 6));  // linear both ways

    // A 64 by 64 texture (log2 = 6), texture alpha used, modulate, table loaded.
    list.ad(gsreg::TEX0_1, tex0(kTextureBlock, 1, PSMT8, 6, 6, true, 0, kClutBlock, PSMCT32, 1));
    list.gif_tag(
        4,
        true,
        0,
        {gsreg::ST, gsreg::RGBAQ, gsreg::XYZF2},
        true,
        kTriangleStrip | kGouraud | kTextured | kFogged
    );
    const float scroll = time * 0.5f;

    // A quad 12 wide, 1 below the camera, from 1.2 to 14 away.
    const Point floor[4] = {{-6, -1, 1.2f}, {6, -1, 1.2f}, {-6, -1, 14}, {6, -1, 14}};
    for (const Point& p : floor) {
        Projected v = project(p);

        // Texture repeats every 2 units; the floor scrolls toward the camera with time.
        list.st(p.x * 0.5f * v.q, (p.z * 0.5f + scroll) * v.q, v.q);
        list.rgbaq(0x80, 0x80, 0x80, 0x80);

        // Fog fades with distance: full at 1.2, 19 less for each unit further, at least 0.
        u32 fog = static_cast<u32>(std::max(0.0f, 255.0f - (p.z - 1.2f) * 19.0f));
        list.xyzf(v.x, v.y, v.z, fog);
    }
    list.end_direct();

    /*
     * A cube drawn by VU1. The list loads the microprogram once (by reference,
     * as the games do), then each frame unpacks a matrix, a GIF tag and 36
     * vertices into VU1's memory and starts the program, which transforms the
     * vertices and kicks the packet itself.
     */
    if (frame == 0) {
        list.ref(uploads.program_address, uploads.program_quadwords, kVifNop, kVifNop);
    }
    {
        const float a = time * 0.9f, b = time * 0.6f;
        const float ca = std::cos(a), sa = std::sin(a), cb = std::cos(b), sb = std::sin(b);

        // Rotation about X then Y, and the cube's place in front of the camera.
        const float rot[3][3] = {{ca, sa * sb, sa * cb}, {0, cb, -sb}, {-sa, ca * sb, ca * cb}};
        const float place[3] = {-1.7f, 0.25f, 5.0f};

        // Six faces of two triangles, three vertices each.
        const u32 vertices = 36;

        /*
         * STCYCL with CL = WL = 1, then UNPACK V4-32 to address 0 relative to TOPS (bit 15). Its
         * count is the 6 quadwords of matrix, tag and count, then 3 for each vertex.
         */
        list.begin_data(
            kVifStcycl | 0x0101, kVifUnpackV4_32 | ((kVuVertices + vertices * 3) << 16) | 0x8000
        );

        /*
         * The matrix, a column a quadword. It takes a point straight to GS
         * coordinates times W: scaled for the screen, with W times the centre of
         * the coordinate space added so that the divide leaves the centre there.
         */
        for (int c = 0; c < 3; c++) {
            list.floats(
                300.0f * rot[0][c] + 2048.0f * rot[2][c],
                -262.0f * rot[1][c] + 2048.0f * rot[2][c],
                0.0f,
                rot[2][c]
            );
        }

        // The fourth column: the translation, with Z scaled to the 24-bit depth range.
        list.floats(
            300.0f * place[0] + 2048.0f * place[2],
            -262.0f * place[1] + 2048.0f * place[2],
            16777215.0f,
            place[2]
        );

        // The tag the program copies to the head of its packet.
        list.gif_tag(
            vertices,
            true,
            0,
            {gsreg::ST, gsreg::RGBAQ, gsreg::XYZ2},
            true,
            kTriangle | kGouraud | kTextured
        );

        // The vertex count the program loops on.
        list.words(vertices, 0, 0, 0);
        for (int face = 0; face < 6; face++) {
            // A face: an axis, a side, and the two axes that span it.
            int axis = face / 2, u_axis = (axis + 1) % 3, v_axis = (axis + 2) % 3;
            float side = (face & 1) ? 1.0f : -1.0f;

            // The two triangles of the face as corners of a unit square.
            const int corner[6][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 0}, {1, 1}, {0, 1}};
            for (const auto& c : corner) {
                // The cube's half-size is 0.8; a corner runs from -0.8 to 0.8 on the other axes.
                float position[3];
                position[axis] = side * 0.8f;
                position[u_axis] = (static_cast<float>(c[0]) * 2.0f - 1.0f) * 0.8f;
                position[v_axis] = (static_cast<float>(c[1]) * 2.0f - 1.0f) * 0.8f;

                // The three quadwords of a vertex: ST, colour, position (W = 1).
                list.floats(static_cast<float>(c[0]), static_cast<float>(c[1]), 1.0f, 0.0f);

                // Each face and corner gets its own grey, so the shape reads.
                u32 light =
                    0x58 + static_cast<u32>(face) * 0x0C + static_cast<u32>(c[0] + c[1]) * 0x10;
                list.words(light, light, light, 0x80);
                list.floats(position[0], position[1], position[2], 1.0f);
            }
        }
        list.end_data();

        // MSCAL at address 0 starts the program.
        list.begin_data(kVifMscal | 0, kVifNop);
        list.end_data();
    }

    // Two Gouraud triangles turning through each other: the Z buffer decides
    // every pixel along the line where they cross.
    list.begin_direct();
    list.gif_tag(6, true, 0, {gsreg::RGBAQ, gsreg::XYZ2}, true, kTriangle | kGouraud);
    const u32 colours[3][3] = {{0xFF, 0x30, 0x30}, {0x30, 0xFF, 0x30}, {0x30, 0x50, 0xFF}};
    for (int t = 0; t < 2; t++) {
        // The two turn at different speeds and in opposite directions.
        float turn = time * (t == 0 ? 1.1f : -0.8f) + static_cast<float>(t) * 1.3f;
        for (int i = 0; i < 3; i++) {
            // The corners of an equilateral triangle: 2.0943951 is 2 pi / 3 radians (120 degrees).
            float around = static_cast<float>(i) * 2.0943951f;
            Point p = {std::sin(around) * 1.1f, std::cos(around) * 1.1f + 0.3f, 0};

            // Turned about the vertical axis through (1.6, 4.6).
            Point r = {1.6f + p.x * std::cos(turn), p.y, 4.6f + p.x * std::sin(turn)};
            Projected v = project(r);
            const u32* c = colours[(i + t) % 3];
            list.rgbaq(c[0], c[1], c[2], 0x80);
            list.xyz(v.x, v.y, v.z);
        }
    }
    list.end_direct();

    /*
     * A head-up layer: a translucent panel, then the texture one texel per
     * pixel through UV coordinates, nearest filtering, table alpha blended.
     */
    list.begin_direct();
    list.gif_tag(2, false, 0, {0xE});
    list.ad(gsreg::TEST_1, (u64{1} << 16) | (u64{1} << 17));
    list.ad(gsreg::TEX1_1, 0);

    // The panel: a black sprite with alpha 0x50, 80 pixels square.
    list.gif_tag(1, false, 0, {gsreg::RGBAQ, gsreg::XYZ2, gsreg::XYZ2}, true, kSprite | kBlended);
    list.rgbaq(0x00, 0x00, 0x00, 0x50);
    list.xyz(-244, -212, 0);
    list.xyz(-244 + 80, -212 + 80, 0);

    // The texture, 64 by 64 pixels on the panel; UV 0.5 to 64.5 samples at texel centres.
    list.gif_tag(
        1,
        true,
        0,
        {gsreg::RGBAQ, gsreg::UV, gsreg::XYZ2, gsreg::UV, gsreg::XYZ2},
        true,
        kSprite | kTextured | kBlended | kUv
    );
    list.rgbaq(0x80, 0x80, 0x80, 0x80);
    list.uv(0.5f, 0.5f);
    list.xyz(-236, -204, 0);
    list.uv(64.5f, 64.5f);
    list.xyz(-236 + 64, -204 + 64, 0);
    list.end_direct();

    list.end();
}

/**
 * Writes an image as a binary PPM file.
 *
 * @param path The file to write.
 * @param image The picture, 8-bit RGBA with R in the low byte; alpha is dropped.
 * @return False if the file could not be opened.
 */
bool write_ppm(const std::string& path, const Image& image) {
    std::FILE* f = std::fopen(path.c_str(), "wb");

    // The caller reports it.
    if (!f) {
        return false;
    }

    std::fprintf(f, "P6\n%d %d\n255\n", image.width, image.height);
    for (u32 p : image.pixels) {
        u8 rgb[3] = {static_cast<u8>(p), static_cast<u8>(p >> 8), static_cast<u8>(p >> 16)};
        std::fwrite(rgb, 1, 3, f);
    }
    std::fclose(f);
    return true;
}

}  // namespace

/**
 * Runs the demo.
 *
 * Options: `--frames N` stops after N frames, `--headless` opens no window, `--ppm FILE` writes
 * the last frame. A headless run draws one frame unless told otherwise.
 *
 * @param argc Number of arguments.
 * @param argv The arguments.
 * @return 0 on success, 1 on a failure while running, 2 for a bad command line.
 */
int main(int argc, char** argv) {
    int frames = -1;
    bool headless = false;
    std::string ppm;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        // Each option that takes a value needs one more argument after it.
        if (arg == "--frames" && i + 1 < argc) {
            frames = std::atoi(argv[++i]);
        } else if (arg == "--ppm" && i + 1 < argc) {
            ppm = argv[++i];
        } else if (arg == "--headless") {
            headless = true;
        } else {
            // Anything else is a mistake: say how it is used.
            std::fprintf(stderr, "usage: openrac-gsdemo [--frames N] [--headless] [--ppm FILE]\n");
            return 2;
        }
    }
#ifdef OPENRAC_NO_WINDOW
    headless = true;
#endif

    // A headless run with no frame count would never end.
    if (headless && frames < 0) {
        frames = 1;
    }

    GuestMemory memory;
    Graphics graphics;
    Gs& gs = graphics.gs;
    Vif1& vif = graphics.vif;
    DmaChannel channel;
    Uploads uploads = prepare_uploads(memory);

    /*
     * One read circuit showing the colour buffer: 512 pixels across 2,560
     * video clock units, 448 lines.
     * PMODE bit 0 enables circuit 1; DISPFB1 has FBP, FBW at bit 9 and PSM at bit 15.
     */
    gs.write_privileged(gspriv::PMODE, 1);
    gs.write_privileged(
        gspriv::DISPFB1, kFramePage | (u64{kWidth / 64} << 9) | (u64{PSMCT32} << 15)
    );

    /*
     * DISPLAY1: MAGH (bit 23) 4 stretches each pixel over 5 clock units; DW (bit 32) and DH
     * (bit 44) are the sizes minus one.
     */
    gs.write_privileged(
        gspriv::DISPLAY1, (u64{4} << 23) | (u64{kWidth * 5 - 1} << 32) | (u64{kHeight - 1} << 44)
    );

#ifndef OPENRAC_NO_WINDOW
    host::Window window;

    // The window could not be opened; the window reports why.
    if (!headless && !window.open("OpenRAC software GS", 960, 720)) {
        return 1;
    }
#endif

    Image image;
    double total_ms = 0;
    int frame = 0;

    // Ends after the requested number of frames, or when the window is closed.
    for (; frames < 0 || frame < frames; frame++) {
#ifndef OPENRAC_NO_WINDOW
        // The window was closed.
        if (!headless && !window.pump()) {
            break;
        }
#endif
        auto start = std::chrono::steady_clock::now();
        build_frame(memory, uploads, frame);
        channel.tadr = kListAddress;
        channel.chcr = 0x1C5;  // from memory, chain mode, tag transfer and tag interrupts on, start

        // The walker sends the list's data to VIF1, as the DMA controller does for a game.
        DmaStop stop = run_source_chain(memory, channel, [&](const u8* data, std::size_t bytes) {
            vif.write(data, bytes);
        });

        // The list always ends in an END tag; anything else is a bug in the demo.
        if (stop != DmaStop::End) {
            std::fprintf(stderr, "the list did not end\n");
            return 1;
        }

        // Reading the picture waits for the drawing to finish; no circuit means nothing to show.
        if (!gs.display(image)) {
            std::fprintf(stderr, "no display circuit is on\n");
            return 1;
        }

        total_ms +=
            std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
                .count();
#ifndef OPENRAC_NO_WINDOW
        if (!headless) {
            window.present(image, 4.0f / 3.0f);
        }
#endif
    }

    // The summary needs at least one frame to average over.
    if (frame > 0) {
        std::printf(
            "%d frames, %.2f ms each to build, draw and read back; %llu primitives, %llu pixels, "
            "%llu transfers, "
            "%llu VU1 instructions\n",
            frame,
            total_ms / frame,
            static_cast<unsigned long long>(gs.stats.primitives),
            static_cast<unsigned long long>(gs.stats.pixels),
            static_cast<unsigned long long>(gs.stats.transfers),
            static_cast<unsigned long long>(graphics.vu1_instructions)
        );

        // Codes the model did not understand are reported, so a change that breaks the demo shows.
        if (vif.unknown_codes || graphics.vu1.unknown_ops) {
            std::fprintf(
                stderr,
                "unknown: %llu VIF codes, %llu VU instructions\n",
                static_cast<unsigned long long>(vif.unknown_codes),
                static_cast<unsigned long long>(graphics.vu1.unknown_ops)
            );
        }
    }

    // The last frame was asked for as a file.
    if (!ppm.empty() && !write_ppm(ppm, image)) {
        std::fprintf(stderr, "cannot write %s\n", ppm.c_str());
        return 1;
    }

    return 0;
}
