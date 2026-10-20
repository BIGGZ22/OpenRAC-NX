/* func_L01_003BC490 (176 bytes, 1 place) -- NEAR MISS, not integrated.
   Allocates the 0xA26 object, copies its flag bytes, the 16-byte quad from the
   second argument, three floats and the +0x38 word, then seeds an angle with
   rand(-pi, pi).  Every instruction matches except the quad store: retail
   materialises the destination first (`addiu $a1,$s0,0x10` then `sq $v0,0($a1)`)
   where this cc1 folds the constant offset into the store (`sq $v1,0x10($s0)`),
   so the candidate is 172 bytes against 176.  Measured again here on five more
   spellings (plain cast, `TI128 *d` local, struct member through the prepared
   L01_003BC490_OBJ typedef, volatile, and a staged local) - all fold.  This is
   the TI-destination wall the sixth wave reported: only a compiler-side lever
   (or the SDK qcopy asm) reaches the addiu. */
typedef int TI128 __attribute__((mode(TI)));

extern void func_002A0F48(char *);
extern void *func_0029EFD0(int);
extern float func_002A7878(float, float);

char *func_L01_003BC490(char *a0, char *a1)
{
    char *s0 = func_0029EFD0(0xA26);

    if (s0 != 0) {
        *(unsigned char *)(s0 + 0x30) = *(unsigned char *)(a0 + 0x30);
        *(unsigned char *)(s0 + 0x20) = 1;
        *(unsigned short *)(s0 + 0x32) = *(unsigned short *)(a0 + 0x32);
        *(unsigned char *)(s0 + 0x31) = 1;
        *(TI128 *)(s0 + 0x10) = *(TI128 *)a1;
        *(float *)(s0 + 0xF0) = *(float *)(a0 + 0xF0);
        *(float *)(s0 + 0xF8) = func_002A7878(-3.1415927f, 3.1415927f);
        *(float *)(s0 + 0xF4) = *(float *)(a0 + 0xF4);
        func_002A0F48(s0);
        *(int *)(s0 + 0x38) = *(int *)(a0 + 0x38);
    }
    return s0;
}
