#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * loaders.cpp in the original source; text 0x202AA8-0x205520.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];
extern int D_0018A3B0[];
extern void func_001F99B0();
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;
extern char D_001940C0[];
extern long D_00151888[3];
extern int D_0015F6FC;
extern short D_0015F534;
extern void func_001FB530(void);
extern void func_001F3D78(void);
extern int D_0015F564;
extern int D_0018DD40[];
extern int D_0018DC40[];
extern short D_0015F59C;
extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];
extern void func_001F6668(void *, void *, void *, void *, void *, int,
                          unsigned char *);
extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);
extern int func_001F4868(int);
extern void func_001FB498(void);
extern void func_001F3008(void);
extern void func_001F3140(void);
extern int D_0018E840[];
extern long D_00152178 NOT_SDA;
extern int func_001FE4D0(void);
extern char D_00199A68[];
extern short D_0015F780;
extern int D_001941CC NOT_SDA;
extern int D_0019A4E8 NOT_SDA;
extern int func_001FF668(int);
typedef struct {
    char b[0x13];
} Cfg13;
extern Cfg13 D_0019A540 NOT_SDA;
extern Cfg13 D_001E7DD8 NOT_SDA;
extern int func_00116810(void);
extern void func_001166FC(Cfg13 *, void *);
extern short D_0015F9D0;
extern void func_00201960(int, int, int, int, int);
extern void func_002023E0(int);
extern void func_002027C0(int);

/*
 * UploadMipTexture: sends a texture (a header, then its palette and mip levels) to the GS at the
 * texture allocation cursor and writes its TEX0, MIPTBP1/TEX1 and MIPTBP1 values to regs[0..2].
 * Used for the planet pictures and area captions of the level transition. Adapted from Lombyte
 * (GPL-3.0): src/assembly/textbin/fun_00202270.c (US 0x00202270, the same function).
 */
typedef struct {
    u8 pad0[8];
    s32 width;
    s32 height;
    s32 psm;
    s32 palette_psm;
    s32 pad18;
    s32 mip_count;
    u8 data[4];
} MipTexture_202AA8;

typedef struct {
    s32 palette_address;
    s32 mip_addresses[4];
    s32 palette_size;
    s32 mip_sizes[4];
    s32 palette_block;
    s32 blocks[4];
    s32 buffer_widths[4];
    s32 width_log2;
    s32 height_log2;
} MipUpload_202AA8;

/* sceGsLoadImage: twelve doublewords. */
typedef struct {
    u64 q[12];
} LoadImage_202AA8 __attribute__((aligned(16)));

/* As loaders.c declares them further down (sceGsSetDefLoadImage, sceGsExecLoadImage, FlushCache,
 * sceGsSyncPath). */
extern int D_0015EF74 MACRO_ADDR; /* gs texture allocation cursor */
extern void func_001F99B0(void *, s32, s32);
extern s32 func_001F9968(s32);
extern void func_00118D80(int);
extern int func_00122630(void *, short, short, short, short, short, short, short);
extern int func_00122958(void *, void *);
extern int func_00120858(int, unsigned short);

s32 UploadMipTexture(MipTexture_202AA8 *tex, u64 *regs)
{
    MipUpload_202AA8 upload;
    LoadImage_202AA8 load_image;
    s32 i;
    s32 bytes;
    u64 tex0;
    u64 mip;
    u64 pal;
    u64 lod;

    FastMemSet(&upload, 0, sizeof(upload));
    switch (tex->psm) {
    case 0:
    case 2:
        upload.palette_address = 0;
        upload.palette_size = 0;
        break;
    case 0x13:
    case 0x14:
        upload.palette_address = (s32)tex->data;
        if (tex->psm == 0x14) {
            if (tex->palette_psm == 0) {
                upload.palette_size = 0x40;
            } else {
                upload.palette_size = 0x20;
            }
        } else {
            if (tex->palette_psm == 0) {
                upload.palette_size = 0x400;
            } else {
                upload.palette_size = 0x200;
            }
        }
        break;
    default:
        break;
    }
    upload.width_log2 = log2dim(tex->width);
    upload.height_log2 = log2dim(tex->height);
    upload.mip_addresses[0] = (s32)tex->data + upload.palette_size;
    switch (tex->psm) {
    case 0:
        upload.mip_sizes[0] = tex->width * tex->height * 4;
        break;
    case 2:
        upload.mip_sizes[0] = tex->width * tex->height * 2;
        break;
    case 0x13:
        upload.mip_sizes[0] = tex->width * tex->height;
        break;
    case 0x14:
        upload.mip_sizes[0] = (tex->width * tex->height) >> 1;
        break;
    }
    if (tex->psm == 0x13 || tex->psm == 0x14) {
        upload.palette_block = D_0015EF74 >> 8;
        if (tex->psm == 0x14) {
            D_0015EF74 += 0x100;
            func_00122630(&load_image, upload.palette_block, 1, tex->palette_psm, 0, 0, 8, 2);
        } else {
            D_0015EF74 += upload.palette_size;
            func_00122630(&load_image, upload.palette_block, 1, tex->palette_psm, 0, 0, 16, 16);
        }
        func_00118D80(0);
        func_00122958(&load_image, (void *)upload.palette_address);
        func_00120858(0, 0);
    }
    for (i = 1; i < tex->mip_count; i++) {
        upload.mip_sizes[i] = upload.mip_sizes[i - 1] >> 2;
        upload.mip_addresses[i] = upload.mip_addresses[i - 1] + upload.mip_sizes[i - 1];
    }
    for (i = 0; i < tex->mip_count; i++) {
                upload.buffer_widths[i] = tex->width >> (i + 6);
        if (upload.buffer_widths[i] <= 0) {
            upload.buffer_widths[i] = 1;
        }
        upload.blocks[i] = D_0015EF74 >> 8;
        func_00122630(&load_image, upload.blocks[i], upload.buffer_widths[i], tex->psm, 0, 0,
                      tex->width >> i, tex->height >> i);
        func_00118D80(0);
        func_00122958(&load_image, (void *)upload.mip_addresses[i]);
        func_00120858(0, 0);
        bytes = upload.mip_sizes[0] >> (i * 2);
        if (bytes <= 0xFF) {
            bytes = 0x100;
        }
        D_0015EF74 += bytes;
    }
    tex0 = (u64)upload.blocks[0];
    tex0 |= (u64)upload.buffer_widths[0] << 14;
    tex0 |= (u64)tex->psm << 20;
    tex0 |= (u64)upload.width_log2 << 26;
    tex0 |= (u64)upload.height_log2 << 30;
    pal = (u64)upload.palette_block << 37;
    pal |= (u64)1 << 34;
    tex0 |= pal;
    tex0 |= (u64)tex->palette_psm << 51;
    tex0 |= (u64)1 << 63;
    regs[0] = tex0;
    mip = (u64)upload.blocks[1];
    mip |= (u64)upload.buffer_widths[1] << 14;
    mip |= (u64)upload.blocks[2] << 20;
    mip |= (u64)upload.buffer_widths[2] << 34;
    lod = (u64)(tex->mip_count - 1) << 2;
    mip |= (u64)upload.blocks[3] << 40;
    mip |= (u64)upload.buffer_widths[3] << 54;
    regs[1] = lod | 0xFFA0000000E0ULL;
    regs[2] = mip;
    return -1;
}

LINKER_REMNANT("asm/remnants/text", func_00202EF8);

extern int D_001601C0 MACRO_ADDR;
extern char D_001CE500[];
extern int D_001CE300[];
typedef struct { int a; int b; } Pair_2F00;
extern Pair_2F00 D_001CDD00[];
extern void func_001F9A00(void *, void *, int);
extern int func_001F9968(int);

/* ParseParticleTexs */
void ParseParticleTexs(int *hdr, int base, int *list, int count) {
    int n = hdr[0];
    int off = hdr[2];
    int size = hdr[3];
    int *p = hdr + 4;
    int i;

    for (i = 0; i < n; i++, p++) {
        if (*p == 0) D_001CE300[i] = (int)D_001CE500;
        else D_001CE300[i] = *p - (off - (int)D_001CE500);
    }
    func_001F9A00(D_001CE500, (char *)hdr + off, size);
    for (D_001601C0 = 0; D_001601C0 < count; D_001601C0++) {
        int a = base + *list++;
        int b = *list++;
        int c = base + *list++;
        int d = *list++;
        D_001CDD00[D_001601C0].a = (a << 4) + b;
        D_001CDD00[D_001601C0].b = (c << 4) + log2dim(d);
    }
}

extern int func_001F9968(int);
extern int D_0015F55C MACRO_ADDR;
extern char D_0018D540[];

typedef struct {
    unsigned long flags;
    short y;
    short x;
    short u;
    short v;
} LoadPoint;
extern LoadPoint D_0018D540_p[] __asm__("D_0018D540");
extern short func_001F9968_s(int) __asm__("func_001F9968");

/* Unpack count point records (x, y, u, v words, 12.4 fixed point for
   x/y) into D_0018D540. Adapted from Lombyte (GPL-3.0) for PAL. */
void UnpackPointRecords(int *src, int count) {
    int x;
    int y;
    int u;
    int v;

    for (D_0015F55C = 0; D_0015F55C < count; D_0015F55C++) {
        x = *src++;
        y = *src++;
        u = *src++;
        v = *src++;
        D_0018D540_p[D_0015F55C].x = x >> 4;
        D_0018D540_p[D_0015F55C].y = y >> 4;
        D_0018D540_p[D_0015F55C].u = func_001F9968_s(u);
        D_0018D540_p[D_0015F55C].v = func_001F9968_s(v);
        D_0018D540_p[D_0015F55C].flags = 0;
    }
}

extern char *D_0016055C MACRO_ADDR;

typedef struct {
    long tag;
    short unk08;
    short unk0A;
    short unk0C;
    short unk0E;
} SkyPageL;
/* A shell is a row of 0x20-byte records: a header holding the count,
   then one record per item whose first word is an offset to relocate. */
typedef struct {
    char *ptr;
    char pad04[0x1C];
} SkyShellItemL;
typedef struct SkyShellL {
    int count;
    char pad04[0x1C];
} SkyShellL;
typedef struct {
    int unk00;
    short unk04;
    short count;      /* 0x06: shells */
    int unk08;
    short npages;     /* 0x0C */
    short unk0E;
    SkyPageL *pages;  /* 0x10 */
    char *unk14;
    char *unk18;
    char *unk1C;
    struct SkyShellL *shells[1]; /* 0x20 */
} SkyDefL;
extern SkyDefL *D_0016055C_s __asm__("D_0016055C") MACRO_ADDR;

/* Relocates the sky definition s just loaded (its pointers are offsets
   from s) and makes it the current one (D_0016055C): the page table and
   the +0x14/+0x18/+0x1C blocks (+0x1C only when present), then each GIF
   page, whose four words are rewritten in place as two 1/16 values, two
   func_001F9968 results and a cleared tag, then each shell and the items
   in it. The pages are reached through D_0016055C at every use, as
   retail reloads it after each call. The item pointer is a separate
   `sh + k` local: loop.c then reduces it to one register starting at sh
   and reaches the item at +0x20 from it, as retail does. */
void RelocateSkyDefinition(SkyDefL *s) {
    int i;

    D_0016055C_s = s;
    s->unk04 = 1;
    s->pages = (SkyPageL *)((char *)s->pages + (int)s);
    s->unk14 = s->unk14 + (int)s;
    s->unk18 = s->unk18 + (int)s;
    if (s->unk1C != 0) {
        s->unk1C = s->unk1C + (int)s;
    }
    {
        int *src = (int *)D_0016055C_s->pages;

        for (i = 0; i < D_0016055C_s->npages; i++) {
            int a = *src++;
            int b = *src++;
            int c = *src++;
            int d = *src++;

            D_0016055C_s->pages[i].unk0A = a >> 4;
            D_0016055C_s->pages[i].unk08 = b >> 4;
            D_0016055C_s->pages[i].unk0C = log2dim(c);
            D_0016055C_s->pages[i].unk0E = log2dim(d);
            D_0016055C_s->pages[i].tag = 0;
        }
    }
    {
        int j;

        for (j = 0; j < D_0016055C_s->count; j++) {
            SkyShellL *sh;
            int k;

            D_0016055C_s->shells[j] = (SkyShellL *)((char *)D_0016055C_s->shells[j] + (int)s);
            sh = D_0016055C_s->shells[j];
            for (k = 0; k < sh->count; k++) {
                SkyShellItemL *it = (SkyShellItemL *)sh + k;

                it[1].ptr = it[1].ptr + (int)s;
            }
        }
    }
}

typedef struct {
    int off;
    int size;
} BankSrc;

typedef struct {
    char pad00[0x20];
    BankSrc bank[6];
} SrcHdr;

typedef struct {
    int pad00;
    int unk04;
    int unk08;
    int unk0C;
    int unk10;
    char pad14[0x40];
    int unk54;
    int unk58;
    int unk5C;
    int unk60;
    int unk64;
    char pad68[0x2C];
    int unk94;
    int pad98;
    int unk9C;
    int unkA0;
    int unkA4;
} HudHdr;

typedef struct {
    char pad00[0x18];
    HudHdr *header;
    void *unk1C;
    void *unk20;
    void *unk24;
    void *unk28;
} HudArena;

extern SrcHdr *D_0015EF4C_src __asm__("D_0015EF4C") MACRO_ADDR;
extern HudArena D_0019A4E8_arena __asm__("D_0019A4E8");

extern char *func_001FFAB8_d(int, int, char *, int) __asm__("func_001FFAB8");
extern void func_00203548(int idx, int size);
extern int func_00234158(int arg0, int arg1, int arg2, int arg3);
extern void func_001FF958(int, void *, int);
extern void func_00118D80(int);
extern void func_001FF7F0(int bank, int addr);

extern int D_001941C0[];
extern int D_0019A520[];
extern char D_0015FC70[];
extern char D_0015FC80[];
extern char D_0015FC90[];
extern char D_0015FCA0[];
extern char D_0015FCB0[];

#define ALIGN64(x) (((x) + 0x3F) & 0xFFFFFFC0)

/* Builds the runtime HUD-bank arena from the compressed HUD image. Adapted from Lombyte (GPL-3.0) for PAL: ui/hud/load_hud_banks.c, load_hud_banks. */
void LoadHudBanks(void) {
    SrcHdr *base;
    int *out;
    int *p;
    int n;
    int v;
    int size24;
    HudHdr *header;
    int bank2;
    unsigned int shift54;
    int size58;
    char *bank58;
    unsigned int shift5C;
    unsigned int shift60;
    unsigned int shift64;
    char *fname;
    int *d941c0;

    base = D_0015EF4C_src;
    out = D_0019A520;
    p = &base->bank[1].size;
    for (n = 0; n < 5; n++) {
        *out++ = ALIGN64(*p);
        p += 2;
    }

    fname = D_0015FC70;
    d941c0 = D_001941C0;
    size24 = ALIGN64(base->bank[0].size);
    header = (HudHdr *) func_001FFAB8_d(size24, 0, fname, 0x23B);
    FastMemCopy(header, (void *) (base->bank[0].off + (int) base), size24);

    D_0019A4E8_arena.header = header;
    D_0019A4E8_arena.unk1C = (char *) header + header->unk04;
    bank2 = d941c0[2] + 0x60000;
    D_0019A4E8_arena.unk20 = (char *) header + header->unk08;
    D_0019A4E8_arena.unk28 = (char *) header + header->unk0C;
    D_0019A4E8_arena.unk24 = (char *) header + header->unk10;

    if (header->unk54 != 0) {
        shift54 = (unsigned int) ALIGN64(base->bank[1].size) >> 4;
        LoadCompressedHudBank(0, bank2);
        D_0019A4E8_arena.header->unk94 = Stash_SendData(
            base->bank[1].off + (int) base, shift54, shift54, (int) D_0015FC80);
        Hud_SendResidentBank(0, (void *) bank2, 1);
    }

    size58 = D_0019A4E8_arena.header->unk58;
    if (size58 != 0) {
        bank58 = func_001FFAB8_d(size58, 0, fname, 0x262);
        LoadCompressedHudBank(1, (int) bank58);
        func_00118D80(0);
        LinkHudBank(1, (int) bank58);
    }

    if (D_0019A4E8_arena.header->unk5C != 0) {
        shift5C = (unsigned int) ALIGN64(base->bank[3].size) >> 4;
        D_0019A4E8_arena.header->unk9C = Stash_SendData(
            base->bank[3].off + (int) base, shift5C, shift5C, (int) D_0015FC90);
    }

    if (D_0019A4E8_arena.header->unk60 != 0) {
        shift60 = (unsigned int) ALIGN64(base->bank[4].size) >> 4;
        D_0019A4E8_arena.header->unkA0 = Stash_SendData(
            base->bank[4].off + (int) base, shift60, shift60, (int) D_0015FCA0);
    }

    if (D_0019A4E8_arena.header->unk64 != 0) {
        shift64 = (unsigned int) ALIGN64(base->bank[5].size) >> 4;
        D_0019A4E8_arena.header->unkA4 = Stash_SendData(
            base->bank[5].off + (int) base, shift64, shift64, (int) D_0015FCB0);
    }
}

/*
 * The heap cursor. Retail reaches it with the one-register macro form
 * (both the load and the $at store), so it is MACRO_ADDR; D_0019A500
 * next to it is the ordinary split lui/%lo, so it stays plain.
 *
 * func_00203548 is 3/100: two addu operand orders are reversed
 * (`addu $2,$4,$2` and the $3/$4 pair in the tail). Every spelling of
 * both address expressions -- base-first, index-first, array indexing
 * on a cast pointer, the base hoisted into a local -- compiles to the
 * identical instruction stream, so this is the known
 * operand-order-is-not-source-steerable case.
 */
extern int *D_0015EF4C MACRO_ADDR;
extern char *D_0019A500;
extern void func_0020C468(int, int);

/* LoadCompressedHudBank(int, char *). Each scaled index in its own local
   gives retail's base-first addu; written inline, the multiply goes
   first. */
void LoadCompressedHudBank(int idx, int size) {
    if (((size + 0xF) & 0xFFFFFFF0) != 0) {
        int *base = D_0015EF4C;
        int off = idx * 8;
        func_0020C468(*(int *)((char *)base + off + 0x28) + (int)base, (size + 0xF) & 0xFFFFFFF0);
    }
    {
        int off = idx * 4;
        *(int *)(D_0019A500 + off + 0x74) = 0;
    }
}

typedef struct ResidentRenderTextureDefinition {
    s32 unk0; /* unused here; a word, so the record is 4-byte aligned */
    s16 width;
    s16 height;
    s16 draw_control_count;
    s16 clut_block_offset; /* 0x0A: TEX0.CBP (bit 37) after adding the base */
    s16 mip_block_offset_0;
    s16 mip_block_offset_1;
} ResidentRenderTextureDefinition;
extern s32 D_0015EF8C MACRO_ADDR;
extern u64 D_0019E7C0_035B0[3] __asm__("D_0019E7C0");
extern u64 D_0019E7D8_035B0[3] __asm__("D_0019E7D8");
extern s32 func_001F9968(s32);

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/rendering/build_indexed_resident_render_packet.c, build_indexed_resident_render_packet. */
void func_002035B0(u64 *packet,
                                          struct ResidentRenderTextureDefinition *texture,
                                          s32 draw_high, s32 draw_shift, s32 material_base,
                                          s32 material_shift, s32 material_index) {
    s32 width_units_128;
    s32 width_units_64;
    s32 width_log2;
    s32 height_log2;
    s32 gs_block_base;
    s32 texture_block;
    s32 mip_block_1;
    s32 mip_block_0;
    u64 *fallback_packet;
    u64 draw_control_word;
    u64 texture_word;
    u64 texture_address_word;
    u64 mip_word;
    u64 mip_address_word;
    s32 draw_control_count;
    u64 filter;
    u64 format;

    width_units_64 = texture->width >> 6;
    width_units_128 = texture->width >> 7;
    if (width_units_64 <= 0) {
        width_units_64 = 1;
    }
    if (width_units_128 <= 0) {
        width_units_128 = 1;
    }
    width_log2 = log2dim(texture->width);
    height_log2 = log2dim(texture->height);
    gs_block_base = D_0015EF8C >> 8;
    mip_block_1 = texture->mip_block_offset_1 + gs_block_base;
    texture_block = texture->clut_block_offset + gs_block_base;
    mip_block_0 = texture->mip_block_offset_0 + gs_block_base;
    /* Preserve sparse writes: every other 64-bit packet word is untouched. */
    draw_control_count = texture->draw_control_count;
    if (material_index >= 0) {
        filter = ((u64)draw_shift << 6) | 0x20;
        draw_control_word = ((u64)(draw_control_count - 1) << 2) | filter;
        draw_control_word |= (u64)draw_high << 32;
        packet[0] = draw_control_word;
        packet += 2;
        packet[0] = material_base | ((u64)material_shift << 2) | ((u64)material_index << 24);
        packet += 2;
        format = ((u64)width_log2 << 26) | 0x1300000;
        texture_word = (((u64)width_units_64 << 14) | format) | ((u64)height_log2 << 30);
        texture_address_word = ((u64)0x8000 << 19) | ((u64)texture_block << 37);
        mip_word = ((u64)width_units_128 << 14) | ((u64)mip_block_0 << 20);
        mip_address_word = ((u64)mip_block_1 << 40) | ((u64)0x8000 << 19);
        mip_word |= mip_address_word;
        texture_word |= texture_address_word;
        texture_word |= (u64)-1 << 63;
        mip_word |= (u64)0x8000 << 39;
        packet[0] = texture_word;
        packet[2] = mip_word;
    } else if (material_index < -1) {
        fallback_packet = D_0019E7C0_035B0;
        if (material_index == -3) {
            fallback_packet = D_0019E7D8_035B0;
        }
        packet[0] = ((u64)draw_shift << 6) | 0x20 | ((u64)draw_high << 32);
        packet += 2;
        packet[0] = 5;
        packet += 2;
        packet[0] = fallback_packet[0];
        packet[2] = fallback_packet[2];
    } else {
        packet[0] = ((u64)draw_shift << 6) | 0x20 | ((u64)draw_high << 32);
        packet += 2;
        packet[0] = 5;
        packet += 2;
        packet[0] = ((((u64)0x8000 << 29) | 0x9980) << 19) | 0x7FFB;
        packet[2] = 0;
    }
}

extern long D_0019E640[];
extern long D_0019E7C0[];
extern long D_0019E7D8[];

/* Fills four A+D qwords (low dwords only) at out: a TEX1-style word
   (MXL bits from the second word of row arg5 of the 3-dword table
   D_0019E640, MMAG 1, MMIN arg2, K arg1), then arg3 | arg4 << 2 |
   arg5 << 24, then the row's first and third words. A negative arg5 has
   no row: -2 and -3 take the rows D_0019E7C0 and D_0019E7D8 (and 5 for
   the second word), -1 a fixed constant and 0. The row is read before
   the sign test, as in retail. The OR operands are named locals so fold
   does not move the 0x20 to the end of the chain. */
void SetUpVisGifViewer(long *out, int arg1, int arg2, int arg3, int arg4, int arg5) {
    long f0 = D_0019E640[arg5 * 3];
    long f1 = D_0019E640[arg5 * 3 + 1];
    long f2 = D_0019E640[arg5 * 3 + 2];

    if (arg5 >= 0) {
        long t = ((long)arg2 << 6) | 0x20;

        *out = (f1 & 0x1C) | t | ((long)arg1 << 32);
        out += 2;
        *out = arg3 | ((long)arg4 << 2) | ((long)arg5 << 24);
        out += 2;
        *out = f0;
        out[2] = f2;
    } else if (arg5 < -1) {
        long *tpl = D_0019E7C0;

        if (arg5 == -3) {
            tpl = D_0019E7D8;
        }
        {
            long v = (long)arg2 << 6;
            long u = ((long)arg1 << 32) | 0x20;
            *out = v | u;
        }
        out += 2;
        *out = 5;
        out += 2;
        *out = tpl[0];
        out[2] = tpl[2];
    } else {
        {
            long v = (long)arg2 << 6;
            long u = ((long)arg1 << 32) | 0x20;
            *out = v | u;
        }
        out += 2;
        *out = 5;
        out += 2;
        *out = 0x80000004CC007FFBL;
        out[2] = 0;
    }
}
__asm__(".section .text\n\tnop\n");

extern int D_0015EF8C MACRO_ADDR;
extern int D_0015EF78 MACRO_ADDR;
extern int D_0015EF74 MACRO_ADDR;
/* libgraph: sceGsSetDefLoadImage, sceGsExecLoadImage, sceGsSyncPath;
   func_00118D80 is the kernel's FlushCache. */
extern int func_00122630(void *, short, short, short, short, short, short, short);
extern int func_00122958(void *, void *);
extern int func_00120858(int, unsigned short);

typedef struct {
    int type;   /* GS pixel format: 0x13 PSMT8, 2 PSMCT16, 0 PSMCT32 */
    int packed; /* width in the low half, height in the high half */
    int pad8;
    int size;   /* offset of the image data from the base */
} Chunk;

typedef struct {
    long w[12];
} GsLoadImage __attribute__((aligned(16)));

/* Uploads `count` images described by `list` into GS memory, starting at
   the VRAM cursor D_0015EF74 (reset from D_0015EF8C) and advancing it by
   each image's size, then records the end in D_0015EF78. An 8-bit image
   is w*h bytes (at least 0x100) with a buffer width of w/64 (at least 1);
   the other two formats are fixed 16x16 uploads. The prototypes are the
   SDK's (short arguments), the clamps are written `< 1` / `< 0x100` so the
   loop pass hoists their constants into saved registers, and the pointer
   advances in the for increment so the reversed counter's decrement
   lands in FlushCache's delay slot. */
void UploadTextureImages(int base, int count, Chunk *list) {
    GsLoadImage li;
    int i;

    D_0015EF78 = D_0015EF74 = D_0015EF8C;
    for (i = 0; i < count; i++, list++) {
        int addr = base + list->size;
        int packed = list->packed;
        int t = D_0015EF74;
        int w = packed & 0xFFFF;
        int h = packed >> 16;

        if (list->type == 0x13) {
            int dbw = w >> 6;
            int sz;

            if (dbw < 1) {
                dbw = 1;
            }
            func_00122630(&li, t >> 8, dbw, 0x13, 0, 0, w, h);
            sz = w * h;
            if (sz < 0x100) {
                sz = 0x100;
            }
            D_0015EF74 += sz;
        } else if (list->type == 2) {
            func_00122630(&li, t >> 8, 1, 2, 0, 0, 0x10, 0x10);
            D_0015EF74 += 0x200;
        } else if (list->type == 0) {
            func_00122630(&li, t >> 8, 1, 0, 0, 0, 0x10, 0x10);
            D_0015EF74 += 0x400;
        }
        func_00118D80(0);
        func_00122958(&li, (void *)addr);
        func_00120858(0, 0);
    }
    D_0015EF78 = D_0015EF74;
}

void relocateObjectPointers(char *arg0, int idx) {
    char *obj;
    int *p;
    int i;
    arg0 += idx * 4;
    obj = *(char **)(arg0 + 0x48);
    if (*(int *)(obj + 0x14) != 0) {
        *(int *)(obj + 0x14) = (int)(obj + *(int *)(obj + 0x14));
    }
    if (*(unsigned char *)(obj + 0x10) != 0) {
        i = 0;
        p = (int *)(obj + 0x1C);
        do {
            *p = (int)(obj + *p);
            i++;
            p++;
        } while (i < *(unsigned char *)(obj + 0x10));
    }
}

typedef u32 u128_03B70 __attribute__((mode(TI), aligned(16)));
typedef union MaterialMap_03B70 {
    u128_03B70 q;
    u8 b[16];
} MaterialMap_03B70;
typedef struct {
    s32 draw_high;
    s32 draw_shift;
    u8 pad8[0x8];
    s32 material_base;
    s32 material_shift;
    u8 pad18[0x8];
    s32 material_index;
    u8 pad24[0x1C];
} ResidentRenderPacket_03B70;
typedef struct {
    s32 blocks;
    s32 count;
    s32 auxiliary_data;
    s32 unused_C;
} ResidentRenderGroup_03B70;
typedef struct {
    u8 first_selector;
    u8 pad1[0xB];
    s32 target;
} MaterialRun_03B70;
typedef struct {
    u8 pad0[0x10];
    u8 count;
    u8 pad11[3];
    s32 optional_data_14;
    u8 pad18[4];
    s32 entry_offsets[1];
} NestedRenderTable_03B70;
typedef struct {
    s32 groups;
    u8 group_count_0;
    u8 group_count_1;
    u8 group_count_2;
    u8 pad7[5];
    u8 nested_table_count;
    u8 padD[3];
    s32 optional_table_10;
    s32 optional_data_14;
    s32 optional_table_18;
    s32 *counted_pointers;
    s32 material_runs;
    u8 pad24[4];
    s32 runtime_table;
    u8 pad2C[0x1C];
    s32 nested_tables[1];
} ResidentClassRenderHeader_03B70;
extern u8 D_001B3E40_03B70[] __asm__("D_001B3E40");
extern MaterialMap_03B70 D_001B6C00_03B70[] __asm__("D_001B6C00");
extern void func_002035B0_03B70(ResidentRenderPacket_03B70 *, void *, s32, s32, s32, s32, s32) __asm__("func_002035B0");
extern void func_00203808_03B70(ResidentRenderPacket_03B70 *packet, s32 draw_high, s32 draw_shift, s32 material_base, s32 material_shift, s32 material_index) __asm__("func_00203808");

/* Adapted from Lombyte (GPL-3.0) for PAL by OpenRAC's tools/port.py: src/textbin/prepare_resident_class_render_data.c, prepare_resident_class_render_data. */
void func_00203B70_r(ResidentClassRenderHeader_03B70 *header, u8 *textures, u8 *material_map, s32 class_id)
    __asm__("func_00203B70");
void func_00203B70_r(ResidentClassRenderHeader_03B70 *header, u8 *textures, u8 *material_map, s32 class_id) {
    s32 group_count;
    s32 class_slot;
    s32 group_index;
    s32 packet_quadword;
    s32 groups_remaining;
    s32 packet_extent;
    ResidentRenderGroup_03B70 *group;
    ResidentRenderGroup_03B70 *relocation_group;
    MaterialRun_03B70 *material_run;
    u8 *selector;
    NestedRenderTable_03B70 *nested_table;
    s32 *entry_offset;
    MaterialMap_03B70 *slot_materials;
    ResidentRenderPacket_03B70 *packet;
    s32 packed_extent;
    s32 packet_start;
    s32 material_index;
    s32 pointer_count;
    s32 pointer_index;
    s32 nested_table_index;
    s32 nested_entry_index;

    /* Serialized pointers are relative to the entire class blob. */
    group_count = header->group_count_0 + header->group_count_1 + header->group_count_2;
    if (header->groups != 0) {
        header->groups = (s32)header + header->groups;
        relocation_group = (ResidentRenderGroup_03B70 *)header->groups;
        if (group_count != 0) {
            groups_remaining = group_count;
            do {
                relocation_group->blocks += (s32)header;
                relocation_group->auxiliary_data += (s32)header;
                groups_remaining--;
                relocation_group++;
            } while (groups_remaining != 0);
        }
    }
    if (header->optional_table_10 != 0) {
        header->optional_table_10 = (s32)header + header->optional_table_10;
    }
    if (header->optional_data_14 != 0) {
        header->optional_data_14 = (s32)header + header->optional_data_14;
    }
    if (header->optional_table_18 != 0) {
        header->optional_table_18 = (s32)header + header->optional_table_18;
    }
    if (header->counted_pointers != 0) {
        header->counted_pointers = (s32 *)((u8 *)header + (s32)header->counted_pointers);
        pointer_count = header->counted_pointers[0];
        for (pointer_index = 0; pointer_index < pointer_count; pointer_index++) {
            header->counted_pointers[pointer_index + 1] += (s32)header;
        }
    }
    if (header->material_runs != 0) {
        header->material_runs = (s32)header + header->material_runs;
        material_run = (MaterialRun_03B70 *)header->material_runs;
        do {
            material_run->target += (s32)header;
            selector = &material_run->first_selector;
            if (material_run->first_selector != 0xFF) {
                do {
                    *selector = material_map[*selector];
                    selector++;
                } while (*selector != 0xFF);
            }
        } while (material_run->target >= 0 && (material_run++, 1));
    }
    if (header->runtime_table != 0) {
        header->runtime_table = (s32)header + header->runtime_table;
    }
    for (nested_table_index = 0; nested_table_index < header->nested_table_count;
         nested_table_index++) {
        if (header->nested_tables[nested_table_index] != 0) {
            nested_table =
                (NestedRenderTable_03B70 *)((u8 *)header + header->nested_tables[nested_table_index]);
            header->nested_tables[nested_table_index] = (s32)nested_table;
            if (nested_table->optional_data_14 != 0) {
                nested_table->optional_data_14 = (s32)header + nested_table->optional_data_14;
            }
            for (nested_entry_index = 0; nested_entry_index < nested_table->count;
                 nested_entry_index++) {
                nested_table->entry_offsets[nested_entry_index] =
                    (s32)header + nested_table->entry_offsets[nested_entry_index];
            }
        }
    }

    class_slot = D_001B3E40_03B70[class_id];
    slot_materials = &D_001B6C00_03B70[class_slot];
    qcopy(slot_materials, material_map);
    group = (ResidentRenderGroup_03B70 *)header->groups;
    for (group_index = 0; group_index < group_count; group_index++, group++) {
        /* High half counts encoded quadwords; low half locates the packet end. */
        packed_extent = group->count;
        packet_extent = packed_extent >> 16;
        packet_start = packed_extent & 0xFFFF;
        group->count = packet_start;
        packet = (ResidentRenderPacket_03B70 *)(group->blocks + (packet_start - packet_extent) * 16);
        for (packet_quadword = 0; packet_quadword < packet_extent; packet_quadword += 4) {
            material_index = packet->material_index;
            if (material_index >= 0) {
                material_index = slot_materials->b[material_index];
            }
            if (textures != 0) {
                func_002035B0_03B70(
                    packet, textures + material_index * 16, packet->draw_high, packet->draw_shift,
                    packet->material_base, packet->material_shift, material_index);
            } else {
                func_00203808_03B70(packet, packet->draw_high, packet->draw_shift,
                                                      packet->material_base, packet->material_shift,
                                                      material_index);
            }
            packet++;
        }
    }
}

extern int D_00160000 MACRO_ADDR;
extern unsigned char D_001B3E40[] NOT_SDA;
extern short D_001B3C80[];
extern char *D_001B3580[] NOT_SDA;
extern int D_001B6500[];
extern void func_00213BB8(int);
extern void func_00203B70(void *, int, int, int);

/* Registers class arg3's data arg0 in the next moby-class slot
   (D_00160000): class -> slot in D_001B3E40, slot -> class in
   D_001B3C80, slot -> data in D_001B3580; then func_00213BB8(arg3) and
   the slot count is bumped. A non-null arg0 also has its +0x2C field
   kept in D_001B6500 and is set up by func_00203B70 once the count is
   bumped. The slot byte is D_00160000 read again as a byte. */
void RegisterMobyClass(void *arg0, int arg1, int arg2, int arg3) {
    int idx = D_00160000;

    D_001B3E40[arg3] = *(unsigned char *)&D_00160000;
    D_001B3C80[idx] = arg3;
    D_001B3580[idx] = arg0;
    if (arg0 == 0) {
        SetMobyClassEntry(arg3);
        D_00160000 = D_00160000 + 1;
    } else {
        D_001B6500[idx] = *(int *)((char *)arg0 + 0x2C);
        SetMobyClassEntry(arg3);
        D_00160000 = D_00160000 + 1;
        MobyClassRelocate(arg0, arg1, arg2, arg3);
    }
}
__asm__(".section .text\n\tnop\n");

/*
 * tie_ad_gif_convert (written from the assembly). Records the tie ad self in
 * the table D_001E1A00 at the count D_0016104C, stores the count and the ad's
 * index, scales the float at +0x48 by 1024 for func_001FA898, and relocates the
 * three pointers at +0 .. +8 and the pair at +0xC and +0x2C. Then, for each of
 * the self's entries (count at +0x23, 0x50 bytes each, from the pointer at +0x2C),
 * writes four 64-bit GIF words from the record at a1 (index idx in D_001E3900)
 * or, when a1 is 0, from D_0019E640.
 */
extern int D_0016104C MACRO_ADDR;
extern char D_001E1C00[] NOT_SDA;
extern char D_001E1D00[] NOT_SDA;
extern char D_001E1A00[] NOT_SDA;
extern char D_001E3100[] NOT_SDA;
extern char D_001E3900[] NOT_SDA;
extern int func_001FA898(float);

void func_00203F68(char *self, int a1, char *src, int a3) {
    int N;
    int N2;
    int R;
    float f;
    int k;
    int j;
    int P;
    int v;
    char *m;
    char *cur;
    char *dest;
    int idx;
    int local4;
    int local8;
    int local12;
    int t23;
    int F1;
    int F2;
    int H;
    int W;
    int t4;
    int K;
    int c7;
    int c9;
    int S8;
    int i;
    int jj;
    unsigned long D0;
    unsigned long T;
    unsigned long D1;
    unsigned long D2;
    unsigned long D3;
    unsigned long dw4;
    unsigned long dw7;
    unsigned long dw8;
    unsigned long c4p;
    unsigned long c7p;
    char *rec;
    char *E;

    *(short *)(D_001E1C00 + D_0016104C * 2) = a3;
    *(unsigned char *)(D_001E1D00 + a3) = D_0016104C;
    *(char **)(D_001E1A00 + D_0016104C * 4) = self;
    *(short *)(self + 0x46) = D_0016104C;
    R = func_001FA898(*(float *)(self + 0x48) * 1024.0f);
    N2 = D_0016104C;
    D_0016104C = N2 + 1;
    *(int *)(D_001E3100 + 4 * N2) = R;

    *(int *)(self + 0x28) = 0;
    *(short *)(self + 0x26) = 0;

    for (k = 0; k < 3; k++) {
        if (((struct { int p[3]; } *)self)->p[k] != 0) {
            ((struct { int p[3]; } *)self)->p[k] += (int)self;
            for (jj = 0; jj < ((struct { char p[0x20]; unsigned char c[4]; } *)self)->c[k]; jj++) {
                ((int *)((struct { int p[3]; } *)self)->p[k])[jj * 4] += ((struct { int p[3]; } *)self)->p[k];
            }
        }
    }

    v = *(int *)(self + 0x0C) + (int)self;
    m = (char *)(*(int *)(self + 0x2C) + (int)self);
    *(int *)(self + 0x0C) = v;
    *(int *)(self + 0x2C) = (int)m;

    dest = D_001E3900 + ((*(unsigned char *)(D_001E1D00 + a3)) << 4);
    qcopy(dest, src);

    for (j = 0, cur = m; j < *(unsigned char *)(self + 0x23); j++, cur += 0x50) {
        local4 = *(int *)(cur + 0x10);
        idx = *(unsigned char *)(dest + j);
        t23 = *(int *)(cur + 0x14);
        local8 = *(int *)(cur + 0x30);
        local12 = *(int *)(cur + 0x34);

        if (a1 != 0) {
            int h, w; int f1; long HH, WW; long G;
            rec = (char *)a1 + (idx << 4);
            t4 = (int)((unsigned int)*(unsigned short *)(rec + 4) << 16);
            h = t4 >> 22;
            w = t4 >> 23;
            if (!(0 < h)) {
                h = 1;
            }
            if (!(0 < w)) {
                w = 1;
            }
            f1 = func_001F9968(t4 >> 16);
            F2 = func_001F9968(*(short *)(rec + 6));
            K = D_0015EF8C >> 8;
            c7 = *(short *)(rec + 0x0E) + K;
            c9 = *(short *)(rec + 0x0C) + K;
            t4 = *(short *)(rec + 0x0A) + K;
            S8 = *(short *)(rec + 0x08);

            HH = ((unsigned long)(long)h << 14) | ((unsigned long)0x13 << 20) | ((unsigned long)(long)f1 << 26)
               | ((unsigned long)(long)F2 << 30) | ((unsigned long)1 << 34)
               | ((unsigned long)(long)t4 << 37) | ((unsigned long)4 << 61);
            D1 = ((unsigned long)(long)(S8 - 1) << 2) | ((unsigned long)1 << 5)
               | ((unsigned long)(long)t23 << 6) | ((unsigned long)(long)local4 << 32);
            WW = ((unsigned long)(long)w << 14) | ((unsigned long)(long)c9 << 20)
               | ((unsigned long)1 << 34) | ((unsigned long)(long)c7 << 40)
               | ((unsigned long)1 << 54);
            D3 = (unsigned long)(long)local8 | ((unsigned long)(long)local12 << 2) | ((unsigned long)(long)idx << 24);

            *(unsigned long *)(cur + 0x00) = HH;
            *(unsigned long *)(cur + 0x10) = D1;
            *(unsigned long *)(cur + 0x20) = WW;
            *(unsigned long *)(cur + 0x30) = D3;
            *(unsigned long *)(cur + 0x40) = 0;
            continue;
        } else {
            unsigned long T2, E1, E3;
            dw7 = *(unsigned long *)((char *)D_0019E640 + 24 * idx);
            i = idx * 3;
            dw4 = *(unsigned long *)((char *)D_0019E640 + ((i + 1) << 3));
            dw8 = *(unsigned long *)((char *)D_0019E640 + ((i + 2) << 3));
            T2 = ((unsigned long)(long)t23 << 6) | 0x20UL;
            E1 = (dw4 & 0x1CUL) | T2 | ((unsigned long)(long)local4 << 32);
            E3 = (unsigned long)(long)local8 | ((unsigned long)(long)local12 << 2) | ((unsigned long)(long)idx << 24);
            *(unsigned long *)(cur + 0x00) = dw7;
            *(unsigned long *)(cur + 0x10) = E1;
            *(unsigned long *)(cur + 0x20) = dw8;
            *(unsigned long *)(cur + 0x30) = E3;
            *(unsigned long *)(cur + 0x40) = 0;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/text", func_00204340);

typedef struct { int x0; int x4; float scale; } FontHdr_04918;
typedef struct {
    char pad0[0x10];
    int glyphs;            /* 0x10 */
    char pad14[8];
    unsigned short off1C;  /* 0x1C */
    char pad1E[0xA];
    unsigned char count;   /* 0x28 */
    char pad29[0x17];
} FontRun_04918;
typedef struct {
    int x0;
    char pad4[0xC];
    int x10;
    int x14;
    char pad18[8];
    int x20;
    int x24;
    char pad28[0x28];
} Glyph_04918;
typedef struct { long lo; long hi; } GifQ_04918;
typedef struct { short x0; short x2; unsigned short x4; short x6; short x8; short xA; short xC; short xE; } GlyphSrc_04918;
extern int D_00160F90 MACRO_ADDR;
extern float D_00160FA0[3];
extern char *D_00160F8C MACRO_ADDR;
extern void func_00234380(void);

void func_00204918(FontHdr_04918 *hdr, char *tbl) {
    char *base;
    float scale;
    int i;
    int j;
    int n;
    int k;

    D_00160F90 = *(int *)((char *)hdr + 4);
    scale = *(float *)((char *)hdr + 8);
    D_00160FA0[0] = scale * 6.0f;
    D_00160FA0[1] = scale * 4.0f;
    D_00160FA0[2] = scale + scale;
    func_00234380();
    base = (char *)hdr + *(int *)hdr;
    D_00160F8C = base;
    n = D_00160F90;
    for (k = 0; k < n; k++) {
        *(int *)(base + k * 0x40 + 0x10) = (int)base + *(int *)(base + k * 0x40 + 0x10);
    }
    for (i = 0; i < D_00160F90; i++) {
        for (j = 0; j < ((FontRun_04918 *)D_00160F8C)[i].count; j++) {
            FontRun_04918 *run = (FontRun_04918 *)((i << 6) + (int)D_00160F8C);
            unsigned long *o = (unsigned long *)((char *)run->glyphs + run->off1C + j * 0x50);
            int k0 = ((Glyph_04918 *)o)->x0;
            int k10 = ((Glyph_04918 *)o)->x10;
            int k14 = ((Glyph_04918 *)o)->x14;
            int k20 = ((Glyph_04918 *)o)->x20;
            int k24 = ((Glyph_04918 *)o)->x24;
            GlyphSrc_04918 *m = (GlyphSrc_04918 *)(tbl + k0 * 16);
            int t = m->x4 << 16;
            int a17 = t >> 22;
            int a19 = t >> 23;
            long u;
            int v;
            int gs;
            unsigned long tex0, tex1, clamp, mip1;

            if (!(0 < a17)) {
                a17 = 1;
            }
            if (!(0 < a19)) {
                a19 = 1;
            }
            u = func_001F9968(t >> 16);
            v = func_001F9968(m->x6);
            gs = D_0015EF8C >> 8;
            tex0 = (unsigned long)0 | ((unsigned long)a17 << 14) | ((unsigned long)0x13 << 20)
                  | ((unsigned long)u << 26) | ((unsigned long)v << 30) | ((unsigned long)1 << 34)
                  | ((unsigned long)0 << 35) | ((unsigned long)(m->xA + gs) << 37)
                  | ((unsigned long)0 << 51) | ((unsigned long)0 << 55) | ((unsigned long)0 << 56)
                  | ((unsigned long)4 << 61);
            t = m->x8 - 1;
            tex1 = (unsigned long)0 | ((unsigned long)t << 2) | ((unsigned long)1 << 5)
                  | ((unsigned long)k14 << 6) | ((unsigned long)0 << 9) | ((unsigned long)0 << 19)
                  | ((unsigned long)(unsigned int)k10 << 32);
            clamp = (unsigned long)k20 | ((unsigned long)k24 << 2) | ((unsigned long)0 << 4)
                  | ((unsigned long)0 << 14) | ((unsigned long)k0 << 24) | ((unsigned long)0 << 34);
            mip1 = (unsigned long)0 | ((unsigned long)a19 << 14) | ((unsigned long)(m->xC + gs) << 20)
                  | ((unsigned long)1 << 34) | ((unsigned long)(m->xE + gs) << 40)
                  | ((unsigned long)1 << 54);
            *o = tex0;
            o += 2;
            *o = tex1;
            o += 2;
            *o = clamp;
            o += 2;
            *o = mip1;
            ((GifQ_04918 *)o)[1].lo = 0;
        }
    }
}

extern int D_00137C80[];
extern char D_1FF7FF0[];
extern int func_002175C8(int, int, int);

int Level_StartDataRead(void) {
    int *hdr = D_00137C80;
    int want = ((hdr[0x13F] << 11) + 0x1057) & 0xFFFFF000;

    D_0015EF4C = (int *)(((int)D_1FF7FF0 - want) & -0x10);
    *D_0015EF4C = 0x60;
    Stream_SubmitRead((int)D_0015EF4C + *D_0015EF4C, hdr[0x13E], hdr[0x13F]);
    return 1;
}

struct DiscFile_04C60 {
    s32 sector;
    s32 size;
};
struct DiscTable_04C60 {
    u8 pad_0[0x12C8];
    struct DiscFile_04C60 level_archives[24]; /* 0x12C8, by level index */
    u8 pad_1388[0x15E0];
    struct DiscFile_04C60 shared_archive;     /* 0x2968 */
    struct DiscFile_04C60 sound_archive;      /* 0x2970 */
    struct DiscFile_04C60 sound_archive_alt;  /* 0x2978, while D_0015EE80 is set */
};
struct LevelArchiveDiscEntry_04C60 {
    u8 pad_0[0x12C8];
    s32 start_sector;
    s32 sector_count;
};
struct LevelArchiveHeader_04C60 {
    u8 pad_0[8];
    s32 sound_bank_offset;
};

extern struct DiscTable_04C60 D_00137C80_04C60 __asm__("D_00137C80");
extern s16 D_0013E156_04C60[] __asm__("D_0013E156");
extern s32 D_0015EE58_04C60 __asm__("D_0015EE58") MACRO_ADDR;
extern u32 D_0015EE5C_04C60 __asm__("D_0015EE5C") MACRO_ADDR;
extern s32 D_0015EE80_04C60 __asm__("D_0015EE80") MACRO_ADDR;
/* The load's state is one small-data object: the stage at its start is reached in one
   instruction (through $gp in a delay slot), the members behind it never are. */
struct LevelArchiveLoad_04C60 {
    u16 stage;                                /* D_0015EF48 */
    s16 busy;                                 /* D_0015EF4A */
    struct LevelArchiveHeader_04C60 *shared;  /* D_0015EF4C */
    u8 *sound;                                /* D_0015EF50 */
    u8 *level;                                /* D_0015EF54 */
};
extern struct LevelArchiveLoad_04C60 D_0015EF48_04C60 __asm__("D_0015EF48") MACRO_ADDR;
extern s32 D_0015EFBC_04C60 __asm__("D_0015EFBC") MACRO_ADDR;
extern s32 D_0015EFC0_04C60 __asm__("D_0015EFC0") MACRO_ADDR;
extern u8 D_1FF8000_04C60[] __asm__("D_1FF8000");
extern void func_0022F090_04C60() __asm__("func_0022F090");
extern s32 func_0012DDC0_04C60() __asm__("func_0012DDC0");
extern void func_0012E1C8_04C60(s32, s32, u64) __asm__("func_0012E1C8");
extern void func_0012E2E8_04C60() __asm__("func_0012E2E8");
extern s32 func_0012E318_04C60(s32) __asm__("func_0012E318");
extern void func_0012E4F8_04C60() __asm__("func_0012E4F8");
extern s32 func_002175C8_04C60(void *, u32, u32) __asm__("func_002175C8");
extern s32 func_001219C8_04C60() __asm__("func_001219C8");
extern s32 func_00121930_04C60() __asm__("func_00121930");
extern s32 func_00120F30_04C60(s32) __asm__("func_00120F30");

/* One step of the level archive load: waits for the disc, steps back a stage on a read
   error or a 720-frame stall, then by stage reads the shared, level and sound archives
   below 0x1FF8000 and swaps the level's sound bank. Returns 1 when everything is in.
   Adapted from Lombyte (GPL-3.0) for PAL: src/gameplay/state/fun_00204428.c, service_level_archive_load. */
s32 LevelArchiveLoadStep(void) {
    s32 stage;
    u16 next_stage;
    s32 retry_stage;
    s32 archive_start_or_bytes;
    s32 sound_aligned_bytes;
    s32 level_archive_sectors;
    s32 archive_start_or_sectors;
    s32 shared_start_sector;
    u8 *sound_archive_buffer;
    u8 *level_archive_buffer;
    u8 *shared_archive_buffer;
    struct LevelArchiveDiscEntry_04C60 *disc_entry;
    struct LevelArchiveDiscEntry_04C60 *next_disc_entry;
    s32 level_index;
    struct LevelArchiveHeader_04C60 *shared_header;

    level_index = D_0013E156_04C60[0] + 1;
    if (func_00120F30_04C60(1) != 0) {
        D_0015EFBC_04C60 = D_0015EFBC_04C60 + 1;
        if (D_0015EE58_04C60 == 1) {
            if (D_0015EFBC_04C60 >= 0x2D1) {
                D_0015EFC0_04C60 = D_0015EE58_04C60;
                retry_stage = D_0015EF48_04C60.stage - 1;
                D_0015EE58_04C60 = 0;
                if ((u16)retry_stage < 3) {
                    D_0015EF48_04C60.stage = retry_stage;
                }
                func_001219C8_04C60();
            }
        }
        return 0;
    }
    if (func_00121930_04C60() != 0) {
        if (D_0015EFC0_04C60 == 0) {
            D_0015EFC0_04C60 = 1;
            retry_stage = D_0015EF48_04C60.stage - 1;
            D_0015EE58_04C60 = 0;
            if ((u16)retry_stage < 3) {
                D_0015EF48_04C60.stage = retry_stage;
            }
        }
    }
    stage = (s16)D_0015EF48_04C60.stage;
    switch (stage) {
    case 0:
        if (D_0015EE80_04C60 != 0) {
            sound_aligned_bytes =
                ((D_00137C80_04C60.sound_archive_alt.size << 11) + 0xFFF) & 0xFFFFF000;
        } else {
            sound_aligned_bytes = ((D_00137C80_04C60.sound_archive.size << 11) + 0xFFF) & 0xFFFFF000;
        }
        disc_entry = (struct LevelArchiveDiscEntry_04C60 *)((u8 *)&D_00137C80_04C60 + level_index * 8);
        level_archive_sectors = *(s32 *)((u8 *)&D_00137C80_04C60 + level_index * 8 + 0x12CC);
        sound_archive_buffer = D_1FF8000_04C60 - sound_aligned_bytes;
        sound_aligned_bytes = ((level_archive_sectors << 11) + 0xFFF) & 0xFFFFF000;
        level_archive_buffer = sound_archive_buffer - sound_aligned_bytes;
        archive_start_or_sectors = D_00137C80_04C60.shared_archive.size;
        sound_aligned_bytes = ((archive_start_or_sectors << 11) + 0xFFF) & 0xFFFFF000;
        shared_archive_buffer = level_archive_buffer - sound_aligned_bytes;
        archive_start_or_bytes = D_00137C80_04C60.shared_archive.sector;
        D_0015EF48_04C60.level = level_archive_buffer;
        D_0015EF48_04C60.sound = sound_archive_buffer;
        D_0015EF48_04C60.shared = (struct LevelArchiveHeader_04C60 *)shared_archive_buffer;
        func_002175C8_04C60(shared_archive_buffer, archive_start_or_bytes,
                            archive_start_or_sectors);
        D_0015EF48_04C60.stage = D_0015EF48_04C60.stage + 1;
        break;
    case 1:
        next_disc_entry = (struct LevelArchiveDiscEntry_04C60 *)((u8 *)&D_00137C80_04C60 + level_index * 8);
        func_002175C8_04C60(D_0015EF48_04C60.level, *(s32 *)((s32)&D_00137C80_04C60 + (level_index << 3) + 0x12C8), *(s32 *)((s32)&D_00137C80_04C60 + (level_index << 3) + 0x12CC));
        D_0015EF48_04C60.stage = D_0015EF48_04C60.stage + 1;
        break;
    case 2:
        if (D_0015EE80_04C60 != 0) {
            func_002175C8_04C60(D_0015EF48_04C60.sound, D_00137C80_04C60.sound_archive_alt.sector,
                                D_00137C80_04C60.sound_archive_alt.size);
        } else {
            func_002175C8_04C60(D_0015EF48_04C60.sound, D_00137C80_04C60.sound_archive.sector,
                                D_00137C80_04C60.sound_archive.size);
        }
        D_0015EF48_04C60.stage = D_0015EF48_04C60.stage + 1;
        break;
    case 3:
        if (D_0015EF48_04C60.busy != 0) {
            return 0;
        }
        func_0012E4F8_04C60();
        if (D_0015EE5C_04C60 != 0) {
            next_stage = D_0015EF48_04C60.stage;
            D_0015EF48_04C60.stage = next_stage + 1;
        } else {
            D_0015EF48_04C60.stage = 6;
        }
        break;
    case 4:
        if (func_0012DDC0_04C60() != 0) {
            return 0;
        }
        func_0012E318_04C60(D_0015EE5C_04C60);
        next_stage = D_0015EF48_04C60.stage;
        D_0015EE5C_04C60 = 0;
        D_0015EF48_04C60.stage = next_stage + 1;
        break;
    case 5:
        if (func_0012DDC0_04C60() != 0) {
            return 0;
        }
        func_0012E2E8_04C60();
        D_0015EF48_04C60.stage = D_0015EF48_04C60.stage + 1;
        break;
    case 6:
        if (func_0012DDC0_04C60() != 0) {
            return 0;
        }
        shared_header = D_0015EF48_04C60.shared;
        D_0015EE5C_04C60 = 0xFFFFFFFFU;
        func_0012E1C8_04C60(shared_header->sound_bank_offset + (s32)shared_header,
                            (s32)func_0022F090_04C60, (u32)&D_0015EE5C_04C60);
        D_0015EF48_04C60.stage = D_0015EF48_04C60.stage + 1;
        break;
    case 7:
        if (func_0012DDC0_04C60() != 0) {
            return 0;
        }
        if ((u32)D_0015EE5C_04C60 == 0xFFFFFFFFU) {
            return 0;
        }
        func_0012E2E8_04C60();
        return 1;
    }
    return 0;
}

struct PartList {
    unsigned char pad00[6];
    unsigned char flag;          /* 0x06 */
    unsigned char pad07[5];
    unsigned char used;          /* 0x0C */
    unsigned char pad0D[0x3B];
    int entries[1];   /* 0x48 */
};

struct ColorSrc {
    unsigned char pad00[0x38];
    unsigned long color;        /* 0x38 */
};

struct RenderGlobals {
    unsigned char pad00[0x2080];
    struct ColorSrc *color_src;  /* 0x2080 */
};

struct GlobalIndex {
    unsigned char pad00[0x26];
    short slot;         /* 0x26 */
};

struct Moby {
    unsigned char pad00[0x24];
    struct PartList *parts;  /* 0x24 */
    unsigned char pad28[0xA];
    unsigned short unk32;        /* 0x32 */
    unsigned short unk34;        /* 0x34 */
    unsigned char pad36[2];
    unsigned long color;        /* 0x38 */
    unsigned char pad40[8];
    int attach;       /* 0x48 */
    unsigned char pad4C[6];
    unsigned char idx;           /* 0x52 */
    unsigned char slot;          /* 0x53 */
    unsigned char pad54[0x1E];
    unsigned char flag72;        /* 0x72 */
    unsigned char flag73;        /* 0x73 */
    unsigned char pad74[4];
    int model;        /* 0x78 */
    unsigned char pad7C[0x18];
    int unk94;        /* 0x94 */
};

struct ModelRec {
    unsigned short flags;        /* 0x00 */
    unsigned char pad02[2];
    int part_count;   /* 0x04 */
    unsigned short pad08;
    unsigned char pad0A[2];
    unsigned short num_parts;    /* 0x0C */
    unsigned char pad0E[2];
    int end_off;      /* 0x10 */
    int part_off[1];  /* 0x14 */
};

struct TransferState {
    unsigned char pad00[0x38];
    int unk38;        /* 0x38 */
    unsigned char pad3C[4];
    unsigned short unk40;        /* 0x40 */
    unsigned char pad42[2];
    short num_parts;    /* 0x44 */
    unsigned short unk46;        /* 0x46 */
    unsigned char pad48[4];
    int unk4C;        /* 0x4C */
    int unk50;        /* 0x50 */
    int unk54;        /* 0x54 */
    struct ModelRec *rec;     /* 0x58 */
    int unk5C;        /* 0x5C */
    unsigned char pad60[0x118];
    struct Moby *slots[1];    /* 0x178 */
};

extern struct TransferState D_0018CC20_t __asm__("D_0018CC20") NOT_SDA;
extern struct GlobalIndex D_0013E130_g __asm__("D_0013E130");
extern struct RenderGlobals D_0013F450_r __asm__("D_0013F450");
extern int D_0015F6E8 MACRO_ADDR;
extern int D_00160588[];
extern int func_0020C468_2(int, int) __asm__("func_0020C468");
extern struct Moby *func_0020D348_m(int) __asm__("func_0020D348");

/* UpdateWorldObjectAnimation: relocate the model record loaded for the
   current chunk (D_0018CC20+0x58) and hook each of its parts to a moby,
   spawning (func_0020D348) and initialising the moby the first time; each
   part's pointer table is made absolute. Adapted from Lombyte (GPL-3.0) for
   PAL. */
void UpdateWorldObjectAnimation(void *arg0) {
    struct ModelRec *rec;
    struct Moby *mob;
    int *cp;
    int *sp;
    int i;
    int k;
    int idx;
    int id;
    int off;
    int endp;
    int pc;

    func_00118D80(0);
    func_0020C468_2(D_0018CC20_t.unk5C, (int)D_0018CC20_t.rec);
    func_00118D80(0);

    rec = D_0018CC20_t.rec;
    D_0018CC20_t.unk38 = 0;
    cp = rec->part_off;
    D_0018CC20_t.unk40 = rec->flags;
    pc = rec->part_count;
    D_0018CC20_t.unk46 = rec->pad08;
    D_0018CC20_t.num_parts = rec->num_parts;
    D_0018CC20_t.unk54 = (int)rec + rec->end_off;
    if (pc < 0x400) {
        D_0018CC20_t.unk4C = 0;
    } else {
        D_0018CC20_t.unk4C = (int)rec + pc;
    }

    for (i = 0; i < D_0018CC20_t.num_parts; i++) {
        off = *cp++;
        sp = (int *)((unsigned char *)rec + off);
        id = sp[0];
        sp = (int *)((unsigned char *)sp + 0xC);
        endp = (int)rec + sp[0];
        sp = (int *)((unsigned char *)sp + 4);
        if (D_0015F6E8 == 6 && i == 0 && id == 0x215) {
            id = D_00160588[D_0013E130_g.slot];
        }
        mob = D_0018CC20_t.slots[i];
        if (mob == 0) {
            mob = func_0020D348_m(id);
            idx = mob->parts->used;
            mob->parts->used = idx + 1;
            mob->idx = idx;
            mob->slot = idx;
            mob->unk32 = 0x1FF;
            mob->unk34 |= 6;
            mob->flag72 = 0xFF;
            mob->unk94 = 0;
            if (D_0013F450_r.color_src == 0) {
                mob->color = 0x38383800000000;
            } else {
                mob->color = D_0013F450_r.color_src->color;
            }
            if (mob->parts->flag) {
                mob->flag73 = 0x18;
            }
            D_0018CC20_t.slots[i] = mob;
        }
        mob->model = endp;
        mob->parts->entries[mob->idx] = sp;
        for (k = 0; k < ((unsigned char *)sp)[0x10]; k++) {
            int *w = (int *)((unsigned char *)sp + 0x1C);

            w[k] = (int)sp + w[k];
        }
    }
}

LINKER_REMNANT("asm/remnants/text", func_00205218);

extern void func_00204FC0(void *);
extern int D_0018CC20 NOT_SDA;
extern int D_001941C8 NOT_SDA;
extern int D_0016100C;

extern int D_0016100C_m __asm__("D_0016100C") MACRO_ADDR;

/* ParseSpaceSceneChunk(int): run func_00204FC0 on chunk index's slot,
   with the chunk's value as the current one meanwhile. The slot address is
   written base - (-(index * 4)), which keeps retail's base-first addu
   (adapted from Lombyte (GPL-3.0) for PAL). */
void ParseSpaceSceneChunk(int arg0) {
    char *base = (char *)&D_0018CC20;
    char *p = base - (-(arg0 * 4));
    *(int *)(base + 0x5C) = *(int *)(p + 0x60);
    UpdateWorldObjectAnimation(p);
    *(int *)(base + 0x5C) = D_001941C8 + D_0016100C_m;
}

typedef struct { char pad00[0x10]; int class_id; char pad14[0x38]; } GadgetRec_05270;
typedef struct { char pad00[0x1A]; short runtime_index; int resource_tag; } ClassRuntimeEntry_05270;
typedef struct { char *packet_data; int packed_packet_extent; char pad08[8]; } ClassRenderGroup_05270;
typedef struct {
    ClassRenderGroup_05270 *render_groups;
    char pad04[2];
    unsigned char third_render_group_count;
    unsigned char patch_group_index;
    char pad08[0x20];
    ClassRuntimeEntry_05270 *runtime_table;
    int original_field_2c;
} ClassResourceHeader_05270;

extern int D_00160044 MACRO_ADDR;
extern int D_00160048 MACRO_ADDR;
extern int D_0016004C MACRO_ADDR;
extern int D_00160050 MACRO_ADDR;
extern void func_0020C468();
extern int D_001CBE40[];
extern int D_001CBEA0[];
extern short D_001CC0E0[][16];
extern char D_001CBF60[][16];
extern unsigned char D_001CAE40[];
extern GadgetRec_05270 D_001864D0[];
extern unsigned char D_0013E620[];
extern long D_0019E7F0[];

void LoadMobyClassResources(int class_id, int buffer_index) {
    int runtime_index;
    int resource_table_index;
    ClassResourceHeader_05270 *resource_data;
    void **resource_table;
    unsigned char class_slot;
    int resource_tag;
    int vendor_item_index;

    if (D_0016004C >= 0 && D_001CBE40[D_0016004C] == class_id) {
        return;
    }
    for (D_0016004C = 0; D_0016004C < D_00160048; D_0016004C++) {
        if (D_001CBE40[D_0016004C] == class_id) {
            break;
        }
    }
    if (buffer_index == -1) {
        buffer_index = D_00160050 == 0;
    }
    D_00160050 = buffer_index;
    resource_data = (ClassResourceHeader_05270 *)((char *)D_001941C0[4] + buffer_index * 0x18000);
    func_00118D80(0);
    ((void (*)(int, void *))func_0020C468)(D_001CBEA0[D_0016004C], resource_data);
    func_00118D80(0);
    class_slot = D_001B3E40[class_id];
    resource_table = D_001B3580;
    resource_table[class_slot] = resource_data;
    D_001B6500[class_slot] = resource_data->original_field_2c;
    ((void (*)(void *, void *, void *, int))MobyClassRelocate)(resource_data, D_001CAE40, D_001CBF60[D_0016004C], class_id);
    resource_table_index = D_0016004C;
    resource_tag = D_00160044;
    for (runtime_index = 0; runtime_index < 16; runtime_index++) {
        short e = D_001CC0E0[resource_table_index][runtime_index];
        if (e >= 0) {
            ((ClassResourceHeader_05270 *)resource_table[class_slot])->runtime_table[runtime_index].runtime_index = e;
            ((ClassResourceHeader_05270 *)resource_table[class_slot])->runtime_table[runtime_index].resource_tag = resource_tag;
        }
    }
    for (vendor_item_index = 0; vendor_item_index < 0x25; vendor_item_index++) {
        if (((GadgetRec_05270 *)((char *)D_001864D0 + 0x10 - 0x10))[vendor_item_index].class_id == class_id) {
            ClassResourceHeader_05270 *rh;
            ClassRenderGroup_05270 *rg;
            char *pp;
            if (D_0013E620[vendor_item_index] == 0) return;
            rh = (ClassResourceHeader_05270 *)D_001B3580[class_slot];
            if (rh->third_render_group_count == 0) return;
            rg = &rh->render_groups[rh->patch_group_index];
            pp = rg->packet_data + (rg->packed_packet_extent - 4) * 0x10;
            *(long *)(pp + 0x20) = D_0019E7F0[0];
            *(long *)(pp + 0x30) = D_0019E7F0[2];
            return;
        }
    }
}
