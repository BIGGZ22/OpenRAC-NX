/* Carried from the PAL decompilation's src/overlays/shared/vendor_002B90A8.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
struct DmaTag_c2b8f98 {
    u32 tag;
    u32 addr;
    u32 vif0;
    u32 vif1;
};
union PacketCursor_c2b8f98 {
    struct DmaTag_c2b8f98 *tag;
    struct GifTag *gif;
    s32 *words;
    u8 *bytes;
    s32 addr;
};
extern union PacketCursor_c2b8f98 D_L01_001611C0 MACRO_ADDR;
extern void func_001F9FF8(float *, float);
extern void func_001F9A68(void *, void *, float);
extern void func_00233830(void *, int);
extern void func_001FA378(void *, void *, void *);
extern void func_001FB680(void);
extern char D_L01_00167240[];
extern int D_L01_0015F5E0 MACRO_ADDR;
extern unsigned short D_0010E800 NOT_SDA;
extern char D_0010E810[];
extern char D_L01_0016CEC0[];
extern char D_L01_001CAC80[];
extern short D_L01_00161234;
extern short D_L01_00161228;
extern short D_L01_0016122C;
extern short D_L01_00161230;



void FUN_L01_002b8f98(void)
{
    float m[4][4];
    struct DmaTag_c2b8f98 *base;
    u8 *p;
    char *camera;

    func_001F9FF8(m, 1024.0f);
    func_001F9A68(m[3], D_L01_00167240, -1024.0f);
    m[3][3] = 1.0f;
    if (D_L01_0015F5E0 != 7) {
        func_00233830(D_0010E810, D_0010E800);
        D_L01_0015F5E0 = 7;
    }
    D_L01_001611C0.tag->tag = 0x10000000;
    D_L01_001611C0.tag->addr = 0;
    D_L01_001611C0.tag->vif0 = 0x11000000;
    D_L01_001611C0.tag->vif1 = 0x1000404;
    base = D_L01_001611C0.tag;
    base[1].tag = 0;
    base[1].addr = 0;
    base[1].vif0 = 0;
    base[1].vif1 = 0x6C0C43A4;
    func_001FA378(base + 2, D_L01_00167240 - 0x100, m);
    func_001FA378(base + 6, D_L01_00167240 - 0x80, m);
    base[10].tag = 0x8000;
    base[10].addr = 0x303EC000;
    base[10].vif0 = 0x412;
    camera = D_L01_0016CEC0;
    *(float *)&base[10].vif1 = *(float *)(camera + 0x210);
    p = (u8 *)(base + 11);
    qcopy(p, camera + 0x190);
    p = (u8 *)(base + 12);
    qcopy(p, camera + 0x1A0);
    *(float *)&base[13].tag = *(float *)(camera + 0x22C);
    *(float *)&base[13].addr = *(float *)(camera + 0x228);
    base[13].vif0 = 0;
    base[13].vif1 = 0;
    base[14].addr = 0x20001D2;
    base[14].vif0 = 0x15000000;
    base[14].tag = 0x3000000;
    base[14].vif1 = 0;
    p = (u8 *)(base + 15);
    D_L01_001611C0.tag->tag |= ((p - (u8 *)D_L01_001611C0.tag) >> 4) - 1;
    D_L01_001611C0.bytes = p;
    func_001FB680();
    D_L01_001611C0.tag->tag = 0x30000007;
    D_L01_001611C0.tag->addr = (u32)D_L01_001CAC80;
    D_L01_001611C0.tag->vif0 = 0;
    D_L01_001611C0.tag->vif1 = 0x50000007;
    *(unsigned *)&D_L01_00161228 = 0xFFFFFFFFu;
    *(unsigned *)&D_L01_0016122C = 0xFFFFFFFFu;
    *(unsigned *)&D_L01_00161230 = 0xFFFFFFFFu;
    *(unsigned *)&D_L01_00161234 = 0xFFFFFFFFu;
    D_L01_001611C0.tag++;
}
