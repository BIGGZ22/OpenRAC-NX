/* func_002B5678 (432 bytes, boot + 20 levels): plain C compiles to 424 bytes against retail's 432; 94 of retail's 108 words align
   with this output (the 14 that do not are among the first 23: prologue saves, the first call's setup, the quadword accesses).
   Cause, diagnosed: the three 16-byte accesses in retail are register-operand inline asm in the original source, which plain C
   cannot reproduce. Retail keeps the position pointer in $s0 across the first call (`addiu $s0,$s2,0x10 ; ... ; lq $v0,0($s0) ;
   sq $v0,0($sp)`) and clears the block through a materialised pointer (`addiu $v0,$s1,0x60 ; sq $zero,0($v0)`). This cc1 folds the
   constant offset into every mode(TI) access (`lq $v0,0x10($s2)`, `sq $zero,0x60($s1)`), exactly as it does for scalar loads, so no
   pointer lives across the call: two `addiu` disappear and the register saves and the first call's argument setup rotate.
   Measured (scratch only, never in src/): with the copy written as an asm statement whose template is `lq $2,0(%1) ; sq $2,0(%0)`
   (operands `&saved` and `position` in registers, $2 clobbered, "memory") and the clear as `sq $0,0(%0)` (operand `q60` in a
   register), this same body is 432 bytes with 0 differing words. In plain C the following were measured and none keeps the
   pointer: mode(TI) pointers and struct members, a TI local, volatile, an inline helper, a second pointer variable, memcpy/memset
   (unaligned ldl/ldr or a call). What would unblock it: a maintainer decision on a register-operand quad-copy macro; it is not a
   register-allocation tie. Same cause as func_002B5828. */
typedef int Quad_002B5678 __attribute__((mode(TI)));

typedef struct {
    char pad0[0x10];
    float pos[3];                       /* 0x10 */
} Self_002B5678;

typedef struct {
    char pad0[0x28];
    float f28;                          /* 0x28 */
    char pad1[0x60 - 0x2C];
    Quad_002B5678 q60;                  /* 0x60: cleared */
    char out70[0x10];                   /* 0x70: movement of this step (func_00282CF0 output) */
    int w80;                            /* 0x80 */
    int w84;                            /* 0x84: cleared */
    char pad2[0x90 - 0x88];
    float f90;                          /* 0x90 */
    char pad3[0xA0 - 0x94];
    float vel[3];                       /* 0xA0 */
    char pad4[0xB0 - 0xAC];
    float speed;                        /* 0xB0 */
} Ctl_002B5678;

extern void func_00282CC0(void *, void *, void *);
extern void func_00282CF0(void *, void *, void *);
extern float func_00282E20(void *);
extern void func_00282F70(void *, void *, float);
extern int func_00282948(void *);
extern int func_002B5D38(void *, void *);
extern int func_002B5E48(void *, void *);

void func_002B5678(Self_002B5678 *self, Ctl_002B5678 *ctl, float *target)
{
    Quad_002B5678 saved;
    Quad_002B5678 *position = (Quad_002B5678 *)self->pos;
    int changed, i, r;
    float len;

    func_00282CC0(position, position, ctl->vel);
    saved = *position;
    ctl->w84 = 0;
    ctl->q60 = 0;
    changed = 0;
    for (i = 0; i < 8; i++) {
        r = func_002B5D38(self, ctl);
        r |= func_002B5E48(self, ctl);
        changed |= r;
        if (r == 0)
            break;
    }
    if (changed) {
        func_00282CF0(ctl->vel, self->pos, target);
        len = func_00282E20(ctl->vel);
        if (ctl->speed < len)
            func_00282F70(ctl->vel, ctl->vel, ctl->speed);
        else
            ctl->speed = len;
        if (self->pos[2] <= ctl->f90) {
            if (self->pos[2] < target[2])
                self->pos[2] = target[2];
            if (ctl->vel[2] < 0.0f)
                ctl->vel[2] = 0.0f;
        }
        if (ctl->speed < ctl->f28 * 0.5f)
            ctl->w80 += 1;
        else
            func_00282948(&ctl->w80);
    } else {
        func_00282948(&ctl->w80);
    }
    func_00282CF0(ctl->out70, self->pos, &saved);
}
