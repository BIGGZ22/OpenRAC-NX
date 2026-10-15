#include "common.h"

/* This file holds the bodies of L09_002F8BA0.c that address a global through
   $gp: the level's small-data area. It is compiled with -G8 (tools/build.py
   picks that up from the -g8.c suffix), which is what turns the scalar below
   into a $gp-relative load. Everything absolute stays absolute: the offsets
   from $gp are far outside the 16-bit displacement, so the compiler keeps
   emitting lui/addiu for them. */

/* The level's per-frame step coefficient, reached as 0x1EF8($gp). */
extern f32 D_L09_001B0EE8 __attribute__((sda));

extern void func_00282CF0(f32 *, const f32 *, const f32 *);
extern void func_00282D30(char *, char *, f32);
extern void func_00282CC0(char *, char *, char *);

/* Rescales the object's vector: the sub-object's pair of points is differenced
   into a 16-byte frame local, that difference is scaled by the level's step
   coefficient, and the scaled vector is written back through both points. The
   sub-object itself is left alone. */
void func_L09_003F6AB8(unsigned char *a0) {
    unsigned char *s0 = a0;
    unsigned char *p = *(unsigned char **)(s0 + 0x68);
    unsigned char *s1 = p + 0x10;
    char buf[16];
    func_00282CF0((f32 *)buf, (const f32 *)p, (const f32 *)s1);
    func_00282D30(buf, buf, D_L09_001B0EE8);
    func_00282CC0((char *)s1, (char *)s1, buf);
    func_00282CC0((char *)(s0 + 0x10), (char *)(s0 + 0x10), (char *)s1);
}

/* func_L09_003FE2D8 and func_L09_003F69A0 belong in this file too -- they read
   a scalar through $gp -- but neither is written yet. func_L09_003FE2D8 reads
   D_L09_001B11D8 (0x21E8($gp)); func_L09_003F69A0 reads D_L09_001B0F04
   (0x1F14($gp)). Their closest attempts are in
   D:/RAC2/work/wave8-20261013/w12/drafts/ . */
