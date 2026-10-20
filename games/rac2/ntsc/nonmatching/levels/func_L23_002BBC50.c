/* func_L23_002BBC50 (1296 bytes, level 23 only) -- NEAR MISS, not integrated.
   This C compiles to 1280 bytes, 16 short of retail's 1296. It belongs in the -G0 unit
   src/levels/23_damosel_orbit/L23_002BBC50.c (the function's own file) in place of the marker.
   check.py: "1280 bytes, expected 1296".
   Right: everything outside two quadword stores to the 16-byte vector at 0($sp). That covers the
   frame and save order, every call and constant, the switch tree and the bare $at stores and loads
   (`sda` in this -G0 unit). It also covers the shared `return 0` block right after the
   f2294 == 0x31 test. That needs 20/7 as one `if` and 0x31 as its own: jump2's cross-jumping
   kept the first `return 0` that follows a conditional jump in every variant measured (one
   five-way `||` put it after the 0xFF test). The flag test is one conditional expression; a
   separate variable takes $a0 where retail uses $v0.
   What differs, all of it the quadword-copy wall:
   - the copy at retail +0x19C. Retail has `beqz v0 {addiu v1,s1,0x10}; lq v0,0(v1);
     sq v0,0(sp); lwc1 f1,0x1C(s1); li.s; c.eq.s; bc1tl {s0 = 1}; b {a0 = P}; lw a2,0x20(s1)`.
     This cc1 folds the offset (`lq v0,0x10(s1)`) and schedules the lwc1 and the constant above
     the copy (alias.c: a stack store never aliases a global-based load). Reorg then puts the
     `sq` in the bc1f's slot instead of optimize_skip's annulled `s0 = 1`, and hoists the else
     arm's `lw a2` into a beql. The block comes out 3 words short;
   - the zero at retail +0x390: retail `sq $zero,0($sp); b {lui s0,0x19}`, this cc1
     `b {sq $zero,0($sp)}`. 1 word short.
   Retail's copy and zero behave like asm insns: an "r" operand (the addiu), never in a delay
   slot, a stop for reorg's backward search (stop_search_p) and a barrier for the scheduler.
   That is what the inline-asm quadword helpers give (docs/COMPILER-NOTES.md, "Quadword copy
   source shapes"), not plain C.
   Measured without effect:
   - reading the source pointer after an inner join: cse skips the `flag = 1` block, so the
     offset is still folded, and the copy moves below the compare;
   - the vector as a union: both are struct refs now, but base_alias_check still keeps the
     stack apart from the table symbol;
   - -fno-schedule-insns: the copy region takes retail's shape, but the rest of the body
     diverges.
*/

typedef unsigned char u8;
typedef short s16;
typedef int s32;
typedef int TI16 __attribute__((mode(TI)));

typedef struct {
    u8 pad0[0x80];
    TI16 f80;
    u8 pad90[0xF0 - 0x90];
    TI16 fF0;
    u8 pad100[0x10];
    TI16 f110;
    u8 pad120[0x1DC - 0x120];
    s16 f1DC;
    u8 pad1DE[0x21C - 0x1DE];
    s16 f21C;
    u8 pad21E[0x68E - 0x21E];
    s16 f68E;
    u8 pad690[0x6C4 - 0x690];
    void *f6C4;
    u8 pad6C8[0x1497 - 0x6C8];
    u8 f1497;
    u8 pad1498[0x1500 - 0x1498];
    s32 f1500;
    u8 pad1504[0x1874 - 0x1504];
    s32 f1874;
    u8 pad1878[0x2290 - 0x1878];
    u8 *f2290;
    s32 f2294;
    u8 pad2298[4];
    s32 f229C;
    u8 pad22A0[0x22B4 - 0x22A0];
    u8 f22B4;
    u8 pad22B5[0x249C - 0x22B5];
    u8 *f249C;
    u8 pad24A0[0x24CC - 0x24A0];
    s32 f24CC;
} Player_002BBC50;

typedef struct {
    u8 pad0[0x10];
    float f10;
    float f14;
    float f18;
    float f1C;
    u8 *f20;
    s32 f24;
    u8 f28;
    u8 pad29[3];
    float f2C;
    s32 f30;
    u8 pad34[4];
    u8 *f38;
    u8 pad3C[4];
} Entry_002BBC50;

extern Player_002BBC50 D_00189E20;
extern Entry_002BBC50 D_L23_001C4D00[];
extern s32 D_L23_001A8F00 __attribute__((sda));
extern u8 D_001A7A32 __attribute__((sda));
extern u8 D_001A7A33 __attribute__((sda));

extern float func_L00_0037E668(void);
extern float func_L00_0037E5F0(float);
extern s32 func_00283CF0(float);
extern void func_L00_002A6708(void);
extern void func_00282CF0(void *, void *, void *);
extern void func_L00_002D36E0(void *, s32, float, float, float);
extern void func_L22_002C9898(s32, s32);
extern u8 *func_002A9640(u8 *, s32, s32);
extern void func_L00_002A4EF8(s32);
extern void func_L00_002B9B88(s32, s32);
extern void func_L00_002B2C58(void *, s32, float, float);
extern void func_L00_002D3C68(void *, void *, float);
extern void func_L23_00423588(u8 *, s32);
extern void func_00282D30(void *, void *, float);
extern void func_00282C88(void *);
extern void func_00282CC0(void *, void *, void *);

s32 func_L23_002BBC50(s32 arg0) {
    TI16 vec;
    Entry_002BBC50 *e;
    u8 *moby;
    u8 *other;
    s32 flag;
    s32 bits;

    if (D_00189E20.f229C == 20 || D_00189E20.f229C == 7) {
        return 0;
    }
    if (D_00189E20.f2294 == 0x31) {
        return 0;
    }
    if (D_00189E20.f1DC != 0) {
        return 0;
    }
    if (D_L23_001A8F00 != 0) {
        return 0;
    }
    D_00189E20.f249C = 0;
    moby = D_00189E20.f2290;
    if (moby[0xA8] == 0xFF) {
        return 0;
    }
    e = &D_L23_001C4D00[moby[0xA8]];
    if (e->f38 != moby) {
        return 0;
    }
    if ((D_00189E20.f22B4 == 2 ? e->f24 & 2 : e->f24 & 9) == 0) {
        return 0;
    }
    if (e->f2C != 0.0f) {
        D_001A7A33 = 0;
        D_001A7A32 = 1;
    }
    if (func_L00_0037E668() != 0.0f && D_00189E20.f22B4 == 0) {
        e->f2C = func_L00_0037E5F0(e->f2C);
        if (e->f2C == 0.0f) {
            return 0;
        }
    }
    D_00189E20.f249C = e->f20;
    if (D_00189E20.f229C == 15) {
        D_00189E20.f6C4 = e;
        D_00189E20.f68E = 1;
        return 0;
    }
    if (D_00189E20.f22B4 == 2) {
        D_00189E20.f1874 -= func_00283CF0(e->f2C);
        if (D_00189E20.f1874 < 0) {
            D_00189E20.f1874 = 0;
        }
    }
    if (arg0 == 0) {
        return 1;
    }
    flag = 0;
    func_L00_002A6708();
    if (e->f30 & 1) {
        vec = *(TI16 *)&e->f10;
        if (e->f1C == 5627.9248046875f) {
            flag = 1;
        }
    } else if (e->f20 != 0) {
        func_00282CF0(&vec, &D_00189E20.f80, e->f20 + 0x10);
    } else {
        func_L00_002D36E0(&vec, 0, -1.0f, 0.0f, 0.0f);
    }
    switch (D_00189E20.f22B4) {
    case 0:
        other = D_00189E20.f249C;
        if (other != 0 && (*(s16 *)(other + 0xAA) == 0x4EB || *(s16 *)(other + 0xAA) == 0x558)) {
            func_L22_002C9898(0x7D, 1);
            return 1;
        }
        if ((D_00189E20.f2294 == 1 || D_00189E20.f2294 == 0x1D || D_00189E20.f1500 != 0)
            && !(e->f24 & 8)) {
            u8 *r = func_002A9640(D_00189E20.f2290, 1, 0);

            if (r != 0) {
                func_L00_002A4EF8(func_00283CF0(*(float *)(r + 0x2C)));
            }
            if (D_00189E20.f24CC == 0) {
                func_L22_002C9898(0x16, 1);
            }
            D_00189E20.f1DC = 0x4D;
            if (D_00189E20.f21C == 0) {
                D_00189E20.f21C = 0x12;
            }
            func_L00_002B9B88(0x14, 0);
            return 0;
        }
        if (D_00189E20.f229C == 0x12) {
            func_L22_002C9898(0x72, 1);
        } else if (D_00189E20.f229C == 0x11) {
            func_L22_002C9898(0x73, 1);
        } else if (e->f24 & 8) {
            func_L22_002C9898(0x8C, 1);
        } else {
            func_L22_002C9898(0x16, 1);
        }
        if (D_00189E20.f2294 == 0x8C) {
            vec = 0;
        } else {
            func_L00_002B2C58(&vec, flag, D_00189E20.f1497 ? 0.0f : 0.055000003f, 0.028333336f);
            if (flag != 0 && e->f28 == 4) {
                func_L00_002D3C68(&vec, &vec, 2.0f);
            }
        }
        break;
    case 1:
        func_L22_002C9898(0x42, 1);
        func_L00_002B2C58(&vec, flag, 0.083333336f, 0.050000004f);
        break;
    case 2:
        if ((e->f24 & 4) || D_00189E20.f1874 <= 0) {
            if (e->f20 != 0 && *(s16 *)(e->f20 + 0xAA) == 0x11DF) {
                func_L22_002C9898(0x5A, 1);
            } else {
                func_L22_002C9898(0x59, 1);
            }
            func_L23_00423588(D_00189E20.f2290, 0);
            func_00282D30(&vec, &vec, 0.095f);
        } else {
            func_00282C88(&vec);
        }
        break;
    }
    func_00282CC0(&D_00189E20.fF0, &D_00189E20.fF0, &vec);
    func_00282CC0(&D_00189E20.f110, &D_00189E20.f110, &vec);
    return 1;
}
