/* func_L02_003DBEC8 (120 bytes, 8 places) - best attempt, NOT MATCHED (register wall).
 *
 * Docker-free ruler (scratch/wslcmp.py): "02_maktar_nebula: 112 bytes, expected 120".
 * Loads, stores, store order, the table lookup, the 16-byte lq/sq copy and the trailing
 * func_002A7750(a0,1,-1) are all in the retail sequence; the whole residue is allocation:
 *   retail: daddu $a2,$a0,$zero ; lw $a3,0x68($a2) ; ... ; sw $zero,0x4($a3)
 *   mine  : daddu $t1,$a0,$zero ; lw $t0,0x68($t1) ; ... ; sw $zero,0x4($t0)
 * and this cc1 sinks the epilogue's `ld $ra,0x0($sp)` into the bnez delay slot where retail
 * reloads the parameter copy into $a3 there, which is where the four missing instructions go.
 *
 * The sibling func_L02_003D7B88 asks for $t1/$t0 (from the same spelling) and func_L08's
 * table gate wants the mirror image, so the register pick is a property of the body, not of
 * the spelling.  Two waves have now hit this wall; needs a lever on the allocator.
 */

typedef unsigned char u8;
typedef signed int s32;
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

extern char *D_L02_001FF1C0[];
extern void func_002A7750(void *, int, int);

void func_L02_003DBEC8(char *a0)
{
    char *q;
    char *e;
    if (*(u8 *)(a0 + 0x20) != 0) {
        return;
    }
    q = *(char **)(a0 + 0x68);
    *(u8 *)(a0 + 0x30) = 0x80;
    *(f32 *)(q + 0xC) = -1.0f;
    e = D_L02_001FF1C0[*(s32 *)q];
    *(s32 *)(q + 0x8) = 0;
    *(char **)(q + 0x10) = e;
    *(TI128 *)(a0 + 0x10) = *(TI128 *)(e + 0x10);
    func_002A7750(a0, 1, -1);
}
