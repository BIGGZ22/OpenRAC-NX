/* from src/game/002B5278.c: 256 bytes against retail's 268, not matched.
 * Retail puts the zero quadword in a register first (`por $v0,$zero,$zero`, the MMI word
 * 0x700014A9 at +0x4, read as POR from the EE MMI table; rabbitizer prints it INVALID) and
 * stores that; this cc1 stores $zero straight to the stack. Retail also materialises
 * obj+0x10 and moby+0x60 (addiu) for the quad copies, where this cc1 folds the offsets into
 * the lq/sq operands. */
typedef int TI128_002B5D38 __attribute__((mode(TI)));
typedef union { TI128_002B5D38 q; float f[4]; } Vec4Q_002B5D38;

extern void func_00282CC0(void *, void *, void *);
extern void func_00282CF0(void *, void *, void *);
extern int func_00277258(void *, int, void *, float);
extern char D_001BAF80[];
extern char D_001BAFB0[];

int func_002B5D38(char *obj, char *moby)
{
    Vec4Q_002B5D38 V;
    Vec4Q_002B5D38 B;
    Vec4Q_002B5D38 Z;
    int mode;
    int r;
    TI128_002B5D38 zero = 0;
    char *dst;

    Z.q = zero;
    Z.f[2] = *(float *)(moby + 4) + *(float *)moby;
    B.q = Z.q;
    func_00282CC0(&V, obj + 0x10, &B);
    mode = (*(unsigned int *)(moby + 0x38) & 2) ? 6 : 4;
    r = func_00277258(&V, mode, obj, *(float *)moby);
    if (r != 0) {
        dst = moby + 0x60;
        *(int *)(moby + 0x84) = *(int *)(D_001BAF80 + 0x18);
        *(TI128_002B5D38 *)dst = *(TI128_002B5D38 *)(D_001BAF80 + 0x20);
        if (*(int *)(D_001BAF80 + 0x18) != 0 && *(int *)(D_001BAF80 + 0x1C) < 0) {
            if (V.f[2] < *(float *)(D_001BAF80 + 0x38))
                *(float *)(D_001BAF80 + 0x38) = V.f[2];
        }
        V.q = *(TI128_002B5D38 *)D_001BAFB0;
        func_00282CF0(obj + 0x10, &V, &B);
        return 1;
    }
    return 0;
}
