#include "include_asm.h"

/* Family 435a49e62f53b8db - 368 bytes, 2 members.
   Placements: levels/3_endako @0x002FC368, levels/19_grelbin @0x002F8AF8.

   Measured binding.  D_L03_001B3188 is a four-byte pointer in the resident image
   (0x001B3188 = 1782152).  The retail body reads it with a per-site
   `lui $r,%hi` + `lw $r,%lo($r)` pair and writes it with a single
   `sw $r,%lo($gp)`; all three of the latter sit in a compiler delay slot
   (two in the `b` before the branch target, one in the `jr $ra` slot).

   That mix is exactly gas's expansion of the bare-symbol macro form
   `lw $r,D_L03_001B3188` / `sw $r,D_L03_001B3188`: absolute `lui`+`%lo` in ordinary
   flow, `$gp` (GPREL16) inside a `.set nomacro` region.  cc1 emits that
   macro only for an `sda` symbol, so the declaration carries `sda`.
   Without it cc1 computes the symbol's address once, CSEs it into a base
   register and reloads `0($base)` per site, which also costs an extra
   `move` to keep the incoming `$a1` alive across that register - 344
   bytes against the 368 of the reference. */

extern int *D_L03_001B3188 __attribute__((sda));

extern void D_00126288(void *, short, short, int, int, int, short, short);

extern void func_0011AEA0(int);

extern void func_001265B0(void *, int);

/* Paired-strip packet emitter, 396 bytes, placed in levels/10_hrugis_cloud and
   levels/5_feltzin_system.  The body writes a 16-byte GIF header into the
   resident packet cursor (a small-data global), advances the cursor, then
   writes the tag words and two packed 64-bit strip descriptors.  The retail
   loads the cursor with LUI/LO and advances it through $gp, so the unit is
   compiled under the qualified small-data profile (-O2 -G8). */
extern int *D_L05_001B2C88_5 __attribute__((sda));

extern int D_001A7350_1 __attribute__((sda));

extern int D_001A7354_1 __attribute__((sda));

/* Paired-strip packet emitter, 396 bytes, placed in levels/19_grelbin and
   levels/3_endako.  The body writes a 16-byte GIF header into the
   resident packet cursor (a small-data global), advances the cursor, then
   writes the tag words and two packed 64-bit strip descriptors.  The retail
   loads the cursor with LUI/LO and advances it through $gp, so the unit is
   compiled under the qualified small-data profile (-O2 -G8). */
extern int *D_L03_001B3188_1 __attribute__((sda));

extern int D_001A7350 __attribute__((sda));

extern int D_001A7354 __attribute__((sda));

/* Family 9b94f3cd1f5fac83 (372 B, 2 placements).
   Retail builds a 64-byte packet from the resident cursor global D_L02_001B3D08 and
   then advances that global itself, so the closing store addresses the global
   through $gp: the qualified small-data profile is required.
   The four header words are written through the global expression, so each one
   carries its own reload of D_L02_001B3D08 before its store. */

extern int *D_L02_001B3D08 __attribute__((sda));

/* shared native body of family 435a49e62f53b8db (default profile) */
void func_0028F948(int a0, int a1, int a2, int a3, int a4, int a5)
{
    char buf[96];
    void *work;
    int t4;
    int s2;

    s2 = 1 << (a3 + a4 - 4);
    t4 = (1 << a3) >> 6;
    if (t4 <= 0)
        t4 = 1;

    if (a5 == 0) {
        *(int *)D_L03_001B3188 = 0x10000006;
        *(int *)((char *)D_L03_001B3188 + 4) = 0;
        *(int *)((char *)D_L03_001B3188 + 8) = 0;
        *(int *)((char *)D_L03_001B3188 + 12) = 0x50000006;
        work = (char *)D_L03_001B3188 + 16;
        D_L03_001B3188 = (int *)((char *)D_L03_001B3188 + 112);
    } else {
        work = buf;
    }

    D_00126288(work, (short)a1, (short)t4, (short)a2, 0, 0,
            (short)(1 << a3), (short)(1 << a4));

    if (a5 == 0) {
        *(int *)D_L03_001B3188 = 0x30000000 | s2;
        *(int *)((char *)D_L03_001B3188 + 4) = a0;
        *(int *)((char *)D_L03_001B3188 + 8) = 0;
        *(int *)((char *)D_L03_001B3188 + 12) = 0x50000000 | s2;
        D_L03_001B3188 = (int *)((char *)D_L03_001B3188 + 16);
    } else {
        func_0011AEA0(0);
        func_001265B0(work, a0);
    }
}

/* shared native body of family a6a1a427e2388f5f */
void func_0028FAB8(int a0, int a1, int a2, int a3, int p4, int p5)
{
    long long *q;

    *(int *)((char *)D_L05_001B2C88_5 + 0) = 0x10000003;
    *(int *)((char *)D_L05_001B2C88_5 + 4) = 0;
    *(int *)((char *)D_L05_001B2C88_5 + 8) = 0;
    *(int *)((char *)D_L05_001B2C88_5 + 12) = 0x50000003;
    q = (long long *)D_L05_001B2C88_5;
    D_L05_001B2C88_5 = (int *)((char *)q + 16);
    q[2] = 0x4400000000008001LL;
    q[3] = 17424;
    q[4] = 65;
    q[5] = p4;
#define LO_D0 (*(int *)((char *)&D_001A7350_1 + 0))
#define HI_D4 (*(int *)((char *)&D_001A7354_1 + 0))

    if (p5 != 0) {
        q[6] = (a0 + LO_D0 - 8)
             | ((long long)(a1 + HI_D4 - 8) << 16)
             | 0xFFFFF000000000LL;
        q[7] = (a2 + LO_D0 - 8)
             | ((long long)(a3 + HI_D4 - 8) << 16)
             | 0xFFFFF000000000LL;
    } else {
        q[6] = ((a0 << 4) + LO_D0 - 16)
             | ((long long)((a1 << 4) + HI_D4 - 16) << 16)
             | 0xFFFFF000000000LL;
        q[7] = ((a2 << 4) + LO_D0 - 16)
             | ((long long)((a3 << 4) + HI_D4 - 16) << 16)
             | 0xFFFFF000000000LL;
    }
    D_L05_001B2C88_5 = (int *)((char *)D_L05_001B2C88_5 + 48);
}

/* shared native body of family 5e27257c03e3aebd */
void func_0028FC48(int a0, int a1, int a2, int a3, int p4, int p5)
{
    long long *q;

    *(int *)((char *)D_L03_001B3188_1 + 0) = 0x10000003;
    *(int *)((char *)D_L03_001B3188_1 + 4) = 0;
    *(int *)((char *)D_L03_001B3188_1 + 8) = 0;
    *(int *)((char *)D_L03_001B3188_1 + 12) = 0x50000003;
    q = (long long *)D_L03_001B3188_1;
    D_L03_001B3188_1 = (int *)((char *)q + 16);
    q[2] = 0x4400000000008001LL;
    q[3] = 17424;
    q[4] = 70;
    q[5] = p4;
#define LO_D0 (*(int *)((char *)&D_001A7350 + 0))
#define HI_D4 (*(int *)((char *)&D_001A7354 + 0))

    if (p5 != 0) {
        q[6] = (a0 + LO_D0 - 8)
             | ((long long)(a1 + HI_D4 - 8) << 16)
             | 0xFFFFF000000000LL;
        q[7] = (a2 + LO_D0 - 8)
             | ((long long)(a3 + HI_D4 - 8) << 16)
             | 0xFFFFF000000000LL;
    } else {
        q[6] = ((a0 << 4) + LO_D0 - 16)
             | ((long long)((a1 << 4) + HI_D4 - 16) << 16)
             | 0xFFFFF000000000LL;
        q[7] = ((a2 << 4) + LO_D0 - 16)
             | ((long long)((a3 << 4) + HI_D4 - 16) << 16)
             | 0xFFFFF000000000LL;
    }
    D_L03_001B3188_1 = (int *)((char *)D_L03_001B3188_1 + 48);
}

/* shared native body of family 9b94f3cd1f5fac83 */
void func_0028FDD8(int a0, int a1, int a2, int a3, long long a4, int a5, int a6)
{
    int *p;

    *(int *)D_L02_001B3D08 = 0x10000003;
    *(int *)((char *)D_L02_001B3D08 + 4) = 0;
    *(int *)((char *)D_L02_001B3D08 + 8) = 0;
    *(int *)((char *)D_L02_001B3D08 + 12) = 0x50000003;
    p = D_L02_001B3D08;
    D_L02_001B3D08 = p + 4;
    *(long long *)(p + 4) = 0x4400000000008001LL;
    *(long long *)(p + 6) = 17424;
    *(long long *)(p + 8) = 70;
    *(long long *)(p + 10) = a4;
    if (a6) {
        *(long long *)(p + 12) = (a0 + D_001A7350 - 8)
                               | ((long long)(a1 + D_001A7354 - 8) << 16)
                               | ((long long)a5 << 32);
        *(long long *)(p + 14) = (a2 + D_001A7350 - 8)
                               | ((long long)(a3 + D_001A7354 - 8) << 16)
                               | ((long long)a5 << 32);
    } else {
        *(long long *)(p + 12) = ((a0 << 4) + D_001A7350 - 16)
                               | ((long long)((a1 << 4) + D_001A7354 - 16) << 16)
                               | ((long long)a5 << 32);
        *(long long *)(p + 14) = ((a2 << 4) + D_001A7350 - 16)
                               | ((long long)((a3 << 4) + D_001A7354 - 16) << 16)
                               | ((long long)a5 << 32);
    }
    D_L02_001B3D08 = (int *)((char *)D_L02_001B3D08 + 48);
}

INCLUDE_ASM("asm/game", func_0028FF50);

INCLUDE_ASM("asm/game", func_00290008);

INCLUDE_ASM("asm/game", func_00290010);

INCLUDE_ASM("asm/game", func_002900B8);



INCLUDE_ASM("asm/game", func_00290668);
