/* func_L12_0041A578 (108 bytes, 2 places) - best attempt, NOT MATCHED.
 *
 * Docker-free ruler (scratch/wslcmp.py): "12_todano: 112 bytes, expected 108".
 * The scan, the two returns (the entry index, or count-1) and the 0.0f float test are
 * understood; residue:
 *   retail: lw $v1,0x68($a0) ; lui $a2,0x20 ; addiu $a2,$a2,-0x1540 ; addiu $a1,$a1,0x1 ; ...
 *           slt $v0,$a1,$a2 ; bnel $v0,$zero,... ; lwc1 ; jr $ra ; daddu $v0,$v1,$zero ;
 *           c.eq.s ; nop ; bc1t ; addiu $a0,$a0,0x10 ; b ; addiu $a1,$a1,0x1
 *   mine  : the index lands in $a2 (the parameter is not incremented in place) and this cc1
 *           folds the loop-back into the float test (`bc1fl`), one instruction longer.
 *
 * Measured, no effect: `i = a1 + 1` in a local, and `a1 = a1 + 1` reusing the parameter.
 */

typedef signed int s32;
typedef float f32;

extern unsigned char D_L12_001FEAC0[];

s32 func_L12_0041A578(char *a0, s32 a1)
{
    char *q = *(char **)(a0 + 0x68);
    char *e = *(char **)(D_L12_001FEAC0 + 4 * *(s32 *)(q + 0x110));
    s32 n = *(s32 *)e;
    char *p;

    a1 = a1 + 1;
    p = e + (a1 << 4) + 0x1C;
    for (; a1 < n; a1++, p += 0x10) {
        if (*(f32 *)p == 0.0f) {
            return a1;
        }
    }
    return n - 1;
}
