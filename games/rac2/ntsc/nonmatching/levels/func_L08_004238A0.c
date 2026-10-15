/* func_L08_004238A0 (204 bytes) — best attempt, NOT MATCHED (wave 6, w04, 2026-10-12)
 *
 * last check.py line: "08_tabora: instruction at +0x38 differs"
 * 202 of the 204 bytes identical (prologue, calls, block order, literals: everything
 * fits). Only deviation, one pair of instructions:
 *   mine  : lw $v1,0x24($s1) ; lwc1 $f0,0x24($v1)
 *   retail: lw $v0,0x24($s1) ; lwc1 $f0,0x24($v0)
 * pure temporary-register choice ($v0 against $v1). Forms tried with no effect:
 * nested expression `((Sub24 *)a0->f24)->f24`, local variable `Sub24 *q;`.
 *
 * Two measured lessons, valid elsewhere in the file:
 *  - `if (m->f20 <= 0.0f) { A } else { B }` (and not `if (f20 > 0)`) is what gives
 *    `c.ole.s $f1,$f0` + `bc1f` AND the retail block order (then inline, else at the
 *    jump); the inverse polarity gave `c.olt.s $f0,$f1`.
 *  - the float literal must be written 0.8377582f: the repo's cc1 converts the
 *    decimals with a bias of about +7 ulp (0.83775866f gives 0x3F56775A,
 *    0.837758f gives 0x3F56774F, 0.8377582f gives the expected 0x3F567752).
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef int s32;
typedef unsigned int u32;
typedef float f32;

typedef struct {
    u8 pad00[0x20];
    u8 f20;
    u8 f21;
    u8 pad22[0x24 - 0x22];
    void *f24;
    u8 pad28[0x2C - 0x28];
    f32 f2C;
    u8 pad30[0x68 - 0x30];
    void *f68;
    u8 pad6C[0x95 - 0x6C];
    u8 f95;
    u16 f96;
    u8 pad98[0xBC - 0x98];
    u8 fBC;
    u8 padBD[0x100 - 0xBD];
} Ent;

typedef struct {
    u8  pad00[0x20];
    f32 f20;
    void *f24;
    u8  pad28[0x2C - 0x28];
    f32 f2C;
    u8  pad30[0x50 - 0x30];
    u8  f50[0x88 - 0x50];
    u16 f88;
    u8  pad8A[0xE4 - 0x8A];
    f32 fE4;
    u8  padE8[0x1BE - 0xE8];
    u16 f1BE;
    u8  pad1C0[0x1E0 - 0x1C0];
    u8  f1E0[8];
} Moby;

typedef struct {
    u8  pad00[0x24];
    f32 f24;
} Sub24;

extern void func_3347E0(Ent *, void *, s32, s32, f32);
extern s32 func_3357E8(Ent *, void *, s32);
extern void func_L08_00422B60(Ent *);
extern void func_L08_00422AD8(Ent *);
extern void func_L08_00422900(Ent *);

void func_L08_004238A0(Ent *a0)
{
    Moby *s0 = a0->f68;
    Sub24 *q;
    f32 f0;
    s32 r;

    func_3347E0(a0, s0->f1E0, 2, 1, 2.5f);
    func_L08_00422900(a0);
    q = a0->f24;
    f0 = q->f24 * 0.5f;
    a0->f2C = f0;
    r = func_3357E8(a0, s0->f50, 0);
    if (r != 0) {
        if (s0->f20 <= 0.0f) {
            s0->f88 |= 2;
            s0->fE4 = 0.8377582f;
        } else {
            s0->f1BE = 330;
        }
        func_L08_00422B60(a0);
        func_L08_00422AD8(a0);
    }
}
