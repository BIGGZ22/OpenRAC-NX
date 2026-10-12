/* DRAFT - 1 instruction short: 276 bytes against 280.
 * check.py: 06_notak: 276 bytes, expected 280  (derive never runs, size gate first)
 * Every instruction matches except retail's `addiu $v1,$s1,0x10` before the first
 * 16-byte copy; retail materialises m+0x10 in $v1 and stores `sq $v0,0($v1)`, this
 * source lets the compiler fold the offset into `sq $v0,0x10($s1)`.
 * Tried: TI128 *hdr = (TI128 *)(m + 0x10); *hdr = *(TI128 *)a0;  -> still folded.
 * Next idea: a BLKmode struct assignment (mips_expand_block_move computes both
 * addresses) for the first copy only; the second copy IS folded in retail.
 */
typedef int TI128 __attribute__((mode(TI)));

extern char *func_002B9030(int a0);
extern int func_00283CF0(float f12);
extern void *D_L06_001FF50C;
extern int func_002A77E0(int a0);
extern float func_00283CE0(int a0);

void func_L06_0037E6B0(char *a0, char *a1, int a2, float f12, float f13)
{
    char *m = func_002B9030(0x53);

    if (m != 0) {
        char *st = m + 0x20;

        *(TI128 *)(m + 0x10) = *(TI128 *)a0;
        *(int *)(m + 4) = 0x50502010;
        *(unsigned char *)(m + 9) = func_00283CF0(2.0f) + 0x40;
        *(unsigned char *)(m + 3) = 0x48;
        *(unsigned char *)(m + 1) = 0;
        *(float *)(m + 0xC) = f12 * 105000.0f;
        *(unsigned char *)(m + 2) = *(unsigned char *)D_L06_001FF50C;
        *(unsigned char *)(m + 8) = func_002A77E0(0xFF);
        *(short *)(m + 0xA) = a2;
        *(TI128 *)st = *(TI128 *)a1;
        *(float *)(st + 0x14) = f12;
        *(float *)(st + 0x1C) = f13;
        *(short *)(st + 0x18) = 0;
        *(float *)(st + 0xC) = func_00283CE0((short)a2);
        *(float *)(st + 0x10) = 0.5f;
        *(short *)(st + 0x1A) = func_002A77E0(6) - 3;
    }
}
