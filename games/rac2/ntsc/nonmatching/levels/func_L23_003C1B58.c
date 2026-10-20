/* func_L23_003C1B58 (812 bytes, level 23 only, -G0 unit L23_0035CE90.c) -- NEAR MISS, not integrated.
   state: 808 bytes against retail's 812 (tools/check.py, -O2 -G0). The eight-way dispatch, all eight cases, the shared
   tails and everything after the switch match: the 134-word middle (+0x4C..+0x264) and the last 38 words are identical.
   Two unreachable things remain:
   - the quadword copies: retail materialises every pointer (`addiu $a0,$sp,0x10 ; sq $v0,0($a0)` for the local copy of
     D_00189EA0, which is why `daddu $s5,$a0,$zero` sits in the jal slot, and `addiu $v1,$s4,0x290 ; lq $v0,0($s0) ;
     sq $v0,0($v1)` for the store into state+0x290). This cc1 folds the offsets (`sq $v0,0x10($sp)`,
     `sq $v0,0x290($s4)`), one word short, and the save/s5 order of the prologue follows from it (same wall as
     func_L23_00394630: retail never puts an offset in lq/sq).
   - the switch compiles to a jump table in the object's .rodata (relocations against it at +0x7C and +0x84), which
     tools/build.py cannot place ("needs .rodata at its place, which is not C"; same as func_00118CC8), so even an exact
     check.py result would not link.
   Recipes found: cases 4..7 need a block-local `char *b = D_00189E20;` (the base kept in $s0 with `lw $a1,0x2290($s0)`),
   cases 0..3 the bare global `D_0018C0B0` (`lui $v0,0x19 ; lw $a1,-0x3F50($v0)`); D_L23_001B1644/164C/36AC need
   `__attribute__((sda))` in this -G0 unit for retail's `lui $v0,0x1B ; lw $v0,0x164C($v0)`. */
typedef int TI128 __attribute__((mode(TI)));

extern void func_00282D30(void *, void *, float);

extern void func_00282CC0(void *, void *, void *);

extern char D_00189EA0[];

extern char D_00189E20[];

extern char *D_0018C0B0;

extern int D_L23_001B1644 __attribute__((sda));

extern int D_L23_001B164C __attribute__((sda));

extern int D_L23_001B36AC __attribute__((sda));

extern void func_L23_00418650(char *, float);

extern int func_002A77E0(int);

extern void func_L23_0041C6B0(char *, char *, int);

extern void func_L23_003C00D0(char *);

extern void func_002A7750(char *, int, int);

extern void func_L23_003BFFC0(char *, int, int, int, int, float);

void func_L23_003C1B58(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    TI128 v[7];

    v[1] = *(TI128 *)D_00189EA0;
    func_L23_00418650((char *)&v[1], 0.0f);
    func_00282D30((char *)&v[0], *(char **)(D_00189EA0 + 0x2210) + 0xE0, -80.0f);
    switch (func_002A77E0(8)) {
    case 0:
        func_00282D30((char *)&v[6], D_0018C0B0 + 0xC0, 50.0f);
        break;
    case 1:
        func_00282D30((char *)&v[6], D_0018C0B0 + 0xC0, -50.0f);
        break;
    case 2:
        func_00282D30((char *)&v[6], D_0018C0B0 + 0xD0, 50.0f);
        break;
    case 3:
        func_00282D30((char *)&v[6], D_0018C0B0 + 0xD0, -50.0f);
        break;
    case 4: {
        char *b = D_00189E20;

        func_00282D30((char *)&v[4], *(char **)(b + 0x2290) + 0xD0, 50.0f);
        func_00282D30((char *)&v[2], *(char **)(b + 0x2290) + 0xC0, 50.0f);
        func_00282CC0((char *)&v[6], (char *)&v[4], (char *)&v[2]);
        func_00282D30((char *)&v[6], (char *)&v[6], 0.5f);
        break;
    }
    case 5: {
        char *b = D_00189E20;

        func_00282D30((char *)&v[2], *(char **)(b + 0x2290) + 0xC0, 50.0f);
        func_00282D30((char *)&v[5], *(char **)(b + 0x2290) + 0xD0, -50.0f);
        func_00282CC0((char *)&v[6], (char *)&v[2], (char *)&v[5]);
        func_00282D30((char *)&v[6], (char *)&v[6], 0.5f);
        break;
    }
    case 6: {
        char *b = D_00189E20;

        func_00282D30((char *)&v[5], *(char **)(b + 0x2290) + 0xD0, -50.0f);
        func_00282D30((char *)&v[3], *(char **)(b + 0x2290) + 0xC0, -50.0f);
        func_00282CC0((char *)&v[6], (char *)&v[5], (char *)&v[3]);
        func_00282D30((char *)&v[6], (char *)&v[6], 0.5f);
        break;
    }
    case 7: {
        char *b = D_00189E20;

        func_00282D30((char *)&v[3], *(char **)(b + 0x2290) + 0xC0, -50.0f);
        func_00282D30((char *)&v[4], *(char **)(b + 0x2290) + 0xD0, 50.0f);
        func_00282CC0((char *)&v[6], (char *)&v[3], (char *)&v[4]);
        func_00282D30((char *)&v[6], (char *)&v[6], 0.5f);
        break;
    }
    }
    func_00282CC0((char *)&v[0], (char *)&v[0], (char *)&v[6]);
    func_00282CC0((char *)&v[0], (char *)&v[0], (char *)&v[1]);
    func_L23_0041C6B0(arg0 + 0x10, (char *)&v[0], 1);
    func_L23_00418650(arg0 + 0x10, 0.0f);
    *(TI128 *)(state + 0x290) = *(TI128 *)(arg0 + 0x10);
    func_L23_003C00D0(arg0);
    func_002A7750(arg0, 5, 0);
    func_L23_003BFFC0(arg0, 3, 0, 0x14, 3, 1.0f);
    D_L23_001B164C = (D_L23_001B164C + 1) % D_L23_001B1644;
    D_L23_001B36AC = D_L23_001B36AC + 1;
}
