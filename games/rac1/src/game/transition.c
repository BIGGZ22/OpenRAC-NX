#include "common.h"
#include "callees.h"
#include "moby.h"
#include "structs.h"

/*
 * transition.cpp in the original source; text 0x1E9E70-0x1EC038.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/*
 * Close but not yet byte-matching, new evidence for the sq/lq open
 * question below: this function only saves $ra (no $s0-$s7 at all), and
 * retail STILL spills it as `sq` here -- unlike every other function
 * seen so far, where retail consistently uses `sd` for a lone $ra save.
 * This compiler always uses `sd` for $ra regardless. Logic/instructions
 * otherwise identical (return func_0022C7E0(); ... 5 calls in a row,
 * body confirmed correct via objdump before reverting this to
 * INCLUDE_ASM):
 *   func_0022C7E0(); func_0022C188(); func_0022C870();
 *   func_00234C98(0x47, 0x5360B);
 *   func_00234C98(0x4E, 0x1000000 | (D_0015EF88 >> 13));
 * Means the sq/lq choice isn't purely "s-regs vs ra", it's something
 * more granular retail decides per-function (maybe per translation
 * unit, or some other property not yet isolated). See "Open toolchain
 * questions" in docs/DECOMP_PROGRESS.md.
 */
extern void func_0022C7E0(void);
extern void func_0022C188(void);
extern void func_0022C870(void);
extern void func_00234C98(int, long);
extern int D_0015EF88 MACRO_ADDR;

/* func_00234C98 is (int, long) and D_0015EF88 MACRO_ADDR; the older
   sq/sd note is obsolete. */
void func_001E9E70(void) {
    SetupSkyGifPaging();
    SkyLevelGeneric___maybe();
    DoSkyGifPaging();
    VU1_addGSregister(0x47, 0x5360B);
    VU1_addGSregister(0x4E, 0x1000000 | (D_0015EF88 >> 13));
}

INCLUDE_ASM("asm/nonmatchings/text", func_001E9EC8);

struct DiscFile_EABE8 {
    s32 sector;
    s32 size;
};

struct DiscTable_EABE8 {
    u8 pad_0[0x14E8];
    struct DiscFile_EABE8 unk14E8;         /* 0x14E8: the transition wad */
    u8 pad_14F0[0x38];
    struct DiscFile_EABE8 help_text;       /* 0x1528 */
};

extern struct DiscTable_EABE8 D_00137C80_EABE8 __asm__("D_00137C80");

typedef struct 
{
  s32 x0;
  s32 data;
  s32 x8;
  s32 xC;
  s32 x10;
  s32 x14;
  s32 n18;
  s32 x1C;
  s32 n20;
  s32 x24;
  s32 n28;
  s32 x2C;
  s32 n30;
  s32 x34;
  s32 n38;
  s32 x3C;
  s32 n40;
  s32 x44;
  s32 n48;
  s32 x4C;
  s32 x50;
  s32 x54;
  s32 x58;
  s32 x5C;
  s32 x60;
  s32 x64;
  s32 x68;
  s32 x6C;
  s32 x70;
  s32 x74;
  s32 x78;
  s32 x7C;
  s32 x80;
  s32 x84;
} WadHeader_EABE8;
typedef struct 
{
  s32 offset;
  u16 x4;
  u16 pad6;
  s32 pad8[2];
} WadTex_EABE8;
typedef struct 
{
  s32 offset;
  s32 x4;
  s32 pad8[2];
  u8 x10[0x10];
} WadClass20_EABE8;
typedef struct 
{
  s32 offset;
  s32 x4;
  s32 pad8[2];
  u8 x10[0x10];
  u8 x20[0x10];
} WadClass30_EABE8;
typedef struct 
{
  s32 offset;
  s32 size;
} WadSound_EABE8;
typedef struct 
{
  u8 pad0[0x14];
  u8 *hdr;
  s32 x18;
  s32 x1C;
} LoadState_EABE8;
typedef struct 
{
  u8 pad0[0x58];
  s32 x58;
  s32 x5C;
  s32 x60[0x46];
} SoundBanks_EABE8;
typedef struct 
{
  s32 offset;
  s32 pad[3];
} TextEntry_EABE8;
typedef struct 
{
  u8 pad0[0x2C];
  s32 count;
} HelpState_EABE8;
extern s32 D_0015EF74_EABE8 __asm__("D_0015EF74") MACRO_ADDR;
extern s32 D_0015EF78_EABE8 __asm__("D_0015EF78") MACRO_ADDR;
extern s32 D_0015EF8C_EABE8 __asm__("D_0015EF8C") MACRO_ADDR;
extern s64 D_0015F048_EABE8 __asm__("D_0015F048") MACRO_ADDR;
extern s32 D_0015F058_EABE8 __asm__("D_0015F058") MACRO_ADDR;
extern s32 D_0015F060_EABE8 __asm__("D_0015F060") MACRO_ADDR;
extern s32 D_0015F064_EABE8 __asm__("D_0015F064") MACRO_ADDR;
extern s32 D_0015F560_EABE8 __asm__("D_0015F560") MACRO_ADDR;
extern TextEntry_EABE8 *D_0015F780_EABE8 __asm__("D_0015F780") MACRO_ADDR;
extern s32 D_00160000_EABE8 __asm__("D_00160000") MACRO_ADDR;
extern s32 D_00160008_EABE8 __asm__("D_00160008") MACRO_ADDR;
extern s32 D_001604CC_EABE8 __asm__("D_001604CC") MACRO_ADDR;
extern s32 D_001604EC_EABE8 __asm__("D_001604EC") MACRO_ADDR;
extern s32 D_00160F94_EABE8 __asm__("D_00160F94") MACRO_ADDR;
extern s32 D_0016100C_EABE8 __asm__("D_0016100C") MACRO_ADDR;
extern s32 D_0016104C_EABE8 __asm__("D_0016104C") MACRO_ADDR;
extern s32 D_00161064_EABE8 __asm__("D_00161064") MACRO_ADDR;
extern u8 D_001862E0_EABE8[] __asm__("D_001862E0");
extern u8 D_00186410_EABE8[] __asm__("D_00186410");
extern SoundBanks_EABE8 D_0018CC20_EABE8 __asm__("D_0018CC20");
extern LoadState_EABE8 D_001941C0_EABE8 __asm__("D_001941C0");
extern u8 D_00194280_EABE8[] __asm__("D_00194280");
extern HelpState_EABE8 D_001997D0_EABE8 __asm__("D_001997D0");
extern u64 D_0019E7C0_EABE8[] __asm__("D_0019E7C0");
extern MobyClass *D_001B3580_EABE8[] __asm__("D_001B3580");
extern u8 D_001B3E40_EABE8[] __asm__("D_001B3E40");
extern s32 D_001B5D00_EABE8[] __asm__("D_001B5D00");
extern s32 D_001B6500_EABE8[] __asm__("D_001B6500");
extern u8 D_001B6C00_EABE8[] __asm__("D_001B6C00");
extern s32 D_001D8840_EABE8[] __asm__("D_001D8840");
extern s32 D_001E0C00_EABE8[] __asm__("D_001E0C00");
extern s32 D_001E2900_EABE8[] __asm__("D_001E2900");
extern void func_002348B8_EABE8(void) __asm__("func_002348B8");
extern void func_001F99B0_EABE8(void *, s32, s32) __asm__("func_001F99B0");
extern void func_00118D80_EABE8(s32) __asm__("func_00118D80");
extern void func_001EB300_EABE8(s32) __asm__("func_001EB300");
extern void func_00120858_EABE8(s32, s32) __asm__("func_00120858");
extern s32 func_001E9EC8_EABE8(u8 *) __asm__("func_001E9EC8");
extern void func_001F3008_EABE8(void) __asm__("func_001F3008");
extern void func_001F3140_EABE8(void) __asm__("func_001F3140");
extern char func_001F9968_EABE8(s32) __asm__("func_001F9968");
extern void func_00201E10_EABE8(void) __asm__("func_00201E10");
extern void func_00202F00_EABE8(u8 *, u8 *, u8 *, s32) __asm__("func_00202F00");
extern void func_00203038_EABE8(u8 *, s32) __asm__("func_00203038");
extern void func_00203118_EABE8(u8 *) __asm__("func_00203118");
extern void func_00203958_EABE8(u8 *, s32, u8 *) __asm__("func_00203958");
extern void func_00203E78_EABE8(u8 *, u8 *, u8 *, s32) __asm__("func_00203E78");
extern void func_00203F68_EABE8(u8 *, u8 *, u8 *, s32) __asm__("func_00203F68");
extern void func_00204340_EABE8(u8 *, u8 *, u8 *, u8 *, s32) __asm__("func_00204340");
extern void func_00204918_EABE8(u8 *, u8 *) __asm__("func_00204918");
extern void func_00205220_EABE8(s32) __asm__("func_00205220");
extern s32 func_0020C468_EABE8(u8 *, u8 *) __asm__("func_0020C468");
extern void func_00217628_EABE8(u8 *, s32, s32) __asm__("func_00217628");
extern unsigned int func_002176C8_EABE8(s32, s32, s32) __asm__("func_002176C8");
extern void func_00217748_EABE8(s32) __asm__("func_00217748");
extern void func_002348E8_EABE8(void) __asm__("func_002348E8");
extern s32 func_00122630_EABE8(void *, s16, s16, s16, s16, s16, s16, s16) __asm__("func_00122630");
extern s32 func_00122958_EABE8(void *, u8 *) __asm__("func_00122958");

/* Loads the space-transition wad to 0x1400000 and unpacks it at the level arena: textures and
   their tables, the three kinds of moby classes, sound and help text tables, the loading image;
   then resets the sequence state and relocates the eight help text tables.
   Adapted from Lombyte (MIT) for PAL: src/gameplay/state/transition_load_wad.c, transition_load_wad. */
void func_001EABE8(void)
{
  u8 li[0x60];
  WadHeader_EABE8 *hdr;
  u8 *data;
  u8 *base;
  s32 size;
  s32 i;
  s32 j;
  s32 k;
  s32 cnt;
  s32 v;
  s32 *out;
  int new_var;
  WadTex_EABE8 *tex;
  s32 new_var4;
  WadClass20_EABE8 *c20;
  u8 *new_var3;
  WadClass30_EABE8 *c30;
  WadSound_EABE8 *snd;
  TextEntry_EABE8 *te;
  WadTex_EABE8 *new_var2;
  s32 k_800 = 0x800;
  s32 adj;
  s64 t;
  s64 u;
  D_0015F058_EABE8 = 0;
  i = 0;
  func_002348B8_EABE8();
  func_00201E10_EABE8();
  D_0016100C_EABE8 = 0x100000;
  D_0015EF8C_EABE8 = 0x2C0000;
  D_0015EF78_EABE8 = 0x2C0000;
  D_0015EF74_EABE8 = 0x2C0000;
  func_001F99B0_EABE8(D_00194280_EABE8, 0x87654321, 0x10);
  func_001F99B0_EABE8(D_001B3E40_EABE8, -1, k_800);
  func_001F99B0_EABE8(D_001B6C00_EABE8, -1, 0xE00);
  func_001F99B0_EABE8(D_001B6500_EABE8, 0, 0xE0);
  func_001F3008_EABE8();
  func_001F3140_EABE8();
  func_002348E8_EABE8();
  func_00217628_EABE8((u8 *)0x1400000, D_00137C80_EABE8.unk14E8.sector, D_00137C80_EABE8.unk14E8.size);
  func_00217748_EABE8(1);
  func_00118D80_EABE8(0);
  size = func_0020C468_EABE8((u8 *)0x1400000, D_001941C0_EABE8.hdr);
  func_00118D80_EABE8(0);
  hdr = (WadHeader_EABE8 *) D_001941C0_EABE8.hdr;
  func_00203958_EABE8(((u8 *) hdr) + hdr->x0, hdr->x8, ((u8 *) hdr) + hdr->xC);
  t = ((s64) ((D_0015EF8C_EABE8 + hdr->x70) >> 8)) | 0x1D308000;
  u = (((s64) ((D_0015EF8C_EABE8 + hdr->x74) >> 8)) << 37) | (((s64) 0xB800) << 19);
  data = ((u8 *) hdr) + hdr->data;
  base = data + hdr->x60;
  D_0019E7C0_EABE8[0] = (t | u) | (((s64) (-1)) << 63);
  D_0019E7C0_EABE8[1] = 0xFFA0000000E0;
  D_0019E7C0_EABE8[2] = 0x0040000400004000;
  tex = (WadTex_EABE8 *) (((u8 *) hdr) + hdr->x34);
  if ((D_00160F94_EABE8 = hdr->n30) > 0)
  {
    do
    {
      D_001E0C00_EABE8[i] = (((s32) base) + tex[i].offset) + (func_001F9968_EABE8(tex[i].x4) << 28);
      i++;
    }
    while (i < D_00160F94_EABE8);
  }
  k = 0;
  tex = (new_var2 = (WadTex_EABE8 *) (((u8 *) hdr) + hdr->x3C));
  if ((D_00160008_EABE8 = hdr->n38) > 0)
  {
    do
    {
      D_001B5D00_EABE8[k] = (((s32) base) + tex[k].offset) + (func_001F9968_EABE8(tex[k].x4) << 28);
      k++;
    }
    while (k < D_00160008_EABE8);
  }
  k = 0;
  tex = (new_var2 = (WadTex_EABE8 *) (((u8 *) hdr) + hdr->x44));
  if ((D_00161064_EABE8 = hdr->n40) > 0)
  {
    do
    {
      D_001E2900_EABE8[k] = (((s32) base) + tex[k].offset) + (func_001F9968_EABE8(tex[k].x4) << 28);
      k++;
    }
    while (k < D_00161064_EABE8);
  }
  k = 0;
  tex = (new_var2 = (WadTex_EABE8 *) (((u8 *) hdr) + hdr->x4C));
  if ((D_001604EC_EABE8 = hdr->n48) > 0)
  {
    do
    {
      D_001D8840_EABE8[k] = (((s32) base) + tex[k].offset) + (func_001F9968_EABE8(tex[k].x4) << 28);
      k++;
    }
    while (k < D_001604EC_EABE8);
  }
  func_00204918_EABE8(data + hdr->x10, ((u8 *) hdr) + hdr->x34);
  func_00203118_EABE8(data + hdr->x14);
  c20 = (WadClass20_EABE8 *) (((u8 *) hdr) + hdr->x1C);
  D_00160000_EABE8 = 0;
  D_0016104C_EABE8 = 0;
  D_001604CC_EABE8 = 0;
  for (k = 0; k < hdr->n18; k++)
  {
    func_00203E78_EABE8((c20->offset != 0) ? (data + c20->offset) : (0), ((u8 *) hdr) + hdr->x3C, c20->x10, c20->x4);
    c20++;
  }

  c20 = (WadClass20_EABE8 *) (((u8 *) hdr) + hdr->x24);
  for (k = 0; k < hdr->n20; k++)
  {
    func_00203F68_EABE8(data + c20->offset, ((u8 *) hdr) + hdr->x44, c20->x10, c20->x4);
    c20++;
  }

  c30 = (WadClass30_EABE8 *) (((u8 *) hdr) + hdr->x2C);
  for (k = 0; k < hdr->n28; k++)
  {
    func_00204340_EABE8(data + c30->offset, ((u8 *) hdr) + hdr->x4C, c30->x10, c30->x20, c30->x4);
    c30++;
  }

  D_0015F560_EABE8 = (s32) (data + hdr->x68);
  func_00203038_EABE8(((u8 *) hdr) + hdr->x5C, hdr->x58);
  new_var3 = data + hdr->x64;
  new_var4 = hdr->x54;
  func_00202F00_EABE8(((u8 *) hdr) + (new_var = hdr->x6C), new_var3, ((u8 *) hdr) + new_var4, hdr->x50);
  func_00122630_EABE8(li, (D_0015EF74_EABE8 << 8) >> 16, 4, 0, 0, 0, 0x100, 0x80);
  func_00118D80_EABE8(0);
  func_00122958_EABE8(li, data + hdr->x84);
  func_00120858_EABE8(0, 0);
  D_001941C0_EABE8.x18 = ((s32) D_001941C0_EABE8.hdr) + size;
  v = D_0015EF74_EABE8;
  D_0015F048_EABE8 = ((v >> 8) | 0x20010000) | (((s64) 0xB800) << 19);
  D_0015EF74_EABE8 = v + 0x20000;
  D_0015EF78_EABE8 = v + 0x20000;
  D_001941C0_EABE8.x1C = func_001E9EC8_EABE8(data + hdr->x7C);
  func_001F99B0_EABE8(&D_0018CC20_EABE8, 0, 0x1C0);
  func_001F99B0_EABE8(D_00186410_EABE8, 0, 0x40);
  D_0018CC20_EABE8.x58 = D_001941C0_EABE8.x1C;
  D_0018CC20_EABE8.x5C = (D_001941C0_EABE8.x1C += 0x40000);
  D_001941C0_EABE8.x1C += 0x40000;
  snd = (WadSound_EABE8 *) (data + hdr->x80);
  cnt = 0;
  if (snd->size != 0)
  {
    out = D_0018CC20_EABE8.x60;
    do
    {
      cnt++;
      *out = ((s32) (data + hdr->x80)) + (snd->offset + k_800);
      snd++;
      out++;
    }
    while ((cnt < 0x46) && (snd->size != 0));
  }
  func_00205220_EABE8(0);
  D_0015F060_EABE8 = D_001941C0_EABE8.x1C;
  func_002176C8_EABE8(D_001941C0_EABE8.x1C, D_00137C80_EABE8.help_text.sector, D_00137C80_EABE8.help_text.size);
  D_0015F064_EABE8 = D_0015F060_EABE8;
  D_001941C0_EABE8.x1C = D_0015F060_EABE8 + (D_00137C80_EABE8.help_text.size << 11);
  for (k = 0; k < 8; k++)
  {
    func_001EB300_EABE8(k);
    j = 0;
    if (D_001997D0_EABE8.count > 0)
    {
      te = D_0015F780_EABE8;
      adj = ((s32) te) - 8;
      {
        do
        {
          te[j].offset += adj;
          j++;
        }
        while (j < D_001997D0_EABE8.count);
      }
    }
  }

  func_001EB300_EABE8(0);
  cnt = (new_var = D_001B3E40_EABE8[0x472]);
  if (cnt >= 0)
  {
    D_001B3580_EABE8[cnt]->unk28 = (s32)D_001862E0_EABE8;
    (*(u8 *)&D_001B3580_EABE8[cnt]->pad_D) = 5;
  }
}

extern int D_0015F064 MACRO_ADDR;
extern int D_0015F060 MACRO_ADDR;
extern int D_001997FC;
/* gp-relative, no retail symbol: gp 0x166D00 - 0x7580 = 0x15F780
   (cursor into the table walked below). */
extern short D_0015F780;

/* Points the D_0015F780 cursor at entry arg0 of the table D_0015F064
   indexes into D_0015F060, past its 8-byte header, and keeps the
   header's first word in D_001997FC. Both table pointers are MACRO_ADDR
   (one register each), and the index goes first in the addition. */
void func_001EB300(int arg0) {
    int *entry = (int *)(arg0 * 4 + D_0015F064);
    int off = *entry;
    char *p = (char *)(D_0015F060 + off);

    D_001997FC = *(int *)p;
    p += 8;
    *(char **)&D_0015F780 = p;
}

extern char D_0018CC20[];
extern float D_0018CEB0;
extern char D_00187180[];
extern void func_001F3140(void);

/* Transition_UpdateMovieCamera: the current keyframe (0x20 bytes: position,
   a flag byte at +0xC, then X/Y/Z angles) sets the camera position and
   its orientation matrix (negated first two rows); returns the flag. */
unsigned char func_001EB338(void) {
    char *t = D_0018CC20;
    char *key = *(char **)(t + 0x54) + *(int *)(t + 0x38) * 32;
    unsigned char flag = key[0xC];
    float *ang = (float *)(key + 0x10);
    char *pos;
    char *cam;
    float m[16];

    D_0018CEB0 = 0.63f;
    UpdateViewContext();
    pos = D_00187180;
    qcopy(pos, key);
    func_00125358(m);
    func_001254A0(m, m, *(float *)(key + 0x10));
    func_00125548(m, m, ang[1]);
    func_001253F8(m, m, ang[2]);
    cam = pos - 0x140;
    *(float *)(cam + 0x350) = -m[8];
    *(float *)(cam + 0x360) = -m[0];
    *(float *)(cam + 0x370) = m[4];
    *(float *)(cam + 0x354) = -m[9];
    *(float *)(cam + 0x364) = -m[1];
    *(float *)(cam + 0x374) = m[5];
    *(float *)(cam + 0x358) = -m[10];
    *(float *)(cam + 0x368) = -m[2];
    *(float *)(cam + 0x378) = m[6];
    return flag;
}

typedef struct {
    f32 x, y, z, w;
} Vec4_EB458 __attribute__((aligned(16)));

typedef struct {
    u8 pad0[0x10];
    Vec4_EB458 position;
    u8 pad20[0x30];
    u8 current_frame;
    u8 next_frame;
    u8 pad52[2];
    f32 frame_fraction;
    u8 pad58[0x19];
    u8 cached_frame;
    u8 pad72[0xD];
    u8 update_enabled;
    u8 pad80[0x26];
    s16 class_id;
} RenderSequenceActor_EB458;

typedef struct {
    u8 pad0[0x78];
    Vec4_EB458 *animation_positions;
} RenderSequenceSidecar_EB458;

typedef struct {
    u8 pad0[0x34];
    s32 time;
    s32 frame;
    s32 sequence_frame;
    s16 end_time;
    u8 pad42[2];
    s16 count;
    u8 pad46[0x132];
    RenderSequenceActor_EB458 *actors[1];
} RenderSequenceState_EB458;

extern RenderSequenceState_EB458 D_0018CC20_EB458 __asm__("D_0018CC20");
extern f32 D_0015F53C_EB458 __asm__("D_0015F53C") MACRO_ADDR;
extern s32 D_0015F050_EB458 __asm__("D_0015F050") MACRO_ADDR;
extern s32 D_0015F054_EB458 __asm__("D_0015F054") MACRO_ADDR;
extern s32 D_0015F6E8_EB458 __asm__("D_0015F6E8") MACRO_ADDR;
extern s32 D_0013CBE4_EB458[] __asm__("D_0013CBE4");
extern void func_00219E60_EB458(void) __asm__("func_00219E60");
extern void func_001E9790_EB458(RenderSequenceActor_EB458 *) __asm__("func_001E9790");
extern void func_001E97A8_EB458(void) __asm__("func_001E97A8");
extern void func_001E97B0_EB458(void) __asm__("func_001E97B0");
extern void func_001EB338_EB458(void) __asm__("func_001EB338");
extern s32 func_001F98C0_EB458(s32) __asm__("func_001F98C0");
extern void func_001F9BD8_EB458(Vec4_EB458 *, Vec4_EB458 *, Vec4_EB458 *) __asm__("func_001F9BD8");
extern void func_001F9C30_EB458(Vec4_EB458 *, Vec4_EB458 *, f32) __asm__("func_001F9C30");
extern f32 func_001F9F90_EB458(f32) __asm__("func_001F9F90");
extern f32 func_001FA888_EB458(s32) __asm__("func_001FA888");
extern void func_001FD3E8_EB458(void) __asm__("func_001FD3E8");
extern void func_00205220_EB458(s32) __asm__("func_00205220");
extern void func_0020D6D0_EB458(RenderSequenceActor_EB458 *) __asm__("func_0020D6D0");
extern void func_0020ED48_EB458(RenderSequenceActor_EB458 *) __asm__("func_0020ED48");
extern void func_0021A1A0_EB458(void) __asm__("func_0021A1A0");
extern void func_0022DD68_EB458(void) __asm__("func_0022DD68");

/* One frame of the space-flight sequence between levels: steps the sequence clock and its
   scene chunks, the movie camera, each actor's animation frame and interpolated position,
   then the intro overlay fades and the mode's own update (pause menu, freeze) and sound.
   Adapted from Lombyte (MIT) for PAL: src/textbin/fun_001eb0a8.c, update_gameplay_frame. */
void func_001EB458(void) {
    Vec4_EB458 first_position;
    Vec4_EB458 next_position;
    RenderSequenceActor_EB458 *actor;
    Vec4_EB458 *animation_positions;
    s32 actor_index;
    f32 fade;

    func_001E97B0_EB458();
    fade = D_0015F53C_EB458 - 0.0625f;
    D_0018CC20_EB458.frame++;
    D_0018CC20_EB458.time++;
    D_0015F53C_EB458 = fade;
    if (fade < 0.0f) {
        D_0015F53C_EB458 = 0.0f;
    }
    if (D_0018CC20_EB458.time >= D_0018CC20_EB458.end_time) {
        D_0018CC20_EB458.time = 0;
        D_0018CC20_EB458.sequence_frame = 0;
        func_00205220_EB458(0);
    } else if (D_0018CC20_EB458.frame >= 0x60) {
        func_00205220_EB458(++D_0018CC20_EB458.sequence_frame);
    }
    func_001EB338_EB458();
    for (actor_index = 0; actor_index < D_0018CC20_EB458.count; actor_index++) {
        u8 frame_index;
        actor = D_0018CC20_EB458.actors[actor_index];
        frame_index = D_0018CC20_EB458.frame >> 1;
        actor->current_frame = D_0018CC20_EB458.frame >> 1;
        actor->next_frame = frame_index + 1;
        func_0020D6D0_EB458(actor);
        actor->frame_fraction = func_001FA888_EB458(D_0018CC20_EB458.frame & 1) * 0.5f;
        animation_positions = ((RenderSequenceSidecar_EB458 *)actor)->animation_positions;
        func_001F9C30_EB458(&first_position, &animation_positions[actor->current_frame],
                            1.0f - actor->frame_fraction);
        func_001F9C30_EB458(&next_position, &animation_positions[actor->next_frame],
                            actor->frame_fraction);
        func_001F9BD8_EB458(&actor->position, &first_position, &next_position);
        actor->cached_frame = 0xFF;
        func_0020ED48_EB458(actor);
        actor->update_enabled = 0;
        if (actor->class_id == 0) {
            func_001E9790_EB458(actor);
        }
    }
    func_001E97A8_EB458();
    if (D_0015F6E8_EB458 == 0) {
        D_0015F058_EABE8++;
        if (func_001F98C0_EB458(0x1E) < D_0015F058_EABE8) {
            if (++D_0015F050_EB458 > 0x40) {
                D_0015F050_EB458 = 0x40;
            }
        }
        if (func_001F98C0_EB458(0x78) < D_0015F058_EABE8) {
            D_0015F054_EB458 =
                (s32)(func_001F9F90_EB458((D_0015F058_EABE8 - func_001F98C0_EB458(0x78)) % 60 * 0.10471976f +
                                          -3.1415927f) *
                      32.0f) +
                0x60;
        }
        if (D_0013CBE4_EB458[0] & 0x840) {
            func_00219E60_EB458();
        }
        func_0022DD68_EB458();
    } else if (D_0015F6E8_EB458 == 3) {
        D_0015F058_EABE8 = func_001F98C0_EB458(0x3C);
        if ((D_0015F050_EB458 -= 0x10) < 0) {
            D_0015F050_EB458 = 0;
        }
        if ((D_0015F054_EB458 -= 0x10) < 0) {
            D_0015F054_EB458 = 0;
        }
        func_0021A1A0_EB458();
        func_0022DD68_EB458();
    } else if (D_0015F6E8_EB458 == 4) {
        func_001FD3E8_EB458();
        func_0022DD68_EB458();
    }
}

struct M2c_D_0016045C
{
  u8 pad_0[0x4];
  s16 unk4;
  u8 pad_6[0x2];
};
extern u8 D_00100AE0[];
extern s32 D_0013E604[];
extern short D_0015EE88;
extern short D_0015F048;
extern s32 D_0015F050 MACRO_ADDR;
extern s32 D_0015F054 MACRO_ADDR;
extern f32 D_0015F53C MACRO_ADDR;
extern s32 D_0015F564 MACRO_ADDR;
extern s32 D_0015F6E8 MACRO_ADDR;
extern s32 D_0015F704 MACRO_ADDR;
extern struct M2c_D_0016045C * D_0016055C MACRO_ADDR;
extern s32 D_0018A3E8[];
extern u8 D_001940C0[];
extern u8 D_001D9240[];
extern u8 D_001E1600[];
extern u8 D_001E3500[];
extern void func_00235290();
extern void func_001F99B0(void *, s32, s32);
extern s32 func_00118D80();
extern void func_001E9E70_EB7C0() __asm__("func_001E9E70");
extern s32 func_001EB338_EB7C0() __asm__("func_001EB338");
extern s32 func_001F2608();
extern s32 func_001F2930();
extern void func_001F3C10();
extern s32 func_001F4630();
extern s32 func_001F4748();
extern u64 func_001F4868();
extern void func_001F4A00();
extern s32 func_001F55C0();
extern void func_001F5800();
extern s32 func_001FA898(f32);
extern s32 func_001FB530();
extern s32 func_001FB848();
extern s32 func_001FBE80();
extern void func_0020DAB0();
extern s32 func_0020DD48();
extern void func_0020E2B0();
extern void func_00218B10();
extern s32 func_0021A610();
extern s32 func_00229D48();
extern void func_00229E50();
extern s32 func_0022B8F8();
extern s32 func_00234620();
extern void func_002346C0();
extern s32 func_002347F0();
extern s32 func_00234AC8();
extern s32 func_00234C98_EB7C0() __asm__("func_00234C98");
extern void func_00234F40();
extern s32 func_002362B0();
extern s32 func_00236A98();
extern void func_00236BE0();
extern s32 func_00238688();
void func_001EB7C0(s32 *arg0);

/* Adapted from Lombyte (MIT) for PAL by OpenRAC's tools/port.py: src/gameplay/state/transition_default_draw.c, transition_default_draw. */
void func_001EB7C0(s32 *arg0)
{
    s32 n;

    if (D_0016055C == 0 || D_0016055C->unk4 != 0) {
        framebuf_appendLargeSetup();
    }
    FastMemSet(D_001940C0, -1, 0x80);
    func_001EB338_EB7C0();
    func_001F2608();
    func_0020DAB0();
    ResetGsRegisters();
    D_0015F704 = -1;
    if (D_0016055C != 0) {
        func_001E9E70_EB7C0();
    }
    DrawTfrag();
    Vif1ChainCmd(0x02010000);
    DrawTies_1();
    Vif1ChainCmd(0x02020000);
    DrawShrubs();
    Vif1ChainCmd(0x02040000);
    if (D_0015F6E8 == 3) {
        func_0021A610();
    } else {
        DrawMobys();
    }
    Vif1ChainCmd(0x02080000);
    SetupGifPaging(0);
    func_00234F40();
    if (D_0015F564 != 0) {
        ExecuteDrawCallbacks();
    }
    func_00234F40();
    if (D_0018A3E8[0] != 0) {
        func_00234C98_EB7C0(8, 5);
        func_00234F40();
        func_00118D80(0);
        PartProc();
        D_0015F704 = 8;
    }
    AA_BlurPass();
    ResetGsRegisters();
    if (D_0015F050 != 0) {
        DrawTexturedQuad(0xEC, 0x10, 0x100, 0x80, 0, 0, 0x100, 0x80,
                      (long)(D_0015F050 << 24 | 0x808080), (*(s64 *)&D_0015F048));
    }
    if (D_0015F054 != 0) {
        n = (*(s32 *)&D_0015EE88) - 1;
        if (n < 0) {
            n = 0;
        }
        DrawTexturedQuad(0xA0, D_0013E604[0] - 0x50, 0xC0, 0x60, 0, 0, 0x100, 0x80,
                      (long)(D_0015F054 << 24 | 0x808080), GetEffectTex(n + 4));
    }
    DoGifPaging();
    if (D_0015F53C > 0.0f) {
        if (D_0015F53C > 1.0f) {
            D_0015F53C = 1.0f;
        }
        emit_rgba_draw_packet(0, 0, 0, truncate_float_to_s32(D_0015F53C * 128.0f));
    }
    VU0_loadMicroProgram(D_00100AE0);
    func_00118D80(0);
    if (D_0015F6E8 == 4) {
        func_001FBE80();
    }
    VU1_syncChain(2);
    LightTfrags(D_001E1600);
    PatchTfragGifs();
    VU1_syncChain(4);
    LightTies(D_001E3500);
    PatchTieGifs();
    VU1_syncChain(8);
    LightShrubs(D_001D9240);
    PatchShrubGifs();
    VU1_syncChain(0x10);
    PatchMobyGifs();
    UpdateFog();
}

extern char D_0013E650[];
extern int D_0015F694;

extern int D_0015F694_m __asm__("D_0015F694") MACRO_ADDR;

/* The older register residual was the split load of D_0015F694; read
   through a MACRO_ADDR alias it is retail's one-register load. */
int func_001EBAF0(int arg0, int arg1) {
    if (arg0 >= 0) {
        char *p = D_0013E650 + arg0 * 0x70;
        if (*(short *)(p + 0x7E) == arg1 + D_0015F694_m) {
            unsigned char s = p[0x74];
            if (s == 1 || s == 2) {
                return 1;
            }
        }
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/text", func_001EBB48);

/* Not a standalone function: single `addiu $sp,$sp,0x30`, no `jr $31` --
   fallthrough fragment, same category as func_00113AD8 in core_text. */
LINKER_REMNANT("asm/remnants/text", func_001EC030);
