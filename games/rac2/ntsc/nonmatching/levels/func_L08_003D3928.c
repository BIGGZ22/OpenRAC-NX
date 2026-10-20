/* func_L08_003D3928 (196 bytes, 3 places) -- NEAR MISS, not integrated.
   Keeps the smallest of rand() samples on the 0x001B34D4 bank (stamping the
   caller, a zero and a quad copy into 0x001B34C0), and re-stamps 0x001B3490 when
   the caller matches the resident 0x001B34A0 word.
   Profile lever found: retail writes every global with `lui $at,%hi; sw $x,%lo($at)`,
   which the default profile does not give - it materialises each address into a
   named register and (worse) CSEs the 0x001B34D4 address across the branch, for a
   188-byte candidate.  Under `-mno-split-addresses` the store form matches and the
   candidate is 192 bytes against 196, with only the prologue save order (this
   cc1: sd $s0,0x0 before sd $s1,0x8) and the bc1f delay slot (this cc1 fills it
   with `addiu $v1,zero,1`, retail has nop and sets it after the sq) left.  Needs a
   unit of its own with that flag, or a save-order lever. */
typedef int TI128 __attribute__((mode(TI)));

extern char D_00189EA0[];
extern float func_00282E80(char *, void *);

extern int D_001B34A0;
extern float D_001B34A4;
extern int D_001B34B0;
extern TI128 D_001B34C0;
extern int D_001B34D0;
extern float D_001B34D4;
extern int D_001B34D8;
extern int D_001B34DC;
extern int D_001B34E0;
extern TI128 D_001B3490;

void func_L08_003D3928(char *a0, char *a1)
{
    float v = func_00282E80(D_00189EA0, a1);

    if (v < D_001B34D4) {
        D_001B34DC = 0;
        D_001B34D0 = (int)a0;
        D_001B34D8 = 0;
        D_001B34D4 = v;
        D_001B34C0 = *(TI128 *)a1;
        D_001B34E0 = 1;
    }
    if (D_001B34A0 == (int)a0) {
        D_001B34A4 = v;
        D_001B3490 = *(TI128 *)a1;
        D_001B34B0 = 1;
    }
}
