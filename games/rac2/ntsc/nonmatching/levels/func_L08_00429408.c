/* func_L08_00429408 (152 bytes, 1 place) - best attempt, NOT MATCHED.
 *
 * Docker-free ruler (scratch/wslcmp.py): "08_tabora: instruction at +0x8 differs".  Body,
 * calls and order are right; the prologue's address materialisation is grouped differently:
 *   retail: lui/addiu 0x1B39E0 (v0) ; lui/addiu 0x1B39A0 (v1) ; sll $s0,$a0,2 ;
 *           addu $v1,$s0,$v1 ; addu $s2,$s0,$v0 ; lui/addiu 0x1B3A30 (v0) ; ...
 *   mine  : the three bases interleave differently and one base is kept in $a0.
 *
 * Measured, no effect: array-index spelling (`D_L08_001B39A0[a0] = 0`) and pointer-local
 * spelling (`s32 *v = &D_L08_001B39E0[a0]`).  Both give the same interleave.
 */

typedef signed int s32;

extern s32 D_L08_001B39A0[];
extern s32 D_L08_001B39E0[];
extern s32 D_L08_001B39F0[];
extern s32 D_L08_001B3A30[];
extern void func_00291450(s32 a0);

void func_L08_00429408(s32 a0) {
    s32 *v = &D_L08_001B39E0[a0];

    D_L08_001B39A0[a0] = 0;
    D_L08_001B3A30[a0] = 0;
    if (*v != -1) {
        func_00291450(*v);
        *v = -1;
    }
    v = &D_L08_001B39F0[a0];
    if (*v != -1) {
        func_00291450(*v);
        *v = -1;
    }
}
