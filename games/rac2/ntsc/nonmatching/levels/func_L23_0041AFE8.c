/* func_L23_0041AFE8 (416 bytes, level 23 only) -- NEAR MISS, not integrated.
   Belongs in src/levels/23_damosel_orbit/L23_00416388-g8.c (it reads D_L23_001B157C,
   D_L23_001B1510..D_L23_001B1534 through $gp, so the game compiler builds it with -G8).
   state: 396 bytes against retail's 416, frame 0xD0 and the argument spill at 0x70 like
   retail. WALL, not a wording problem: retail copies four of its five 16-byte vectors to or
   from a local through an address register,
       lq $v0,0($a2)  ; sq $v0,0($s0)                       (s0 = sp+0x20, kept in a register)
       addiu $v1,$sp,0x60 ; lq $v0,0($s2) ; sq $v0,0($v1)
       addiu $a0,$sp,0x40 ; lq $v0,0($s2) ; sq $v0,0($a0)   (a second, separate load)
       addiu $v1,$sp,0x40 ; lq $v0,0($s0) ; sq $v0,0($v1)   (in the loop)
   This cc1 folds the frame offset into the access (`sq $v0,0x60($sp)`), reuses the first
   load for the second store and needs no address `addiu`s: it is short by those
   instructions and one load. It is the wall already recorded for the L08 quad copies in
   docs/COMPILER-NOTES.md; every spelling of the copy tried here (TI128 locals, an array of
   them, pointer variables, a second name for the loop vector) gives the folded form. The
   loop's register numbering differs as well (retail keeps &v50 in two registers, $s0 and
   $s2, and &v30 in $fp).
   What is right in structure: the call sequence, the loop (`k = 2; k >= 0; k--`, three
   passes), and the 16-argument call to func_L23_004256F0, which takes eight integer
   registers ($a0-$a3 and $t0-$t3), four float registers ($f12-$f15) and four stack words
   (D_L23_001B1534, 1, -1, 1). The handle slot test is where retail uses an annulled
   `bnel $v1,$zero,...` with `addiu $s4,$s4,4` in the slot (another recorded wall: the
   annulled branch slot); this C compiles it as `bnez` and puts the increment after.
   The five locals sit at sp+0x20..0x6F in declaration order (an array of TI128 locals keeps
   that order; separate scalars would be placed in the order of their first address use). */

typedef int TI128 __attribute__((mode(TI)));

extern float D_L23_001B157C __attribute__((sda));
extern float D_L23_001B378C;
extern char D_L23_001B37A0[];
extern int D_L23_001B1534 __attribute__((sda));
extern unsigned char D_L23_001B1510 __attribute__((sda));
extern unsigned char D_L23_001B1514 __attribute__((sda));
extern float D_L23_001B1518 __attribute__((sda));
extern float D_L23_001B151C __attribute__((sda));
extern float D_L23_001B1520 __attribute__((sda));
extern float D_L23_001B1524 __attribute__((sda));
extern short D_L23_001B1528 __attribute__((sda));
extern short D_L23_001B152C __attribute__((sda));
extern int D_L23_001B1530 __attribute__((sda));

extern void func_L23_00418650(char *, float);
extern void func_00282CC0(float *, float *, float *);
extern void func_00282CF0(float *, float *, float *);
extern void func_00282F20(float *, float *, float);
extern int func_L23_004256F0(char *, char *, char *, int, int, int, int, int, float, float, float, float, int, int, int, int);

void func_L23_0041AFE8(char *obj, TI128 *p1, TI128 *p2, TI128 *out, int *handles) {
    TI128 loc[5];
    TI128 *v20 = &loc[0];
    TI128 *v30 = &loc[1];
    TI128 *v40 = &loc[2];
    TI128 *v50 = &loc[3];
    TI128 *v60 = &loc[4];
    TI128 *q;
    char *prev;
    int k;

    *v20 = *p2;
    func_L23_00418650((char *)v20, D_L23_001B157C);
    func_00282CF0((float *)v30, (float *)v20, (float *)p1);
    func_00282F20((float *)v30, (float *)v30, D_L23_001B378C);
    *v60 = *p1;
    *v40 = *p1;
    func_L23_00418650((char *)v40, D_L23_001B157C);
    prev = D_L23_001B37A0;
    q = v50;
    for (k = 2; k >= 0; k--) {
        func_00282CC0((float *)q, (float *)v60, (float *)v30);
        func_L23_00418650((char *)q, D_L23_001B157C);
        *out = *q;
        if (*handles == 0) {
            *handles = func_L23_004256F0(prev, obj, (char *)out, D_L23_001B1510, D_L23_001B1514, D_L23_001B1528, D_L23_001B152C, D_L23_001B1530, D_L23_001B1518, D_L23_001B151C, D_L23_001B1520, D_L23_001B1524, D_L23_001B1534, 1, -1, 1);
        }
        handles++;
        *v40 = *v50;
        prev = (char *)out;
        func_00282CC0((float *)v60, (float *)v60, (float *)v30);
        out++;
    }
}
