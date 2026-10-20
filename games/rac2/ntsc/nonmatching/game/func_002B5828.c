/* func_002B5828 (916 bytes, boot + 20 levels): plain C compiles to 916 bytes, retail's size, with 13 of 229 words different.
   Cause, diagnosed: the one 16-byte copy `*(Quad *)position = *(Quad *)newpos` is register-operand inline asm in the original
   source (`lq $v0,0($s4) ; sq $v0,0($s3)` in retail, both pointers kept in registers). This cc1 folds the constant offset into the
   store (`sq $v0,0x10($s1)`, scheduled into the jal's delay slot), so `position` has one reference fewer and is allocated $s5
   instead of retail's $s3 (newpos $s3 against $s4, target $s4 against $s5). Retail materialises `addiu $s3,$s1,0x10` first.
   Measured (scratch only, never in src/): with that statement replaced by an asm statement whose template is
   `lq $2,0(%1) ; sq $2,0(%0)` (operands `position` and `newpos` in registers, $2 clobbered), the body below is 916 bytes with
   0 differing words, so everything else in this C is right. In plain C the following were measured and none materialises the
   pointer: mode(TI) pointer or struct, volatile, an inline helper, a local pointer. What would unblock it: a maintainer decision on
   a register-operand quad-copy macro. Same cause as func_002B5678.
   Also pinned down while getting here: the byte comparisons need `self->b43` read afresh each time and `ctl->bEA = self->b43`
   written directly (a local `u8` makes this cc1 hoist the load into a delay slot), and the divisor select is best written as
   `ctl->fD0 != 0.0f ? ctl->fD0 : ctl->f28` (or the same as if/else; `div = ctl->fD0; if (ctl->fD0 == 0.0f) div = ctl->f28;`
   adds register copies). */
typedef int Quad_002B5828 __attribute__((mode(TI)));
extern void func_002B5BC0(void *, void *, float);
extern float func_00282E20(void *);
extern float func_00283248(float, float);
extern float func_00282E80(void *, void *);
extern int func_002B6318(void *);
extern void func_002A7470(void *, int, int, int);
extern int func_002B63C0(void *, void *);
extern void func_002A90C8(void *, int, int, void *, float);

typedef struct {
    char pad0[0x10];
    float pos[3];                 /* 0x10 */
    char pad1[0x43 - 0x1C];
    unsigned char b43;            /* 0x43 */
    char pad2[0x48 - 0x44];
    float f48;                    /* 0x48 */
} Self_002B5828;

typedef struct {
    char pad0[0x08];
    float f08, f0C, f10;          /* 0x08 .. 0x10 */
    char pad1[0x28 - 0x14];
    float f28;                    /* 0x28 */
    char pad2[0x34 - 0x2C];
    float f34;                    /* 0x34 */
    unsigned flags38;             /* 0x38 */
    char pad3[0x50 - 0x3C];
    float v50[3];                 /* 0x50 */
    char pad4[0x90 - 0x5C];
    float f90;                    /* 0x90 */
    unsigned flags94;             /* 0x94 */
    char pad5[0x9C - 0x98];
    int flags9C;                  /* 0x9C */
    char pad6[0xA8 - 0xA0];
    float fA8;                    /* 0xA8 */
    char pad7[0xB0 - 0xAC];
    float speed;                  /* 0xB0 */
    float fB4;                    /* 0xB4 */
    char pad8[0xC0 - 0xB8];
    short hC0, hC2;               /* 0xC0 */
    char pad9[0xCC - 0xC4];
    float fCC;                    /* 0xCC */
    float fD0, fD4;               /* 0xD0 */
    char padA[0xE6 - 0xD8];
    unsigned char bE6, bE7, bE8, bE9, bEA;   /* 0xE6 */
} Ctl_002B5828;

void func_002B5828(Self_002B5828 *self, Ctl_002B5828 *ctl, void *target, void *newpos)
{
    int changed;
    float *position;
    float angle, dist, div, ratio;
    unsigned char b;

    func_002B5BC0(self, ctl, 0.0f);
    changed = 0;
    if (self->pos[2] < ctl->f90 - ctl->f08) {
        changed = 1;
        ctl->flags94 |= 2;
    }
    if (((ctl->flags38 ^ 1) & 1) && ctl->hC2 < 5) {
        if (ctl->f90 + ctl->f0C < self->pos[2]) {
            changed = 1;
            ctl->flags94 |= 1;
        }
    }
    if (ctl->hC0 != 0) {
        angle = func_00283248(ctl->v50[2], func_00282E20(ctl->v50));
        if (ctl->f10 < angle) {
            changed = 1;
            ctl->flags94 |= 4;
        }
    }
    position = self->pos;
    if (changed) {
        *(Quad_002B5828 *)position = *(Quad_002B5828 *)newpos;
        func_002B5BC0(self, ctl, 0.0f);
    }
    if (self->pos[2] <= ctl->f90) {
        if (ctl->f90 <= self->pos[2] - ctl->fA8)
            self->pos[2] = ctl->f90;
        ctl->hC2 = 0;
        ctl->hC0++;
    } else {
        ctl->hC0 = 0;
        ctl->hC2++;
    }
    dist = func_00282E80(position, target);
    ctl->fCC = dist;
    if (dist <= ctl->f34)
        ctl->flags94 |= 0x80;
    if (ctl->flags9C & 1) {
        if (func_002B6318(self))
            ctl->flags9C &= ~1;
        else
            ctl->flags94 |= 0x100;
    }
    if (ctl->flags38 & 0x80) {
        if (self->b43 == ctl->bE8 || self->b43 == ctl->bE9) {
            div = ctl->fD0 != 0.0f ? ctl->fD0 : ctl->f28;
            ratio = ctl->speed / div;
            self->f48 = ratio;
            if (ctl->fD4 < ratio)
                self->f48 = ctl->fD4;
            else if (ratio < 1.0f / ctl->fD4)
                self->f48 = 1.0f / ctl->fD4;
        }
    }
    if (ctl->flags38 & 0x40) {
        if ((ctl->flags9C & 2) && ctl->fB4 != 0.0f) {
            if (self->b43 == ctl->bE8 || self->b43 == ctl->bE9) {
                ctl->bEA = self->b43;
                func_002A7470(self, 0.0f < ctl->fB4 ? ctl->bE6 : ctl->bE7, 0, 10);
            }
        } else {
            if (self->b43 == ctl->bE6 || self->b43 == ctl->bE7) {
                if (ctl->bEA != 0xFF)
                    func_002A7470(self, ctl->bEA, 0, 10);
            }
        }
    }
    if (ctl->flags38 & 0x200) {
        if (func_002B63C0(self, ctl))
            func_002A90C8(self, 0, 0x10000, position, 255.0f);
    }
}
